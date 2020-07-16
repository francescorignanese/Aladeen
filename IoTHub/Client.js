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

    // TRAFFIC MESSAGE

    var message = new Message(JSON.stringify(
    {
      "description": "Dati unificati traffico",
      "sensor": "traffic",
      "id_cross": 1,
      "date": new Date().toISOString().slice(0,10),
      "time": '18:18:18',//new Date().toISOString().slice(11,19)
      "data_carriers": [
        {
          "id_road": 1,
          "id_semaphores": 1, //coppia 0 o 1
          "data_vehicles": [
            {
              "type": "Automobile",  //car, heavy, moto
		          "value": 20
            },
            {
              "type": "Motociclo",  //car, heavy, moto
		          "value": 20
            },
            {
              "type": "Mezzo Pesante",  //car, heavy, moto
		          "value": 13
            }
          ]
        },
        {
          "id_road": 2,
          "id_semaphores": 0, //coppia 0 o 1
          "data_vehicles": [
            {
              "type": "Automobile",  //car, heavy, moto
		          "value": 50
            },
            {
              "type": "Motociclo",  //car, heavy, moto
		          "value": 30
            },
            {
              "type": "Mezzo Pesante",  //car, heavy, moto
		          "value": 11
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
    

    //CLIMATE MESSAGE

    var message1 = new Message(JSON.stringify(
    {
      "description": "Dati unificati traffico",
      "sensor": "climate",
      "id_cross": 1,
      "date": new Date().toISOString().slice(0,10),
      "time": new Date().toISOString().slice(11,19),
      "data_climate": [
        {
            "type": "pressure",
            "value": 10
        },
        {
          "type": "humidity",
          "value": 70
        },
        {
          "type": "temperature",
          "value": 24
        }
      ]
    }));

    message1.contentEncoding = "utf-8";
    message1.contentType = "application/json";

    // A unique identifier 
    message1.messageId = uuid.v4();

    //add custom properties
    message1.properties.add("Status", "Active");

    //console.log('Sending message: ' + message.getData());
    
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

/*
let json_climate = 
{
  "description": "Dati unificati traffico",
  "sensor": "climate",
  "id_cross": 1,
  "date": new Date().toISOString().slice(0,10),
  "time": new Date().toISOString().slice(11,19),
  "data_climate": [
    {
        "type": "pressure",
        "value": 10
    },
    {
      "type": "humidity",
      "value": 70
    },
    {
      "type": "temperature",
      "value": 24
    }
  ]
}
*/