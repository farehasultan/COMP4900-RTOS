// Purpose: middleman, act as a client to C/QNX, act as server to browser

/***
* You need to use npm install (or npm i) and run node middleman.js to run this.
* You need to have ./backend running before you run ./middleman because
*  the middleman will immediately try to connect to the backend upon being run.
* You need to manually end the backend or the middleman; there are no commands
*  to stop them.
* The middleman requires you to use the client to interact with it and the
*  server. http://localhost:3000/engine will bring you to the client page.
* The middleman will initiate a back and forth loop between itself and the
*  server, and the timing is based on the latency between messages being
*  sent. This might be within the range of 0.5-2ms.
* If any clients are connected to the middleman via a WebSocket, the client
*  will be sent each of these updates as well.
*/

ADDRESS = "192.168.56.103"  // Address assigned by host bridge for QNX VM
PORTNUM = 6000  // The port used by the (backend) server

const express = require('express'); // to make middleman serve browser/client resources
const net = require('net'); // to talk to C/QNX
const fs = require("fs"); // so the middleman can read files
const app = express(); // basic core functionality
const http = require('http').createServer(app); // so the middleman can talk to client
const socket = require('socket.io')(http)// Create a Socket.IO instance for the middleman

app.use(express.static("static")); // serve client resources using folder "static"
app.use(express.json()); // automatically parse json

// uncomment this if you would like to see your IP address (but it won't be useful)
// because you need the one from QNX
//const ip = require('ip');
//console.log(ip.address())

let connected = false;
let receivingRPM; // updated by backend<->middleman
let throttle = 0.0; // updated by middleman<->client

/***
 * The convert function converts the data received from the server into a format
 *  readable by the middleman.
 */
function convert(data) {
  const buffer = Buffer.from(data);
  let offset = 0;
  receivingRPM = buffer.readDoubleLE(offset);
  //console.log("The rpm is : ", receivingRPM);
  return receivingRPM
}

/***
 * The sendToServer function converts the data the middleman has into a format
 *  readable by the server (backend).
 */
function sendToServer() {
    const buffer = Buffer.alloc(4)
    const givefloat = parseFloat(throttle)
    buffer.writeFloatLE(givefloat, 0)
    client.write(buffer)
}

// communication wth client
socket.on('connection', function(client){
    console.log('Connection event listener created')
    console.log("Id of client was:", client.id)

    client.on('throttle', (data) => {
        // set the global variable to whatever the client sent us
        throttle = parseFloat(data["throttle"])
    })

    client.on('disconnect', function(){
        console.log('Server has disconnected');
        console.log("Id of client was:", client.id)
    })
})

// communication with C/QNX
var client = new net.Socket();
client.connect(PORTNUM, ADDRESS, () => {
    console.log('connected to server (backend)');
    connected = true;
})
client.on('data', (data) => {
    receivingRPM = parseFloat(convert(data))
    // send back to backend just as soon as we receive; talk back and forth
    sendToServer()
    // send to client:
    socket.emit("display", receivingRPM)
});
client.on('end', () => {
    console.log('disconnected from server (backend)');
    connected = false;
});

//communication with client (placeholder using HTTP for now)
app.get("/engine", [giveHTML]);

// give client.html whenever the client has /engine in browser
function giveHTML(req, res, next) {
	res.format({
		"application/json": () => {
            // you may need to set headers to get here
			console.log("try to give json");
            // there is no json, it's an error
            res.status(500).send("Server Error");
		},
		"text/html": () => {
			console.log("try to give html")
			fs.readFile(`./static/client.html`, function(error, data){
			  if(error){
				res.status(500).send("Server Error");
				//throw error;
				return;
			  }
			  res.status(200).send(data);
			})
		}
	})
}

// start the first call, then the one in client.on('data') will do the rest
sendToServer();

http.listen(3000);
console.log("Server listening at http://localhost:3000");
