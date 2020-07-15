//simulazione dati ricevuti==========================
/*
let byte1 = '10101010';
let byte2 = '01101010';
let byte3 = '11101010';
let byte4 = '01101010';
let byte5 = '01100000';

let dati = [byte1, byte2, byte3, byte4, byte5];

let controlled = error_control(dati);
*/
//====================================================


module.exports = {
    func1: function error_control(data) {
        console.log('original data: ', data);

        //conversione dei singoli byte in da stringa a int
        let byte1 = data[0].split("").map(Number);
        let byte2 = data[1].split("").map(Number);
        let byte3 = data[2].split("").map(Number);
        let byte4 = data[3].split("").map(Number);
        let byte5 = data[4].split("").map(Number);

        //array con i byte convertiti
        let data_converted = [byte1, byte2, byte3, byte4, byte5];

        //dichiarazioni variabili
        var err_col;
        var err_riga;
        var somma_riga;

        //controllo righe
        for (let i = 0; i < 5; i++) {

            for (let j = 0; j < 7; j++) {
                somma_riga += data_converted[i][j];
            }

            if (somma_riga % 2 == 1) {
                console.log("Errore riga " + (i + 1));
                err_riga = i;

            }
            somma_riga = 0;
        }


        //controllo colonne
        for (i = 1; i < data_converted.length; i++) {
            if (byte1[i] ^ byte2[i] ^ byte3[i] ^ byte4[i] != byte5[i]) {
                console.log("Errore colonna " + (i + 1));
                err_col = i;
            }
        }

        //controllo presenza errori
        if (err_col == null && err_riga == null) {
            console.log("Nessun errore presente");
        }
        else {
            //coordinate del bit sbagliato
            var wrong_bit = data_converted[err_col][err_riga];
            console.log("Il bit sbagliato conteneva: " + wrong_bit);

            //correzione il bit sbagliato invertendone il valore
            if (wrong_bit == 1) {
                wrong_bit = 0;
            }
            else {
                wrong_bit = 1;
            }
            //sostituzione del bit sbagliato con quello corretto
            data_converted[err_col][err_riga] = wrong_bit;
            console.log("Il bit è stato corretto con: " + wrong_bit);

        }

        //riconversione dei byte da int a stringa
        let byte1r = data_converted[0].map(String).join("");
        let byte2r = data_converted[1].map(String).join("");
        let byte3r = data_converted[2].map(String).join("");
        let byte4r = data_converted[3].map(String).join("");
        let byte5r = data_converted[4].map(String).join("");

        //salvataggio dei byte in un nuovo array di stringhe
        let data_reconverted = [byte1r, byte2r, byte3r, byte4r, byte5r];
        console.log("reconverted data: ", data_reconverted);

        //restituisco l'array corretto
        return data_reconverted;

    }
};

