const SerialPort = require('serialport');
const port = new SerialPort('/COM7', {databits:8, parity:'none'});

//RICHIESTA DATI TRAFFICO

let wanted_traffik = true;

if(wanted_traffik)
{ 
    let traffic_cmd = [0x08, 0x00, 0x00, 0x00, 0x00];
    port.write(traffic_cmd);
    console.log('Sent value to Pic:', traffic_cmd);
}

