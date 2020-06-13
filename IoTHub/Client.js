var config = require('config');
'use strict';

var uuid = require('uuid');
var Protocol = require('azure-iot-device-mqtt').Mqtt;
var Client = require('azure-iot-device').Client;
var Message = require('azure-iot-device').Message;
var clientConfig = config.get('Clients');
//console.log(clientConfig)

var client = Client.fromConnectionString(clientConfig[0].connectionString, Protocol);

// open connection
client.open(function (err) {
  if (err) {
    console.error('Could not connect: ' + err.message);
  }
  else {
    console.log('Client connected');

    client.on('error', function (err) {
      console.error(err.message);
      process.exit(-1);
    });

    // build message
    var message = new Message(JSON.stringify({
      typemessage:"sensors",
      value:{
          temperature: '40.0',
          umidity: '88.0',
          pressure: '20.0'
        }
    }));
    
    message.contentEncoding = "utf-8";
    message.contentType = "application/json";

    // A unique identifier 
    message.messageId = uuid.v4();

    //add custom properties
    message.properties.add("Status", "Active");
    

    console.log('Sending message: ' + message.getData());
    client.sendEvent(message, function (err) {
      if (err) {
        console.error('Could not send: ' + err.toString());
        process.exit(-1);
      } else {
        console.log('Message sent: ');
        process.exit(0);
      }
    });
  }

});