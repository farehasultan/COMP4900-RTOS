import { io } from "socket.io-client";
const socket = io()
console.log("socket constant created")

function init(){
    //initialization for sockets .on for connect.
	socket.on('connect', function(){

    //socket.on 'connect'; simply console log relevant message
	console.log("socket has been connected")
});
    //socket.on 'response'; console log the data and call updateDiv(data)
	socket.on('response', function(){
	console.log("Data")
});
    //socket.on 'disconnect'; simply console log relevant message
	socket.on('disconnect', function(){
	console.log("socket has been disconnected")
});

}
const messageButton =  document.querySelector("#messagesend");
console.log(messageButton);

//messageButton.addEventListener("click", messagesend);
function messagesend(){
	
    //get data from input box into variable
	//var inputData = 
	//document.getElementbyId('textbox_id').value;
	//document.addEventListener('keypress', (event) => {
	//	var name = event.key;
	//	var code = event.code;
		// Alert the key name and key code on keydown
		//alert(`Key pressed ${name} \r\n Key code value: ${code}`);
	 // }, false);
	
	
	//console.log("1");
	
	

	
	


    //socket emit to "checksum", with this format:
	socket.emit("checksum", {message:inputData});
    //{message: <variable from above>}
    //this wont work until I implement websockets into the server

}

//this will be called by putting it inside the socket.on for "response"
function updateDiv(data){
    //set inner html of item with results_div id using data


}


