const net = require('net');
const ip = require('ip');
console.log(ip.address())
ADDRESS = ip.address()
PORTNUM = 3000

var server = net.createServer(); //we are server, not client
server.listen(PORTNUM, ADDRESS, (client) => {
    //console.log(`opened server on port:${PORTNUM}, address:${server.address().address}`)
    console.log("opened server with ", server.address())

})
server.on("connection", (client) => {
    console.log(`new connection from ${client.remoteAddress}:${client.remotePort}`)
    client.on("data", (data) => {
        console.log("from client: " + data.toString());
        if (data.toString() == "test") {
          client.write("clientstop")
        }
    })
    client.on("close", () => {
        console.log("client closed?")
    })
    client.on("error", (err) => {
        console.log("error apparently received:")
        console.log(err)
    })
    //server.write("data");
    client.write("hellothere, i am server")
})

server.on("error", (e) => {
    if (e.code === "EADDRINUSE") {
        console.log("Address already in use. Close and reopen?")
        setTimeout(() => {
            server.close()
            server.listen(PORTNUM, ADDRESS);
        }, 1000)
    } else {
        console.log("server has error? here:")
        console.log(e)
    }
})
