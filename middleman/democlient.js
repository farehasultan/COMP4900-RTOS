const net = require('net');
const ip = require('ip');
console.log(ip.address())
ADDRESS = ip.address()
PORTNUM = 3000

var client = new net.Socket();
client.connect(PORTNUM, ADDRESS, () => {
    console.log('connected to server?');
    client.write('test');
})
client.on('data', (data) => {
  console.log("from server: " + data.toString());
  if (data.toString() == "clientstop") {
    console.log("ok i am client stopping now")
    client.end();
  }

});
client.on('end', () => {
  console.log('disconnected from server');
});
