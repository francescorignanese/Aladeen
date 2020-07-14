const signalR = require("@microsoft/signalr");
module.exports = function (context, IoTHubMessages) {
    context.log(`JavaScript eventhub trigger function called for message array: ${IoTHubMessages[0]}`);
    
    /*
    IoTHubMessages.forEach(message => {
        context.log(`Processed message: ${message}`);
    });
    */
   context.log(IoTHubMessages[0].sensor);
   context.log(IoTHubMessages[0]);
   var connection = new signalR.HubConnectionBuilder()
    .withUrl("http://localhost:15480/telemetry")
    .build();
    connection.start()
    .then(() => {
        context.log("Successfully connected");
        connection.invoke('SendMessage',IoTHubMessages[0].sensor.toString(), IoTHubMessages[0].toString()); //invoke hub function 
        }).then(() =>{
            context.log("send")
        })
        .catch((err) =>{
            context.log("invoke error: ",err)
        })
    .catch((err) => {
        context.log("Unable to connect : ",err);
    })
    //context.done();
};