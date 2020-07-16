const signalR = require("@microsoft/signalr");
module.exports = function (context, IoTHubMessages) {
    context.log(`JavaScript eventhub trigger function called for message array: ${IoTHubMessages[0]}`);
    
   context.log(IoTHubMessages[0].sensor);
   context.log(IoTHubMessages[0]);
   var connection = new signalR.HubConnectionBuilder()
    .withUrl("http://localhost:15000/telemetry")
    .withAutomaticReconnect()
    .build();
    connection.start()
    .then(() => {
        context.log("Successfully connected");
        connection.invoke('SendMessage',IoTHubMessages[0].sensor.toString(), JSON.stringify(IoTHubMessages[0])); //invoke hub function 
        }).then(() =>{
            context.done();
        })
        .catch((err) =>{
            context.log("invoke error: ",err)
        })
    .catch((err) => {
        context.log("Unable to connect : ",err);
    })
    context.done();
};