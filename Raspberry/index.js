const SerialPort = require('serialport')
const ByteLength = require('@serialport/parser-byte-length')

const port = new SerialPort('/dev/ttyS0');

//con questo parser lavoriamo con 5 byte alla volta 
const parser = port.pipe(new ByteLength({ length: 5 }))
parser.on('data', parseMsg)

function parseMsg(data) {
        console.log(data);

        //per ogni elemento del buffer ricevuto via seriale, estraiamo il valore in binario
        let msgSize = data.length;
        for (let i = 0; i < msgSize; i++) {
                let valore = parseInt(data[i], 10).toString(2);
                //aggiungiamo gli 0 omessi perchè non significanti
                console.log(valore.padStart(8, '0'));

        }

    //creiamo due variabili dove inserire il balore binario dei cinque byte
	let byte1 = parseInt(data[0], 10).toString(2).padStart(8, '0');
	let byte2 = parseInt(data[1], 10).toString(2).padStart(8, '0');	
	let byte3 = parseInt(data[2], 10).toString(2).padStart(8, '0');
	let byte4 = parseInt(data[3], 10).toString(2).padStart(8, '0');
	let byte5 = parseInt(data[4], 10).toString(2).padStart(8, '0');
	console.log('byte1', byte1);
	console.log('byte2', byte2);
	console.log('byte3', byte3);
	console.log('byte4', byte4);
	console.log('byte5', byte5);
	console.log('------------');

	let color = byte1.substring(1, 3); //posiz 5,6 bit del byte : COLORE
	let id_sa = byte1.substring(3, 7);   //posiz. 4, 3, 2, 1 bit del byte : ID Sensore / Attuatore
	let sa = byte1.substring(7); //posiz. 0 mi identifica se è un sensore o un attuatore
	let bit_parità = byte1.substring(0, 1);
    let corrupted = false;
	if(!corrupted) {
		
		if (sa === "0") { //se è un sensore
			switch (id_sa) {
				case "1010":
						console.log("Temperatura");
						break;
				case "1110":
						console.log("Umidità");
						break;
				case "1100":
						console.log("Traffico");
						break;
				case "1000":
						console.log("Temporizzazione");
						break;
			}
		} 
		
		console.log('................')
	}
}



//qui arrivano i dati dal microcontrollore tramite seriale


//DA BYTE A DATO



//invio alla coda di redis


//riceve dei dati che arrivano dal cloud ??? 



//DA DATO A BYTE 




//invio dei dati al microcontrollore tramite seriale