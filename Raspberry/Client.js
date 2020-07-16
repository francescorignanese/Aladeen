var config = require('config');
'use strict';
var uuid = require('uuid');
const redis = require('redis');
var Protocol = require('azure-iot-device-mqtt').Mqtt;
var Client = require('azure-iot-device').Client;
var Message = require('azure-iot-device').Message;
var clientConfig = config.get('Clients');
const redisClient = redis.createClient(6379, '192.168.0.99');
redisClient.on("error", (err) => {
    console.log("error", err)
});
redisClient.on("connect", (err) => {
    console.log("Redis connected!");
});
redisClient.on("ready", (err) => {
    redisNotReady = false;
    console.log("Redis ready to pop data!");
});
//console.log(clientConfig)

//Invia richiesta Clima ogni 60 secondi
setInterval(() => {
	redisClient.lpop(['climate'], function (err, reply) {
        console.log("Popped item", reply);
        sendData(reply);
    });
}, 60000);

//Invia richiesta traffico ogni 40 secondi
setInterval(() => {
	redisClient.lpop(['traffic'], function (err, reply) {
        console.log("Popped item", reply);
        sendData(reply);
    });
}, 40000);


function sendData(resMex) {
    if(resMex !== null) {

        var client = Client.fromConnectionString(clientConfig[0].connectionString, Protocol);

        // open connection
        client.open(function (err) {
        if (err) {
            console.error('Could not connect: ' + err.message);
        } else {
            console.log('Client connected');
            client.on('error', function (err) {
            console.error(err.message);
            process.exit(-1);
        });


        //CLIMATE MESSAGE
        console.log(resMex);
        console.log('----------------------------------');
        var message1 = new Message(resMex);
        console.log('mex', message1);

        message1.contentEncoding = "utf-8";
        message1.contentType = "application/json";

        // A unique identifier 
        message1.messageId = uuid.v4();

        //add custom properties
        message1.properties.add("Status", "Active");

        console.log('Sending message: ' + message1.getData());

        client.sendEvent(message1, function (err) {
            if (err) {
            console.error('Could not send: ' + err.toString());
            process.exit(-1);
            } else {
            console.log('Message sent: ', message1);
            //process.exit(0);
            }
        });
        }

        });
    } else {
        console.log('messaggio vuoto');
    }
}
