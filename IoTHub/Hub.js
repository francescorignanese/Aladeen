'use strict';
var config = require('config');

/**
 *  Programma di invio della temporizzazione del semaforo dall'hub al dispositivo selezionato
 */



var Client = require('azure-iothub').Client;
var Message = require('azure-iot-common').Message;

var clientConfig = config.get('Clients');

var serviceClient = Client.fromConnectionString(config.get('Hub.connectionString'));


//inviare un messagio di esempio ad un dispositivo
serviceClient.open(function (err) {
  if (err) {
    console.error('Could not connect: ' + err.message);
  } else {
    console.log('Service client connected');
    serviceClient.getFeedbackReceiver(receiveFeedback);

    var message = new Message('Test message');
    //message.ack = 'full';
    //message.messageId = "My Message ID";
    //console.log('Sending message: ' + message.getData());
    serviceClient.send(clientConfig[0].id, message, printResultFor('send'));
  }
});
