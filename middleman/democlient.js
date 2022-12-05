const net = require('net');
const ip = require('ip');
const sharedStructs = require('shared-structs'); //SHARED STRUCTS 
const strings = require('shared-structs/string');
const { Buffer } = require('node:buffer');




console.log(ip.address()) //
ADDRESS = "192.168.56.103" //adress assigned 
PORTNUM = 6000

/**
 * Shared struct code from:
 * https://www.npmjs.com/package/shared-structs?activeTab=readme
 * The struct below will be shared between the server and the client.
 * Engine start is a boolean where 1 means it's running and 0 means it has stopped. 
 * The values for the engine start are toggled everytime the 'W' key is pressed. 
 * -rpm, an F1 can have up to 20,000 revolutions per minute (rpm)
 */
const structs = sharedStructs(`
  struct aStruct {
    int engine_start; 
    char someString[255];
    float rpm;
    int fuel_amount;
  }
`)

const car = structs.aStruct();
car.engine_start = 1; //would be controlled by a keyboard using event handler.
strings.encode('Vroom', car.someString);



var client = new net.Socket();
client.connect(PORTNUM, ADDRESS, () => {
  console.log('connected to server?');
    //client.write('test');
  client.write(car.rawBuffer);
  console.log("Client Sent Data!");
})

function fireStart() {
  car.engine_start = 1;
  client.write(car.rawBuffer);
  console.log("Client Sent Data!");
}
function stop() {
  car.engine_start = 0;
  client.write(car.rawBuffer);
  console.log("Client Sent Data!");
}



// pass this to c, and it will be able to parse it
// console.log(car.rawBuffer)
// console.log(car.engine_start);
// console.log(strings.decode(car.someString));




client.on('data', (data) => {
  convert(data.buffer); //here
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
 * [NOTE: I still need to add more stuff in the struct so this will keep getting updated]
 */
function convert(data) {
  const buffer = Buffer.from(data);
  let offset = 0;
  
  car.engine_start = buffer.readInt32LE(offset);
  offset += 4;
  strings.encode(buffer.toString('utf8', offset), car.someString);
  offset += 252;
  // float rpm;
  // int fuel_amount;

  console.log("[CLIENT]: Receive from server");
  console.log(car.engine_start);
  console.log(strings.decode(car.someString));
  
}