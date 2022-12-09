const net = require('net');
const ip = require('ip');
const { Buffer } = require('node:buffer');

console.log(ip.address()) //
ADDRESS = "192.168.56.103" //adress assigned 
PORTNUM = 6000


let sendThrottle = Math.PI;
let receivingRPM;
const buf = Buffer.alloc(4);

var client = new net.Socket();
client.connect(PORTNUM, ADDRESS, () => {
  console.log('connected to server?');
  buf.writeFloatLE(sendThrottle, 0);
  client.write(buf);
  console.log("Client Sent Data!");
})


client.on('data', (data) => {
  convert(data);
  if (data.toString() == "clientstop") {
    console.log("ok i am client stopping now")
    client.end();
  }
});

client.on('end', () => {
  console.log('disconnected from server');
});

/***
 * The convert function converts the data received from the server into a readable format and updates
 * the struct.
 */

function convert(data) {
  const buffer = Buffer.from(data);
  let offset = 0;
  receivingRPM = buffer.readDoubleLE(offset);
  console.log("The receiving rpm is : ", receivingRPM);
  
}

