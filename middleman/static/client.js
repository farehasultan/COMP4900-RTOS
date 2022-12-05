const socket = io()

console.log("socket constant created")

function init(){
    //initialize sockets .on for connect, response, disconnect

    //socket.on 'connect'; simply console log relevant message
    //socket.on 'response'; console log the data and call updateDiv(data)
    //socket.on 'disconnect'; simply console log relevant message
}

function messagesend(){
    //get data from input box into variable

    //socket emit to "checksum", with this format:
    //{message: <variable from above>}
    //this wont work until I implement websockets into the server

}

//this will be called by putting it inside the socket.on for "response"
function updateDiv(data){
    //set inner html of item with results_div id using data
}

//EVENT LISTENERS FOR W KEY:
let w = document.getElementById('testing-keys');

  w.addEventListener('keydown', (e) => {
    if (e.key === 'w' && !e.repeat) {
      console.log(`Key "${e.key}" pressed [event: keydown]`);
        fireStart();
    } else {
      if (e.key === 'w') {
        console.log(`Key "${e.key}" repeating [event: keydown]`);
      }
    }
  });


textarea.addEventListener('keyup', (e) => {
  if (e.key === 'w') {
    console.log(`Key "${e.key}" released [event: keyup]`);
      stop();
  }
});