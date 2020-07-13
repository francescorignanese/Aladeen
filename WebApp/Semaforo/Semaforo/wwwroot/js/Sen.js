"use strict";

var connection = new signalR.HubConnectionBuilder().withUrl("/smartcross").build();


connection.on("ReceiveMessage", function (user, message) {
    
    var msg = message.replace(/&/g, "&amp;").replace(/</g, "&lt;").replace(/>/g, "&gt;");
    var encodedMsg = user + " says " + msg;
    var li = document.createElement("li");
    li.textContent = encodedMsg;
    console.log(message);
   

});
connection.start()
    .then(function () {
        onConnected(connection);
    })
    .catch(function (error) {
        console.error(error.message);
    });