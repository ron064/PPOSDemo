# Build script for Overlay project for pebble watch
# Copyright (C) 2014  Gregoire Sage and Ron Ravid
# See README.md for license details
# (Dual Licence: GPL is valid for free application with open source, contact us for other cases)

top = '.'
out = 'build'

import re
from subprocess import Popen,PIPE

def options(ctx):
    ctx.load('pebble_sdk')

def configure(ctx):
    ctx.load('pebble_sdk')

    pebbleone_path = ctx.path.find_dir('libraries/PebbleONE/delivery').abspath()
    ctx.check(stlib='pebbleone', stlibpath=pebbleone_path, uselib_store='pebbleone')

def build(ctx):
    ctx.load('pebble_sdk')
    
	
    # generate the ld script template with overlay sections
    ctx(
        rule    = generate_ld_tpl_ovl,
        source  = ['pebble_app.ld.tpl', 'overlays.json'],
        target  = ['pebble_app_ovl.ld.tpl','overlays.h']
    )

    # generate the default ld script
    ctx(
        rule    = generate_default_ld_script,
        source  = 'pebble_app_ovl.ld.tpl',
        target  = 'pebble_app_default.ld'
    )

    # next step depends on ld script generation
    ctx.add_group()
        
    # Build elf (first pass)
    ctx.pbl_program(source=ctx.path.ant_glob('src/**/*.c'),
                    target='pebble-app_comp.elf',
                    ldscript='pebble_app_default.ld',
                    use=['pebbleone'])

    # generate the final ld script
    ctx(
        rule    = generate_final_ld_script,
        source  = ['pebble_app_ovl.ld.tpl','pebble-app_comp.elf'],
        target  = 'pebble_app_final.ld'
    )

    # add group for the final pbw build
    ctx.add_group()

    # if the binary files have changed after the extraction, at this time the program resources
    # are not good, we must call the script one more time to have a correct generated final .pbw
    ctx.pbl_program(source=ctx.path.ant_glob(['src/**/*.c']),
                    target='pebble-app.elf',
                    ldscript='pebble_app_final.ld',
                    use=['pebbleone']	)

    # extract the binary overlay resources from elf file
    ctx(
        rule   = extract_ovl_sections,
        source = 'pebble-app.elf'
    )

    ctx.add_group()

    ctx.custom_pbl_bundle(elf='pebble-app.elf',
                   js=ctx.path.ant_glob('src/js/**/*.js'))

    # # ctx.pbl_bundle(elf='pebble-app.elf',
    # #                js=ctx.path.ant_glob('src/js/**/*.js'))

from waflib import TaskGen, Node
from waflib.TaskGen import before_method,feature
import string
import json

def generate_ld_tpl_ovl(task):
    template_script = open( task.inputs[0].abspath() ,'r' )
    overlays_json = open( task.inputs[1].abspath() ,'r' )
    ldscript = open( task.outputs[0].abspath() ,'w' )
    overlay_header = open( task.outputs[1].abspath() ,'w' )

    ovl_content = ''
    ovl_table = ''

    firstoverlay_name = ''
    overlay_name = ''

    overlay_header.write('typedef enum {\n')

    overlays=json.load(overlays_json)

    for idx, overlay in enumerate(overlays['overlays']):

        overlay_name = overlay['name']

        ovl_content = ovl_content + '\t\t'+overlay_name+'_ovl\n'
        ovl_content = ovl_content + '\t\t{\n'

        for fileName in overlay['files'] :
            if 'library' in overlay :
                libraryName = overlay['library']
                ovl_content = ovl_content + '\t\t\t*/'+libraryName+':'+fileName+'*(.text)\n'
                ovl_content = ovl_content + '\t\t\t*/'+libraryName+':'+fileName+'*(.text.*)\n'
                ovl_content = ovl_content + '\t\t\t*/'+libraryName+':'+fileName+'*(.rodata)\n'
                ovl_content = ovl_content + '\t\t\t*/'+libraryName+':'+fileName+'*(.rodata*)\n'
                ovl_content = ovl_content + '\t\t\t*/'+libraryName+':'+fileName+'*(.data.*)\n'
                ovl_content = ovl_content + '\t\t\t*/'+libraryName+':'+fileName+'*(.bss)\n'
                ovl_content = ovl_content + '\t\t\t*/'+libraryName+':'+fileName+'*(.bss*)\n'
            else :
                ovl_content = ovl_content + '\t\t\t*/'+fileName+'*(.text)\n'
                ovl_content = ovl_content + '\t\t\t*/'+fileName+'*(.text.*)\n'
                ovl_content = ovl_content + '\t\t\t*/'+fileName+'*(.rodata)\n'
                ovl_content = ovl_content + '\t\t\t*/'+fileName+'*(.rodata*)\n'
                ovl_content = ovl_content + '\t\t\t*/'+fileName+'*(.data.*)\n'
                ovl_content = ovl_content + '\t\t\t*/'+fileName+'*(.bss)\n'
                ovl_content = ovl_content + '\t\t\t*/'+fileName+'*(.bss*)\n'

        ovl_content = ovl_content + '\t\t}\n'

        if(firstoverlay_name == ''):
            ovl_table = '\t\t\tLONG(0);\n'
            firstoverlay_name = overlay_name
        else:
            ovl_table = ovl_table + '\t\t\tLONG(LOADADDR('+overlay_name+'_ovl) - LOADADDR('+firstoverlay_name+'_ovl));\n'
        ovl_table = ovl_table + '\t\t\tLONG(SIZEOF(' + overlay_name +'_ovl));\n'

        overlay_header.write('\t' + overlay_name.upper() + '_OVL = ' + str(idx) + ',\n')

    overlay_header.write('} OverlayId;')

    values = {'OVL_CONTENT' : ovl_content, 'OVL_TABLE' : ovl_table}

    for line in template_script:
        line = string.Template(line)
        line = line.safe_substitute(values)
        ldscript.write(line)

def generate_default_ld_script(task):
    template_script = open( task.inputs[0].abspath() ,'r' )
    ldscript = open( task.outputs[0].abspath() ,'w' )

    values = {
        'STRT_ORIGIN'   : '0',
        'STRT_LENGTH'   : '0x3000',
        'APP_ORIGIN'    : '0x3000',
        'APP_LENGTH'    : '0x5000',
        'OVERLAY_AT'    : '0xA000'
    }

    for line in template_script:
        line = string.Template(line)
        line = line.substitute(values)
        ldscript.write(line)

def get_app_length(elf_file):
    app_length=0
    readelf_process=Popen(['arm-none-eabi-readelf','-SW',elf_file],stdout=PIPE)
    readelf_output=readelf_process.communicate()[0]
    if not readelf_output:
        raise InvalidBinaryError()
    for line in readelf_output.splitlines():
        if len(line)<10:
            continue
        line=line[6:]
        columns=line.split()
        if columns[0]=='.text2' :
            app_length -= int(columns[2],16)
        elif columns[0]=='.bss':
            app_length += int(columns[2],16) + int(columns[4],16)
    app_length += 0x100 - (app_length % 0x100) # 0x100 alignment
    return app_length

def get_strt_length(elf_file):
    strt_length=0
    readelf_process=Popen(['arm-none-eabi-readelf','-SW',elf_file],stdout=PIPE)
    readelf_output=readelf_process.communicate()[0]

    if not readelf_output:
        raise InvalidBinaryError()
    for line in readelf_output.splitlines():
        if len(line)<10:
            continue
        line=line[6:]
        columns=line.split()
        if columns[0].endswith("_ovl") and not columns[0].startswith(".") :
            strt_length = max(strt_length, int(columns[4],16))
    strt_length += 0x84
    strt_length += 0x100 - (strt_length % 0x100) # 0x100 alignment
    return strt_length

def get_overlay_address(elf_file):
    strt_length = get_strt_length(elf_file)
    app_length  = get_app_length(elf_file)
    return (strt_length + app_length)

def generate_final_ld_script(task):
    template_script = open( task.inputs[0].abspath() ,'r' )
    ldscript = open( task.outputs[0].abspath() ,'w' )

    strt_length = get_strt_length(task.inputs[1].abspath())
    app_origin = strt_length
    app_length = get_app_length(task.inputs[1].abspath())
    overlay_at = get_overlay_address(task.inputs[1].abspath())

    print "STRT_LENGTH = " + str(hex(strt_length))
    print "APP_LENGTH = " + str(hex(app_length))
    print "OVERLAY_AT = " + str(hex(overlay_at))

    values = {
        'STRT_ORIGIN'   : '0',
        'STRT_LENGTH'   : str(hex(strt_length)),
        'APP_ORIGIN'    : str(hex(app_origin)),
        'APP_LENGTH'    : str(hex(app_length)),
        'OVERLAY_AT'    : str(hex(overlay_at))
    }

    for line in template_script:
        line = string.Template(line).substitute(values)
        ldscript.write(line)

def extract_ovl_sections(task):
    def extract_section(elf_file, offset, size):
        elf = open(elf_file,'rb')
        elf.seek(offset)
        with open('resources/data/OVL.bin', "ab") as f:
            data = elf.read(size)
            f.write(data)

    def extract_sections(elf_file):
        readelf_process=Popen(['arm-none-eabi-readelf','-SW',elf_file],stdout=PIPE)
        readelf_output=readelf_process.communicate()[0]
        if not readelf_output:
            raise InvalidBinaryError()
        for line in readelf_output.splitlines():
            if len(line)<10:
                continue
            line=line[6:]
            columns=line.split()
            if columns[0].endswith("_ovl") and not columns[0].startswith(".") :
                extract_section(elf_file, int(columns[3],16), int(columns[4],16))

    with open('resources/data/OVL.bin', "wb") as f:
            f.write('')

    extract_sections(task.inputs[0].abspath())

import time
import waflib.extras.pebble_sdk as pebble_sdk
import waflib.extras.inject_metadata as inject_metadata
import waflib.extras.ldscript as ldscript
import waflib.extras.mkbundle as mkbundle
import waflib.extras.objcopy as objcopy
import waflib.extras.c_preproc as c_preproc
import waflib.extras.xcode_pebble
from waflib import Logs
from waflib.TaskGen import before_method,feature

@feature('custom_pbl_bundle')
def custom_make_pbl_bundle(self):
    timestamp=self.bld.options.timestamp
    pbw_basename='app_'+str(timestamp)if timestamp else self.bld.path.name
    if timestamp is None:
        timestamp=int(time.time())
    elf_file=self.bld.path.get_bld().make_node(getattr(self,'elf'))
    if elf_file is None:
        raise Exception("Must specify elf argument to pbl_bundle")
    full_raw_bin_file=self.bld.path.get_bld().make_node('pebble-app.full.raw.bin')
    raw_bin_file=self.bld.path.get_bld().make_node('pebble-app.raw.bin')
    def trim_raw_file(task):
        overlay_address=get_overlay_address(task.inputs[0].abspath())
        full_raw    = open( task.inputs[1].abspath() ,'rb' )
        trimmed_raw = open( task.outputs[0].abspath() ,'wb' )
        bytes=full_raw.read(overlay_address)
        trimmed_raw.write(bytes)
    self.bld(rule=objcopy.objcopy_bin,source=elf_file,target=full_raw_bin_file)
    self.bld(rule=trim_raw_file,source=[elf_file, full_raw_bin_file],target=raw_bin_file)
    js_nodes=self.to_nodes(getattr(self,'js',[]))
    js_files=[x.abspath()for x in js_nodes]
    has_jsapp=len(js_nodes)>0
    def inject_data_rule(task):
        bin_path=task.inputs[0].abspath()
        elf_path=task.inputs[1].abspath()
        res_path=task.inputs[2].abspath()
        tgt_path=task.outputs[0].abspath()
        cp_result=task.exec_command('cp "{}" "{}"'.format(bin_path,tgt_path))
        if cp_result<0:
            from waflib.Errors import BuildError
            raise BuildError("Failed to copy %s to %s!"%(bin_path,tgt_path))
        inject_metadata.inject_metadata(tgt_path,elf_path,res_path,timestamp,allow_js=has_jsapp)
    resources_file=self.bld.path.get_bld().make_node('app_resources.pbpack.data')
    bin_file=self.bld.path.get_bld().make_node('pebble-app.bin')
    self.bld(rule=inject_data_rule,name='inject-metadata',source=[raw_bin_file,elf_file,resources_file],target=bin_file)
    resources_pack=self.bld.path.get_bld().make_node('app_resources.pbpack')
    pbz_output=self.bld.path.get_bld().make_node(pbw_basename+'.pbw')
    def make_watchapp_bundle(task):
        watchapp=task.inputs[0].abspath()
        resources=task.inputs[1].abspath()
        outfile=task.outputs[0].abspath()
        return mkbundle.make_watchapp_bundle(appinfo=self.bld.path.get_src().find_node('appinfo.json').abspath(),js_files=js_files,watchapp=watchapp,watchapp_timestamp=timestamp,sdk_version=pebble_sdk.SDK_VERSION,resources=resources,resources_timestamp=timestamp,outfile=outfile)
    self.bld(rule=make_watchapp_bundle,source=[bin_file,resources_pack]+js_nodes,target=pbz_output)
    def report_memory_usage(task):
        src_path=task.inputs[0].abspath()
        size_output=task.generator.bld.cmd_and_log([task.env.SIZE,src_path],quiet=waflib.Context.BOTH,output=waflib.Context.STDOUT)
        text_size,data_size,bss_size=[int(x)for x in size_output.splitlines()[1].split()[:3]]
        app_ram_size=data_size+bss_size+text_size
        max_app_ram=inject_metadata.MAX_APP_MEMORY_SIZE
        free_size=max_app_ram-app_ram_size
        Logs.pprint('YELLOW',"Memory usage:\n=============\n""Total app footprint in RAM:     %6u bytes / ~%ukb\n""Free RAM available (heap):      %6u bytes\n"%(app_ram_size,max_app_ram/1024,free_size))
    self.bld(rule=report_memory_usage,name='report-memory-usage',source=[elf_file],target=None)
from waflib.Configure import conf
@conf
def custom_pbl_bundle(self,*k,**kw):
    kw['features']='custom_pbl_bundle'
    return self(*k,**kw)