//purpose: middleman, act as a client to C/QNX, act as server to browser

//you need to use npm install and run node middleman.js to run this.
//when the C server stops, the middleman continues running because...
//...express is unrelated to the C<->middleman communication. express is...
//... just for middleman<->client/browser.
//send the message 'stop' to stop the C server.
//use POSTMAN to test sending a get request if you cant use the frontend.
//for example, GET http://localhost:3000/checksum?message=test .
//the middleman can only so far connect to the server when it is run for the...
//...first time.
//if the middleman dies while it is connected to the server, the server may...
//...probably enter an infinite loop.

const express = require('express'); //to make middleman serve browser/client
const net = require('net'); //to talk to C/QNX
const fs = require("fs"); //so the middleman can read files
const app = express();
const http = require('http').createServer(app);
// Create a Socket.IO instance, passing it our server
const socket = require('socket.io')(http)

app.use(express.static("static")); //serve client resources
app.use(express.json()); //automatically parse json

//const ip = require('ip');
//console.log(ip.address())
//ADDRESS = ip.address()
//PORTNUM = 3000
ADDRESS = "192.168.56.103"  // Address assigned by host bridge for QNX VM
PORTNUM = 6000  // The port used by the server

//for HTTP to work we are forced to use global variables and waits
//ideally we drop HTTP for websockets
var connected = false;
var message = undefined;
var sent = false;
let receivingRPM;
let throttle = 0.0;

/***
 * The convert function converts the data received from the server into a readable format and updates
 * the struct.
 * [NOTE: I still need to add more stuff in the struct so this will keep getting updated]
 */

function convert(data) {
  const buffer = Buffer.from(data);
  let offset = 0;
  receivingRPM = buffer.readDoubleLE(offset);
  //console.log("The rpm is : ", receivingRPM);
  return receivingRPM
}

socket.on('connection', function(client){
    console.log('Connection event listener created')
    console.log("Id of client was:", client.id)

    client.on('throttle', (data) => {
        throttle = parseFloat(data["throttle"])
    })

    client.on('disconnect', function(){
        console.log('Server has disconnected');
        console.log("Id of client was:", client.id)
    })
})

//communication with C/QNX
var client = new net.Socket();
client.connect(PORTNUM, ADDRESS, () => {
    console.log('connected to server?');
    connected = true;
    //client.write('test');
})
client.on('data', (data) => {
  //data = convert(data2)
  //console.log("from server: " + data.toString());
  receivingRPM = parseFloat(convert(data))
  //console.log(receivingRPM)
  /*message = data.toString();
  sent = true;
  if (data.toString() == "middlestop") {
    console.log("ok i am middleman stopping now")
    client.end();
  }*/
  //send back just as soon as we receive; talk back and forth
  sendToServer()
  socket.emit("display", receivingRPM)
});
client.on('end', () => {
  console.log('disconnected from server');
  connected = false;
});

function sendToServer() {
    const buffer = Buffer.alloc(4)
    const givefloat = parseFloat(throttle)
    buffer.writeFloatLE(givefloat, 0)
    client.write(buffer)
}

//start the first call, then the one in client.on('data') will do the rest
sendToServer()

//communication with client (placeholder using HTTP for now)
app.get("/engine", [giveHTML])//, queryRefine, processQuery]);

function giveHTML(req, res, next) {
	res.format({
		"application/json": () => {
            //you may need to set headers to get here
			console.log("try to give json")
            res.status(500).send("Server Error");
            //next()
		},
		"text/html": () => {
			console.log("try to give html")
            //the html is raw and will need client.js to get json
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

/*function queryRefine(req, res, next) {
    let query = req.query
    if (!(query.hasOwnProperty("message"))) {
        res.status(400).send("you need a query parameter called message")
        return
    }
    res.locals.messageValue = query.message
    next();
}

function processQuery(req, res, next) {
    if (connected == false) {
        res.status(500).send("error: C/QNX server unreachable")
        console.log("error: C/QNX server unreachable")
    } else {
        console.log("send message to server")
        const buffer = Buffer.alloc(4)
        const givefloat = parseFloat(res.locals.messageValue)
        console.log("send float: " + givefloat)
        buffer.writeFloatLE(givefloat, 0)

        client.write(buffer)

        console.log("waiting for response from C")
        let time = 1;
        function waitForMsg() {
            if (sent == true) {
                console.log(`waited ${time}ms, got response`)
                res.status(200).json({message:message})
                sent = false;
                console.log("message sent to client")
            } else {
                //console.log(`waited ${time}ms, nothing yet`)
                time += 1;
                setTimeout(waitForMsg, 1)
            }
        }
        let timeout = setTimeout(waitForMsg, 1)
    }
}*/


http.listen(3000);
console.log("Server listening at http://localhost:3000");
