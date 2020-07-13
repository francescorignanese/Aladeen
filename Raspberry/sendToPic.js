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

   
});

let tookTime = false;
if(!tookTime) {
    getTimings();
}
function getTimings() {
 // Create device Twin
    var receiveTimings = new Promise(function(resolve, reject) { 
        client.getTwin(function(err, twin) {
            if (err) {
                console.error('error getting twin: ' + err);
                process.exit(1);
            }
            // Output the current properties
            console.log('twin contents:');
            console.log(twin.properties);
            twin.on('properties.desired', function(delta) {
                console.log('new desired properties received:');
                let json_timings = twin.properties.desired;
                let json_string = JSON.stringify(twin.properties.desired);
                clientRedis.rpush(['timings', json_string]);
                fs.writeFileSync("temps.json", JSON.stringify(twin.properties.desired));
                resolve(json_timings);
            });
            
            //sendTiming(twin.properties.desired.timing);
        });
    });
    tookTime = true;
    return receiveTimings;
}




function parseTimings() {
        let reply = require('./temps.json');
        //console.log(reply);
        
        //let JSON_received = JSON.parse(reply[0]);
        let arrayTimings = reply.timing;
        
        let hour = new Date().toISOString().slice(11, 13);
        let firstCouple = arrayTimings.shift();
        let secondCouple = arrayTimings.shift();
        
        let couples = [firstCouple, secondCouple];

        couples.forEach(couple => {
            if(couple.semafores_couples === 0) {
                //l'id del semaforo sarà 2: 
                //verde: 01100101
                //giallo: 01000101
                //rosso: 00100101
                let yellowTemp = 5;
                let redTemp = 5 + couple.value;
                let greenValue = parseInt(`0x${Number(couple.value).toString(16)}`);
                let yellowValue = parseInt(`0x${Number(yellowTemp).toString(16)}`);
                console.log(greenValue, yellowValue);
                let redValue = redTemp.toString(16);
                
                let greenId = 0x65;
                console.log(greenId);
                let yellowId = 0x45;
                let redId = 0x25;
                let bit_parità = 0x00; //DA CALCOLARE:... 

                let greenPack = [greenId, 0x00, greenValue, 0x00, bit_parità];
                let yellowPack = [yellowId, 0x00, yellowValue, 0x00, bit_parità];
                let redPack = [redId, 0x00, redValue, 0x00, bit_parità];
                //let megaPack = [greenId, 0x00, greenValue, 0x00, bit_parità, yellowId, 0x00, yellowValue, 0x00, bit_parità, redId, 0x00, redValue, 0x00, bit_parità];
                let megaPack = [0x20,0x00,greenValue,0x00,0x00, 0x40,0x00,0x0A,0x00,0x00, 0x60,0x00,0x0A,0x00,0x00];
                //primo rosso - 15 sec, verde - 10 sec, giallo -5 sec
                console.log('prova', megaPack);
                port.write(megaPack);
                let megaPack2 = [0x22,0x00,greenValue,0x00,0xA5, 0x42,0x00,0x0A,0x00,0xC5, 0x62,0x00,0x05,0x00,0x65];
                port.write(megaPack2);
                console.log('pacchetti inviati');
            } else if (couple.semafores_couples === 1) {
                //l'id del semaforo sarà 1 o 3
            }
        });
        
    

       // console.log(arrayTimings);
        //let firstTime = returnedValue.shift();
        //1 - semaforo 1 a rosso: 00100011
        //2 - vuoto
        // 3 e 4 - 00000000 00001111 15 secondi
        // 5 - 00000000
        /*let packetGreen = [01100011, 00000000, 00001111, 00000000, 01101100];
        let packetYellow = [11000011, 00000000, 00000101, 00000000, 11000110];
        let packetRed = [10100011, 00000000, 00011110, 00000000, 00111111];*/

        /*let packetGreen = [0x63, 0x00, 0x0F, 0x00, 0x6C];
        let packetYellow = [0xC3, 0x00, 0x05, 0x00, 0xC6];
        let packetRed = [0xA3, 0x00, 0x1E, 0x00, 0x3F];

        port.write(packetGreen);
        console.log('mandato pacco verde');
        port.write(packetYellow);
        console.log('mandato pacco giallo');
        port.write(packetRed);
        console.log('mandato pacco rosso');*/

    
   
}


setInterval(() => {
	parseTimings();
	
}, 5000);
