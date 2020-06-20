const fs = require("fs");
const SerialPort = require('serialport');
const ByteLength = require('@serialport/parser-byte-length');
const redis = require('redis');
const port = new SerialPort('/COM7');
const client = redis.createClient(6379, '192.168.0.99');
let key;
let sensor;

//-----------------------------------------------------------------------------------------------
//RICHIESTA DATI TRAFFICO
function sendToPic() {

	let wanted_traffik = true;

	if(wanted_traffik)
	{ 
		let traffic_cmd = [0x0A, 0x00, 0x00, 0x00, 0x00];
		port.write(traffic_cmd);
		console.log('Sent value to Pic:', traffic_cmd);
		
	}
}
sendToPic();

//con questo parser lavoriamo con 5 byte alla volta
const parser = port.pipe(new ByteLength({length: 5}))
parser.on('data', parseMsg);
client.on("error", (err) => {
		console.log("error", err)
	 });
client.on("connect", (err) => {
	console.log("Redis connected!");
});
client.on("ready", (err) => {
	redisNotReady = false;
	console.log("Redis ready to accept data!");
});


function parseMsg(data) {
	console.log('dataaaaa', data);
	//per ogni elemento del buffer ricevuto via seriale, estraiamo il valore in binario
	let msgSize = data.length;
	for (let i = 0; i < msgSize; i++) {
		let valore = parseInt(data[i], 10).toString(2);
		console.log(data[i]);
		//aggiungiamo gli 0 omessi perchè non significanti
		console.log(valore.padStart(8, '0'));
	}

	//creiamo due variabili dove inserire il balore binario dei cinque byte
	let byte1 = parseInt(data[0], 10).toString(2).padStart(8, '0');
	let byte2 = parseInt(data[1], 10).toString(2).padStart(8, '0');
	let byte3 = parseInt(data[2], 10).toString(2).padStart(8, '0');
	let byte4 = parseInt(data[3], 10).toString(2).padStart(8, '0');
	let byte5 = parseInt(data[4], 10).toString(2).padStart(8, '0');
	let arrayBin =  [byte1, byte2, byte3, byte4, byte5];
	//console.log("arraybin", arrayBin);

	/*for (let i = 0; i < arrayBin.length; i++) {
		if(arrayBin[1] != '00000000') {
			let shifted = arrayBin.shift();
			arrayBin.push(shifted);
		}
	}*/
	//console.log("last arryabin: ", arrayBin);

	console.log('byte1', arrayBin[0]);
	console.log('byte2', arrayBin[1]);
	console.log('byte3', arrayBin[2]);
	console.log('byte4', arrayBin[3]);
	console.log('byte5', arrayBin[4]);
	

	//BYTE 1
	let color = arrayBin[0].substring(1, 3); //posiz 5,6 bit del byte : COLORE
	let byte1_id = arrayBin[0].substring(3, 7);   //posiz. 4, 3, 2, 1 bit del byte : ID Sensore / Attuatore
	let byte1_sa = arrayBin[0].substring(7); //posiz. 0 mi identifica se è un sensore o un attuatore
	let bit_parità = arrayBin[0].substring(0, 1);

	//BYTE 2 
	let byte2_type = arrayBin[1].substring(6, 8);
	let byte2_full = false 

	if (byte2_type === '01' || byte2_type === '10' || byte2_type === '11') {
		byte2_full = true;
		console.log('Il secondo byte è pieno!');
		
	}
	console.log(byte2_type);

	//BYTE 3 & 4
	let clean_value3 = arrayBin[2].substring(1,8);
	let clean_value4 = arrayBin[3].substring(1,8);
	let value = clean_value4.concat(clean_value3);
	var decimal = parseInt(value, 2); //parso il binario in decimale
	console.log(decimal);

	//let json = '{ "traffic":' + decimal+ '}';

	let corrupted = false;
	let date = (new Date(new Date().toString().split('GMT')[0]+' UTC').toISOString().split('.')[0]);
	
	//sensori atmosferici
	let json_climate = {
		"sensor": "climate",
		"id_cross": 1,
		"date": new Date(),
		"time": new Date(),
		"type": "", //temp, humidity, pressure
		"value": decimal,
	};
 
	//JSON UNICO PER DATI TRAFFICO-SEMAFORI-STRADE
	let json_traffic = {
		"description": "Data collection of the traffic from all the roads",
		"sensor": "traffic",
		"id_cross": 1,
		"date": new Date(),
		"time": new Date(),
		"data_carriers": []
	}
	
	let json_carrier = {
		"id_road": 0, //1, 2, 3, 4 
		"id_semaphores": 0, //coppia 0 o 1
		"data_veichles": []
	}
	let json_veichle = {
		"type": "",  //car, heavy, moto
		"value": 0
	}
	let climate_received = false;
	let veichle_received = false;
	if(!corrupted) {
		
		if (byte1_sa === '0') { //se è un sensore
			
			//SE NEL BYTE 1 ARRIVANO ID RIFERITI A SENSORI ATMOSFERICI
			switch (byte1_id) {
				case '0001': {
					console.log('Temperature');
					json_climate.type = 'temperature';
					json_climate.value = decimal;
					climate_received = true;
					break;
				}
				case '0010': {
					console.log('Humidity');
					json_climate.type = 'humidity';
					json_climate.value = decimal;
					climate_received = true;
					break;
				}
				case '0011': {
					console.log('Pressure');
					json_climate.type = 'pressure';
					json_climate.value = decimal;
					climate_received = true;
					break;
				}
			}
		}

		if (byte1_sa === '1') {
			console.log('attuatore');
			switch (byte1_id) {
				case '0001': {
					console.log('Semaforo 1');
					json_carrier.id_road = 1;
					json_carrier.id_semaphores = 1;
					break;
				}
				case '0010': {
					console.log('Semaforo 2');
					json_carrier.id_road = 2;
					json_carrier.id_semaphores = 0;
					break;
				}
				case '0011': {
					console.log('Semaforo 3');
					json_carrier.id_road = 3;
					json_carrier.id_semaphores = 1;
					break;
				}
				case '0100': {
					console.log('Semaforo 4');
					json_carrier.id_road = 4;
					json_carrier.id_semaphores = 0;
					break;
				}
			}

			if (byte2_full) {
				switch(byte2_type) {
					case '01': {
						json_veichle.type = 'motorcycle';
						json_veichle.value = decimal;
						console.log(json_veichle.type);
						veichle_received = true;
						break;
					}
					case '10': {
						json_veichle.type = 'car';
						json_veichle.value = decimal;
						console.log(json_veichle.type);
						veichle_received = true;
						break;
					}
					case '11': {
						json_veichle.type = 'truck';
						json_veichle.value = decimal;
						console.log(json_veichle.type);
						veichle_received = true;
						break;
					}
				}
			}
		}
		
	}
	
	let json_string;
	if(climate_received) {
		json_string = JSON.stringify(json_climate);
		console.log(json_string);
		client.rpush([json_climate.sensor, json_string], function (err, reply) {
			//console.log("Queue Length", reply);
		});
	}


	if(veichle_received) {
		json_carrier.data_veichles.push(json_veichle);
		json_traffic.data_carriers.push(json_carrier);
		console.log(json_traffic);
		json_string = JSON.stringify(json_traffic);
		//json_string = JSON.stringify(json_veichle);
		client.rpush([json_traffic.sensor, json_string], function (err, reply) {
			//console.log("Queue Length", reply);
		});
	}
	fs.writeFileSync("prova.json", json_string);
	console.log('................');
	

	client.lrange(json_climate.sensor, 0, 1, function (err, reply) {
		//console.log("Queue result", reply);
	});
}
