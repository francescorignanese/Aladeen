const fs = require("fs");
const SerialPort = require('serialport');
const port = new SerialPort('/COM7', {databits:8, parity:'none'});

var config = require('config');
var Client = require('azure-iot-device').Client;
var Protocol = require('azure-iot-device-mqtt').Mqtt;
var clientConfig = config.get('Clients');

var client = Client.fromConnectionString(clientConfig[0].connectionString, Protocol);

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
        console.log(twin.properties);
        twin.on('properties.desired', function(delta) {
            console.log('new desired properties received:');
            console.log(JSON.stringify(delta));
            fs.writeFileSync("temps.json", JSON.stringify(twin.properties.desired.timing));
        });
        
        //sendTiming(twin.properties.desired.timing);
        
  });
});

function sendTiming(timings) {
    let data = timings;
    console.log(data);


    /*let cmd = [0x08, 0x00, 0x00, 0x00, 0x00];
    port.write(cmd);
    console.log('Sent value to Pic:', cmd);*/
}
