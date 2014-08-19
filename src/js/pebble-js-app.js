// thanks to Ted Gerold (tgwaste) for PebbleJQuerySettingsExample

// app ready event
//
Pebble.addEventListener("ready",
	function(e) {
		console.log("connect! [" + e.ready + "] (" + e.type + ")");
	}
);
// app message sent successfully
//
function appMessageAck(e) {
	console.log("App message sent successfully");
}

// app message failed to send
//
function appMessageNack(e) {
	console.log("App message failed to send: " + e.error.message);
}

// display configuration screen
//
Pebble.addEventListener("showConfiguration",
	function() {
		// No HTML file yet
		var config = "http://ron64.com/pposdemo/pposdemo1.0.html";
		var settings = encodeURIComponent(localStorage.getItem("SelectedSettings"));
		var url = config + "?settings=" + settings;
		console.log("Showing configuration screen...");
		console.log(url);
		Pebble.openURL(url);
	}
);


// close configuration screen
//
Pebble.addEventListener("webviewclosed",
	function(e) {
		var options;
		try {
			options = JSON.parse(decodeURIComponent(e.response));
			//localStorage.clear();
			localStorage.setItem("SelectedSettings", JSON.stringify(options));
			//console.log("Settings: " + localStorage.getItem("SelectedSettings"));
			Pebble.sendAppMessage(options, appMessageAck, appMessageNack);
			console.log("sentmsg: "+ JSON.stringify(options));
		} catch(err) {
			console.log("No JSON response or received Cancel event");
			console.log("Settings: " + JSON.parse(decodeURIComponent(e.response)));
			options = false;
		}
	}
);

Pebble.addEventListener("appmessage",
	function(e) {

});