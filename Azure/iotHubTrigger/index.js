module.exports = function (context, IoTHubMessages) {
    context.log(`JavaScript eventhub trigger function called for message array: ${IoTHubMessages[0]}`);

    context.bindings.signalRMessages = [{
        "target": "message",
        "arguments": [ IoTHubMessages[0] ]
    }];
    
    /*
    IoTHubMessages.forEach(message => {
        context.log(`Processed message: ${message}`);
        console.log(message);

    });
    */
    context.log(IoTHubMessages[0])
    context.done();
};