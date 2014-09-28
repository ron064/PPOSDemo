# PPOSDemo - Overlay demonstration #


By Ron Ravid & Gregoire Sage

PPOSDemo is a Pebble watch face demonstrating how to use overlay to load/unload parts of program from resources to into the available 24K memory. The idea is to free program memory so that more code and data can be used. This is achieved by loading only selected parts of program to memory. Each selected part is loaded only when it is needed.
It may be used as demo of watchfaces in point-of-sale

## Licence ##
The Overlay parts (Including overlord- the overlay loader, build script, generating linker scripts etc) are under dual license:

* GPL is available for developers who publish their derived source code with folowing conditions:
  * The derived source code is open source.
  * There is no charge for application or for required application/service needed for the application.
* Other developers are required to contact us about license.
License for each watchface and for some of the infrastructure included here is according to original license of developers. See credits section for full list.


### Compatibility ###
Overlay can work from SDK 2.3. For working with older SDK a modification is needed in one of the SDK files(ask us if interested)

### Bypassing limitation for Overlay  ###
There are several difficulties to convert specific type of code to work in overlay. These types can cause a crash of the program. I guess that mostly it is related to tables which compiler create in memory and don't work well with overlay.
Here is a list of problems we found and how to fix them.

* Change any string array from form of `const char* days[]` to `const char days[][12]` (one-two bytes bigger than length of your longest string.)
* If you use GPath, move your GPath data to external file with setup function(see hop_picker for example).
* `menu_layer_set_callbacks`. Instead of defining 'MenuLayerCallbacks menu_callbacks' and giving it to that function as parameter, put it directly inside the 3rd parameter of menu_layer_set_callbacks like: `(MenuLayerCallbacks){ .get_num_rows = get_num_rows, .draw_row = draw_row, .select_click = select_click });` see example in [Pebble demos feature_menu_layer](https://github.com/gregoiresage/pebble-demos/blob/c20c31ca4994f08dfae740127edad41b206ece1e/src/watchapps/feature_menu_layer/src/feature_menu_layer.c)
* `vibes_enqueue_custom_pattern`. Move your custom pattern to external non-overlay file. see example of separating a `doVibe()` command in [Pebble demos feature_vibe_custom](https://github.com/gregoiresage/pebble-demos/tree/c20c31ca4994f08dfae740127edad41b206ece1e/src/watchapps/feature_vibe_custom/src)

### Watch face modification steps ###
To practice use of overlay we suggest that you will prepare a watchface for the project. It can help us as well to add something for the demo.

* Remove references to Window in init() and deinit(). Let the launcher create and destroy the main window.
* Add #include "config.h"
* Provide entrypoints for creating (load_xxx), destroying (unload_xxx),  and redrawing (redraw_xxx) the window content. The last line of load_xxx should call configRedraw(); Add ENTRYPOINTS(xxx) and ENTRYPOINTROW(xxx), into config.h
* Make all the remaining global variables and functions in your app static - to prevent identical named globals from other apps from clashing.
* Change all GColorBlack to backColor and GColorWhite to foreColor
* If you use float and it's not totally necessary, please convert it to integer. (float SW support adds 2K to code)
* Test your watchface for leaks. Use pebble logs and make sure you see Still allocated <0B> when you exit the app.
* For settings, define yours in the enum at config.h . Use this enum for your settings with AllSet[SET_my_def], and set its values in the html file.

We made two example projects where we converted existing watchface of other developers to work as overlay in this project:

- [BlockSlide](https://github.com/ron064/Blockslide-Date_2.0)
- [PebbleONE](https://github.com/ron064/PebbleONE)

### for overlay ###

* In config.h for OVL_RESOURCE_IDS[], add 0 for every screen in its correct position when it's not overlay. Add RESOURCE_ID when it is overlay.
* In config.h for OVL_debug[], put -1 for screen that is not in overlay, and the index when it is in overlay. (this part is for for debug information, it is not implemented yet)
* in overlays.json add entry for your files

## Compilation ##
Currently you will need to compile 3 times in most cases. The script we use does this triple compilation. The building process includes:

* Generate the default LD script and build the first elf file, calculate the maximum overlay size from elf file and generate the final LD script
* Build a second elf file with the final LD script, extract the overlays resource files from the new elf file
* Generate the resources pack file and resources_ids.auto.h, build the final elf and build the PBW

We plan to improve the process


### ToDo list ###

Project is not complete. Here is partial list of ToDo:

* General invert layer may be added
* In html we need a list with screen icons.
* Debug information is not implemented yet for overlay.
* More screens should be added

## Credits ##

* Overlay design By Ron64 (Overlay research, linker overlay script design and memory management)
* Overlay wscript implementation by Gregoire (automatic creation of LD overlay script, and automatic injecting overlay memory into resources)
* The infrastructure of this program is based on the project [sevenwatchfaces](https://github.com/mikebikemusic/sevenwatchfaces) by mikebikemusic. We added the memory management, designed the linker LD scripts that handle the overlay, the wscript that prepare the LD script, and prepare the overlay parts for the resources.
* Setting, Tap control etc by Ron64

### watchfaces ###

* [accuinfo](https://github.com/bobhwasatch/accuinfo/) By Bob Hauck
* [BlockSlide](https://github.com/Jnmattern/Blockslide-Date_2.0) By Jnm (Jean-Nöel Mattern)
* [perspective](https://github.com/Jnmattern/Perspective/) By Jnm
* [hop_picker](https://github.com/gregoiresage/hop-picker) By Gregoire Sage
* [illudere](https://github.com/dmnd/illudere) By Desmond Brand
* [retroclock](https://bitbucket.org/lingen/retroclock-pebble/wiki/Home) By Ronald van der Lingen
* fuzzy_time.c By Pebble
* just_a_bit.c By Pebble
* tic_tock_toe By Pebble
* simplicity By Pebble
* [PebbleOne] (https://github.com/bertfreudenberg/PebbleONE/) by Bert Freudenberg

Most Pebble watchfaces were adjusted by mikebikemusic
BlockSlide & accuinfo and setting in all watchfaces adjusted by Ron64.
HopPicker adjusted by Gregoire Sage

