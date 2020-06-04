const SerialPort = require('serialport');
const ByteLength = require('@serialport/parser-byte-length');
const redis = require('redis');

const port = new SerialPort('/COM6');
const client = redis.createClient();
 
//con questo parser lavoriamo con 5 byte alla volta
const parser = port.pipe(new ByteLength({length: 5}))
parser.on('data', parseMsg) 

function parseMsg(data) {

	//per ogni elemento del buffer ricevuto via seriale, estraiamo il valore in binario
	let msgSize = data.length;
	for (let i = 0; i < msgSize; i++) {
		let valore = parseInt(data[i], 10).toString(2);
		//aggiungiamo gli 0 omessi perchè non significanti
		//console.log(valore.padStart(8, '0'));
	}

	//creiamo due variabili dove inserire il balore binario dei cinque byte
	let byte1 = parseInt(data[0], 10).toString(2).padStart(8, '0');
	let byte2 = parseInt(data[1], 10).toString(2).padStart(8, '0');
	let byte3 = parseInt(data[2], 10).toString(2).padStart(8, '0');
	let byte4 = parseInt(data[3], 10).toString(2).padStart(8, '0');
	let byte5 = parseInt(data[4], 10).toString(2).padStart(8, '0');
	let arrayBin =  [byte1, byte2, byte3, byte4, byte5];
	console.log("arraybin", arrayBin);

	for (let i = 0; i < arrayBin.length; i++) {
		if(arrayBin[1] != "00000000") {
			let shifted = arrayBin.shift();
			arrayBin.push(shifted);
		}
	}
	console.log("last arryabin: ", arrayBin);

	console.log('byte1', arrayBin[0]);
	console.log('byte2', arrayBin[1]);
	console.log('byte3', arrayBin[2]);
	console.log('byte4', arrayBin[3]);
	console.log('byte5', arrayBin[4]);
	console.log('................')

	//BYTE 1
	let color = arrayBin[0].substring(1, 3); //posiz 5,6 bit del byte : COLORE
	let id_sa = arrayBin[0].substring(3, 7);   //posiz. 4, 3, 2, 1 bit del byte : ID Sensore / Attuatore
	let sa = arrayBin[0].substring(7); //posiz. 0 mi identifica se è un sensore o un attuatore
	let bit_parità = arrayBin[0].substring(0, 1);

	//BYTE 3 & 4
	let clean_value3 = arrayBin[2].substring(1,8);
	let clean_value4 = arrayBin[3].substring(1,8);
	console.log("value bin", clean_value4, clean_value3);
	let value = clean_value4.concat(clean_value3);
	var decimal = parseInt(value, 2); //parso il binario in decimale
	console.log(decimal);

	let corrupted = false;
	let key;
	if(!corrupted) {
		
		if (sa === "0") { //se è un sensore
			switch (id_sa) {
				case "1010":
						console.log("Temperature");
						key = "temperature";
						break;
				case "1110":
						console.log("Humidity");
						key = "humidity";
						break;
				case "1100":
						console.log("Traffic");
						key = "traffic";
						break;
				case "1000":
						console.log("Timing");
						key = "timing";
						break;
			}
		}
		
	}
	client.on("error", function(error) {
		console.error(error);
	});
	   
	client.set(key, decimal, redis.print);
	client.get(key, redis.print);

	client.rpush('list-traffic', decimal, function (err, reply) {
		console.log("Queue Length", reply);
	});

	client.lrange('list-traffic', 0, -1, function(err, reply) {
		console.log(reply);
	});
}



		/*if (sa === "1") { //se è un attuatore
			switch (id_sa) {
				case "0001":
						console.log("Semaforo 1");
						let traffic_light_1 = "traffic_light_1";
					break;
				case "0011":
						console.log("Semaforo 2");
						break;
				case "0111":
						console.log("Semaforo 3");
						break;
				case "1111":
						console.log("Semaforo 4");
						break;
			}
			switch (color) {
				case "00":
						console.log("Rosso");
						break;
				case "01":
						console.log("Giallo");
						break;
				case "11":
						console.log("Verde");
						break;
			}
		}
		
		*/