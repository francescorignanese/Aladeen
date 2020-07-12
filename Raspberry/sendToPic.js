const fs = require("fs");
const SerialPort = require('serialport');
const port = new SerialPort('/COM7', {databits:8, parity:'none'});
const redis = require('redis');
var config = require('config');
var Client = require('azure-iot-device').Client;
var Protocol = require('azure-iot-device-mqtt').Mqtt;
var clientConfig = config.get('Clients');
const clientRedis = redis.createClient(6379, '192.168.0.99');
var client = Client.fromConnectionString(clientConfig[0].connectionString, Protocol);

//Connection to Redis
clientRedis.on("error", (err) => {
    console.log("error", err)
});
clientRedis.on("connect", (err) => {
    console.log("Redis connected!");
});
clientRedis.on("ready", (err) => {
    
    console.log("Redis ready to accept data!");
});

client.on('error', function (err) {
  console.error(err.message);
});

// connect to the hub
client.open(function(err) {
    if (err) {
        console.error('error connecting to hub: ' + err);
        process.exit(1);
    }
    console.log('client opened');

    // Create device Twin
    client.getTwin(function(err, twin) {
        if (err) {
            console.error('error getting twin: ' + err);
            process.exit(1);
        }
        // Output the current properties
        console.log('twin contents:');
        //console.log(twin.properties);
        twin.on('properties.desired', function(delta) {
            console.log('new desired properties received:');
            //console.log(JSON.stringify(delta));
            let json_string = JSON.stringify(twin.properties.desired.timing);
            clientRedis.rpush(['timings', json_string]);
            fs.writeFileSync("temps.json", JSON.stringify(twin.properties.desired.timing));
        });
        
        //sendTiming(twin.properties.desired.timing);
  });
});

//ricevo la temporizzazione per le 24 ore per la coppia di semafori 
function getDataFromRedis() {
    var popRedis = new Promise(function(resolve, reject) { 
        clientRedis.lpop(['timings'], function (err, reply) {
            //console.log("Popped item", reply);
            resolve(reply);
        });
    });
    return popRedis;
}


function parseTimings() {
    getDataFromRedis().then(function(returnedValue) {
        console.log('array', returnedValue);
        //let firstTime = returnedValue.shift();
        //1 - semaforo 1 a rosso: 00100011
        //2 - vuoto
        // 3 e 4 - 00000000 00001111 15 secondi
        // 5 - 00000000
        /*let packetGreen = [11100011, 00000000, 00001111, 00000000, 11101100];
        let packetYellow = [11000011, 00000000, 00000101, 00000000, 11000110];
        let packetRed = [10100011, 00000000, 00011110, 00000000, 10111111];*/

        let packetGreen = [0xE3, 0x00, 0x0F, 0x00, 0xEC];
        let packetYellow = [0xC3, 0x00, 0x05, 0x00, 0xC6];
        let packetRed = [0xA3, 0x00, 0x1E, 0x00, 0xBF];

        port.write(packetGreen);
        console.log('mandato pacco verde');
        port.write(packetYellow);
        console.log('mandato pacco giallo');
        port.write(packetRed);
        console.log('mandato pacco rosso');

    });
   
}

parseTimings();

// per tutte le ore vado a prendermi i valori e li parso in HEX 

// li invio al pic


function sendTiming(timings) {
    let data = timings;
    //console.log(data);


    /*let cmd = [0x08, 0x00, 0x00, 0x00, 0x00];
    port.write(cmd);
    console.log('Sent value to Pic:', cmd);*/
}
