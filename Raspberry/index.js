const fs = require("fs");
const SerialPort = require('serialport');
const ByteLength = require('@serialport/parser-byte-length');
const redis = require('redis');
const port = new SerialPort('/COM7');
const client = redis.createClient(6379, '192.168.0.99');
const _ = require('lodash');
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

//..............JSON DATA................

//sensori atmosferici
let json_climate = {
	"description": "Data collection of the atmospheric sensors of the cross",
	"sensor": "climate",
	"id_cross": 1,
	"date": new Date().toISOString().slice(0,10),
	"time": new Date().toISOString().slice(11,19),
	"data_climate": []
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


let hasbeenSent = false;
let trafficSent = false;
let climateSent = false;
let cmd;
//-----------------------------------------------------------------------------------------------
//RICHIESTA DATI TRAFFICO
var sendToPic = async function() {
 
	if (!hasbeenSent) {
		cmd = [0x08, 0x00, 0x00, 0x00, 0x00];
		port.write(cmd);
		console.log('Sent value to Pic:', cmd);
		hasbeenSent = true;
		if(cmd[0] === 10) {
			climateSent = true;
		} else if (cmd[0] === 8) {
			trafficSent = true;
		}
	}
	return hasbeenSent;
}


if(!hasbeenSent) {
	sendToPic();
	console.log('message written');
}

//RICEVO I DATI E LI PACCHETTIZZO
var byteReceive = new Promise(function(resolve, reject) { 
	let arrayBinary = [];
	const parser = port.pipe(new ByteLength({length: 5}));
	parser.on('data', (data) => { 
		let msgSize = data.length;
		for (let i = 0; i < msgSize; i++) {
			let valore = parseInt(data[i], 10).toString(2);
			//aggiungiamo gli 0 omessi perchè non significanti
			let binary = valore.padStart(8, '0');
			arrayBinary.push(binary);
			//console.log(arrayBinary);
		}

		let packets = _.chunk(arrayBinary, 5)
		//console.log(packets);
		//console.log('----------------------');

		if(climateSent && packets.length > 2) {
			resolve(packets);
		} else if (trafficSent && packets.length > 11) {
			resolve(packets);
		}
		
	});
});

let corrupted = false;
let date = (new Date(new Date().toString().split('GMT')[0]+' UTC').toISOString().split('.')[0]);
let climate_received = false;
let vehicle_received = false;


function climateManagement() {
    
    byteReceive.then(function(returnValue) {
        returnValue.forEach(pack => {
			console.log('pack', pack);

			//BYTE 1
			let byte1_id = pack[0].substring(3, 7);   //posiz. 4, 3, 2, 1 bit del byte : ID Sensore / Attuatore
			let byte1_sa = pack[0].substring(7); //posiz. 0 mi identifica se è un sensore o un attuatore
			let bit_parità = pack[0].substring(0, 1);

			//BYTE 3 & 4
			let clean_value3 = pack[2].substring(1,8);
			let clean_value4 = pack[3].substring(1,8);
			let value = clean_value4.concat(clean_value3);
			var decimal = parseInt(value, 2); //parso il binario in decimale
			console.log(decimal);

			let json_weather = {
				"type": "",  //temp, humidity, pressure
				"value": 0
			}

			if (byte1_sa === '0' && !corrupted) { //se è un sensore
		
				//SE NEL BYTE 1 ARRIVANO ID RIFERITI A SENSORI ATMOSFERICI
				switch (byte1_id) {
					case '0001': {
						console.log('Temperature');
						json_weather.type = 'temperature';
						json_weather.value = decimal;
						climate_received = true;
						break;
					}
					case '0010': {
						console.log('Humidity');
						json_weather.type = 'humidity';
						json_weather.value = decimal;
						climate_received = true;
						break;
					}
					case '0011': {
						console.log('Pressure');
						json_weather.type = 'pressure';
						json_weather.value = decimal;
						climate_received = true;
						break;
					}	
				}
				
			}
			pushClimateOnRedis(json_weather);
		});
		console.log('................');
    }, function(err) {
        console.log(err);
    });
}



function pushClimateOnRedis(json_weather) {
	let json_string;
	if (climate_received) {
		json_climate.data_climate.push(json_weather);
		json_string = JSON.stringify(json_climate);
		console.log(json_string);
		client.rpush([json_climate.sensor, json_string], function (err, reply) {
			//console.log("Queue Length", reply);
		});
		fs.writeFileSync("climate.json", json_string);
	}
	
}

function trafficManagement() {
	byteReceive.then(function(returnValue) {
		let byte2_full = false;
		let byte2_type;
		var decimal;
		
        returnValue.forEach(pack => {
			console.log('pack', pack);

			//BYTE 1
			let byte1_id = pack[0].substring(3, 7);   //posiz. 4, 3, 2, 1 bit del byte : ID Sensore / Attuatore
			let byte1_sa = pack[0].substring(7); //posiz. 0 mi identifica se è un sensore o un attuatore
			let bit_parità = pack[0].substring(0, 1);

			//BYTE 2 
			byte2_type = pack[1].substring(6, 8);
			
			if (byte2_type === '01' || byte2_type === '10' || byte2_type === '11') {
				byte2_full = true;
			}

			//BYTE 3 & 4
			let clean_value3 = pack[2].substring(1,8);
			let clean_value4 = pack[3].substring(1,8);
			let value = clean_value4.concat(clean_value3);
			decimal = parseInt(value, 2); //parso il binario in decimale
			console.log(decimal);

			let json_carrier = {
				"id_road": 0, //1, 2, 3, 4 
				"id_semaphores": 0, //coppia 0 o 1
				"data_vehicles": []
			}

			if (byte1_sa === '1') {
				
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
							isFirstTime = false;
						
						break;
					}
				}
				
				
			}
				let prova = vehicleDistribution(byte2_full, byte2_type, decimal);
				let isPresent = false;
				json_traffic.data_carriers.forEach(carrier => {
					if(carrier.id_road === json_carrier.id_road) {
						isPresent = true;
						carrier.data_vehicles.push(prova);
					}
				});

				if(!isPresent) {
					json_carrier.data_vehicles.push(prova);
					json_traffic.data_carriers.push(json_carrier);
				}
				
				json_string = JSON.stringify(json_traffic);
				console.log(json_traffic);
				fs.writeFileSync("traffic.json", json_string);
		});	
			
	});
}


function vehicleDistribution(byte2_full, byte2_type, decimal) {

	let json_vehicle = {
		"type": "",  //car, heavy, moto
		"value": 0
	}
	
	if (byte2_full) {
		vehicle_received = true;
		switch(byte2_type) {
			case '01': {
				json_vehicle.type = 'motorcycle';
				json_vehicle.value = decimal;
				console.log(json_vehicle.type);
				
				break;
			}
			case '10': {
				json_vehicle.type = 'car';
				json_vehicle.value = decimal;
				console.log(json_vehicle.type);
				break;
			}
			case '11': {
				json_vehicle.type = 'truck';
				json_vehicle.value = decimal;
				console.log(json_vehicle.type);
				break;
			}
		}
		return json_vehicle;
	}
}

if(climateSent) {
	climateManagement();
}

if(trafficSent) {
	trafficManagement();
}