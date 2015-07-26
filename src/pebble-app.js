var dict;


Pebble.addEventListener('ready', function(e) {
	console.log('js ready');
	// check for localStorage values previously sent from config.html
	dict = localStorage.config ? JSON.parse(localStorage.config) : {};
});

Pebble.addEventListener('showConfiguration', function(e) {
	// Show config page
	Pebble.openURL('http://toes.tdsb.on.ca/apps/pebble/config.html?config=' + encodeURIComponent(JSON.stringify(dict)));
});


Pebble.addEventListener('webviewclosed', function(e) {
	// Decode and parse config data as JSON
	dict = JSON.parse(decodeURIComponent(e.response));
	console.log('Config window returned: ', JSON.stringify(dict));
	

	// Send settings to Pebble watchapp
	Pebble.sendAppMessage(dict, function(){
		console.log('Sent config data to Pebble'); 
		
		// save dict to localStorage to use later
		localStorage.config = JSON.stringify(dict);
		
	}, function() {
		console.log('Failed to send config data!');
	});
});