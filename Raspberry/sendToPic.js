const SerialPort = require('serialport')
const port = new SerialPort('/COM6', {databits:8, parity:'none'});

let buffer3=
[
	0xA0,
	0x00,
	0x03,
	0x00,
	0xA3,
	0xC0,
	0x00,
	0x87,
	0x00,
	0x47,
	0x60,
	0x00,
	0x0F,
	0x00,
	0x6F
]
port.write(buffer3)