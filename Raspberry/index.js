//require per bit di parità
const parity = require('./bit_parita');
const fs = require("fs");
const SerialPort = require('serialport');
const ByteLength = require('@serialport/parser-byte-length');
const redis = require('redis');
const port = new SerialPort('/COM7');
const client = redis.createClient(6379, '192.168.0.99');
const _ = require('lodash');

let corrupted = false;
let climate_received = false;
let vehicle_received = false;

let trafficSent = false;
let climateSent = false;

let json_traffic;
let json_climate;

//Connection to Redis
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

//-----------------------------------------------------------------------------------------------
//Richiesta Dati Clima al Pic
const sendDataRequestClimate = function() {
	let cmd_climate = [0x0A, 0x00, 0x00, 0x00, 0x00];
	port.write(cmd_climate);
	console.log('Sent value to Pic:', cmd_climate);

	if(cmd_climate[0] === 10) {
		climateSent = true;
	}
	//reset cmd
	cmd_climate = [0x00, 0x00, 0x00, 0x00, 0x00];
}

//Richiesta Dati Traffico al Pic
const sendDataRequestTraffic = function() {
	let cmd_traffic = [0x08, 0x00, 0x00, 0x00, 0x00];
	port.write(cmd_traffic);
	console.log('Sent value to Pic:', cmd_traffic);

	if (cmd_traffic[0] === 8) {
		trafficSent = true;
	}
	//reset cmd
	cmd_traffic = [0x00, 0x00, 0x00, 0x00, 0x00];
}

//Invia richiesta di Clima ogni 55 secondi x simulazione
setInterval(() => {
	sendDataRequestClimate();
	if (climateSent) {
		climateManagement();
	}

}, 55000);

//Invia richiesta ogni 35 secondi x simulazione
setInterval(() => {
	sendDataRequestTraffic();
	if (trafficSent) {
		trafficManagement();
	}
}, 35000);


function parseBytes() {
	var byteReceive = new Promise(function (resolve, reject) {
		let arrayBinary = [];
		const parser = port.pipe(new ByteLength({ length: 5 }));
		parser.on('data', (data) => {
			let msgSize = data.length;
			for (let i = 0; i < msgSize; i++) {
				let valore = parseInt(data[i], 10).toString(2);
				let binary = valore.padStart(8, '0');
				arrayBinary.push(binary);
			}
			
			let packets = _.chunk(arrayBinary, 5)
			//console.log(packets);
			//console.log('----------------------');

			if (climateSent && packets.length > 2) {
				resolve(packets);
			} else if (trafficSent && packets.length > 11) {
				resolve(packets);
			}
		});
	});
	return byteReceive;
}


function climateManagement() {
	let date = new Date();
	//JSON sensori atmosferici
	json_climate = {
		"sensor": "climate",
		"id_cross": 1,
		"date": date.toISOString().slice(0, 10),
		"time": date.toISOString().slice(11, 19),
		"data_climate": []
	};

    parseBytes().then(function(returnValue) {
        returnValue.forEach(pack => {
			//console.log('pack', pack);
			
			//controllo errori bit di parità
			//let controlled_pack = parity.func1(pack);

			//BYTE 1
			let byte1_id = pack[0].substring(3, 7);   //posiz. 4, 3, 2, 1 bit del byte : ID Sensore / Attuatore
			let byte1_sa = pack[0].substring(7); //posiz. 0 mi identifica se è un sensore o un attuatore
			let bit_parità = pack[0].substring(0, 1);

			//BYTE 3 & 4 ripuliti dai bit di parità intermedi
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
	}, function (err) {
		console.log(err);
	})
}

function pushClimateOnRedis(json_weather) {
	let json_string;
	if (climate_received) {
		//let json_climate = buildJSONClimate();
		json_climate.data_climate.push(json_weather);
		json_string = JSON.stringify(json_climate);
		console.log(json_string);
		if(json_climate.data_climate.length === 3) {
			client.rpush([json_climate.sensor, json_string]);
		}
		
		fs.writeFileSync("climate.json", json_string);
		climateSent = false;
	}
}

function trafficManagement() {

	//JSON unico per traffico proveniente da N strade
	json_traffic = {
		"description": "Data collection of the traffic from all the roads",
		"sensor": "traffic",
		"id_cross": 1,
		"date": new Date().toISOString().slice(0, 10),
		"time": new Date().toISOString().slice(11, 19),
		"data_carriers": []
	}

	parseBytes().then(function(returnValue) {
		let byte2_full = false;
		let byte2_type;
		var decimal;

		returnValue.forEach(pack => {
			//console.log('pack: ', pack);

			//controllo errori bit di parità
			//let controlled_pack = parity.func1(pack);

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
			let clean_value3 = pack[2].substring(1, 8);
			let clean_value4 = pack[3].substring(1, 8);
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
						break;
					}
				}
			} else {
				console.log('ERROR: Invalid data for traffic request, check if the roads are received!')
			}
			let json_vehicles = vehicleDistribution(byte2_full, byte2_type, decimal);
			pushJSONTraffic(json_vehicles, json_carrier);
		});	
		
	})
	.then(res => {
		pushOnRedis(json_traffic);
	})
	.catch((err) => {
		console.log(err);
	});	
}
/**
 * 
 * @param {object} json_vehicles - json vehicles ritornato dalla funzione vehicleDistribution
 * @param {object} json_carrier - json che contiene N strade
 */
function pushJSONTraffic(json_vehicles, json_carrier) {
	
	let isPresent = false;
	json_traffic.data_carriers.forEach(carrier => {
		if (carrier.id_road === json_carrier.id_road) {
			isPresent = true;
			carrier.data_vehicles.push(json_vehicles);
		}
	});

	if(!isPresent) {
		json_carrier.data_vehicles.push(json_vehicles);
		json_traffic.data_carriers.push(json_carrier);
	}
	json_string = JSON.stringify(json_traffic);
	fs.writeFileSync("traffic.json", json_string);
	console.log(json_traffic);
	trafficSent = false;
	return json_traffic;
}

/**
 * This function pushes on Redis once the json traffic is ready
 * @param {object} json_traffic 
 */
function pushOnRedis(json_traffic) {
	json_string = JSON.stringify(json_traffic);
	client.rpush([json_traffic.sensor, json_string]);
}

/**
 * 
 * @param {boolean} byte2_full - Controlla se il byte 2 è pieno e quindi se contiene la tipologia dei veicoli.
 * @param {string} byte2_type - Tipologia di veicoli
 * @param {string} decimal - Valore numerico ritornato dal pic
 * 
 * @return json_vehicle; // che successivamente verrà pushato nel data_vehicles
 */
function vehicleDistribution(byte2_full, byte2_type, decimal) {

	let json_vehicle = {
		"type": "",  //car, heavy, moto
		"value": 0
	}

	if (byte2_full) {
		vehicle_received = true;
		switch (byte2_type) {
			case '01': {
				json_vehicle.type = 'Motociclo';
				json_vehicle.value = decimal;
				console.log(json_vehicle.type);
				break;
			}
			case '10': {
				json_vehicle.type = 'Automobile';
				json_vehicle.value = decimal;
				console.log(json_vehicle.type);
				break;
			}
			case '11': {
				json_vehicle.type = 'Mezzo Pesante';
				json_vehicle.value = decimal;
				console.log(json_vehicle.type);
				break;
			}
		}
		return json_vehicle;
	} else {
		console.log('ERROR: Second byte does not contain vehicles type!');
	}
}
