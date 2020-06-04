var config = require('config');
'use strict';

var uuid = require('uuid');
var Protocol = require('azure-iot-device-mqtt').Mqtt;
var Client = require('azure-iot-device').Client;
var Message = require('azure-iot-device').Message;
var clientConfig = config.get('Clients');


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
      temp: 20,
      traffic: 150
    }));
    // A message can have custom properties that are also encoded and can be used for routing
    //message.properties.add('propertyName', 'propertyValue');

    // A unique identifier can be set to easily track the message in your application
    message.messageId = uuid.v4();

    console.log('Sending message: ' + message.getData());
    client.sendEvent(message, function (err) {
      if (err) {
        console.error('Could not send: ' + err.toString());
        process.exit(-1);
      } else {
        console.log('Message sent: ' + message.messageId);
        process.exit(0);
      }
    });
  }

});