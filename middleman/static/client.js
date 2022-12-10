const socket = io()
console.log("socket constant created")

function init(){
    //initialization for sockets .on for connect.

	//socket.on 'connect'; simply console log relevant message
	socket.on('connect', function(){
		console.log("socket has been connected")
	});
	//socket.on 'response'; console log the data and call updateDiv(data)
	socket.on('display', function(data){
		//console.log("Received Data")
		//console.log(data)
		updateDiv(data);
	});
	//socket.on 'disconnect'; simply console log relevant message
	socket.on('disconnect', function(){
		console.log("socket has been disconnected")
	});
}

// called whenever the button is pressed
function messagesend(){
	// get data from input box into variable
	let throttleGet = document.getElementById("messageInput").value
	// socket emit to "throttle", with this format:
	socket.emit("throttle", {throttle:throttleGet});
	console.log("sent")

	// update the throttle on the page, with extra warning if server overrides
	let extrapart = ""
	if (parseFloat(throttleGet) < 0) {
		extrapart = " (server will override to 0)"
	} else if (parseFloat(throttleGet) > 1) {
		extrapart = " (server will override to 1)"
	}
	document.getElementById("throttle_div").innerHTML = throttleGet + extrapart;
}

// this will be called by putting it inside the socket.on for "response"
function updateDiv(data){
    // set inner html of item with results_div id using data
	document.getElementById("results_div").innerHTML = data;
}
