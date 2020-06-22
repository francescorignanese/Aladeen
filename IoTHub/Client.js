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

    // var data= new Date();
    // var ora= data.getHours.toString()+":"+data.getMinutes.toString()+":"+data.getSeconds.toString();
    // // build message
    var message = new Message(JSON.stringify(
    {
      "description": "Dati unificati traffico",
      "sensor": "traffic",
      "id_cross": 1,
      "date": new Date(),
      "time": new Date(),
      "data_carriers": [
        {
          "id_road": 1,
          "id_semaphores": 1, //coppia 0 o 1
          "data_vehicles": [
            {
              "type": "Automobile",  //car, heavy, moto
		          "value": 12
            },
            {
              "type": "Motociclo",  //car, heavy, moto
		          "value": 13
            },
            {
              "type": "Mezzo Pesante",  //car, heavy, moto
		          "value": 15
            }
          ]
        },
        {
          "id_road": 2,
          "id_semaphores": 0, //coppia 0 o 1
          "data_vehicles": [
            {
              "type": "Automobile",  //car, heavy, moto
		          "value": 16
            },
            {
              "type": "Motociclo",  //car, heavy, moto
		          "value": 17
            },
            {
              "type": "Mezzo Pesante",  //car, heavy, moto
		          "value": 18
            }
          ]
        }

        
      ]
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