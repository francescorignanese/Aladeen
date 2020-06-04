//simulazione dati ricevuti==========================
let byte1 = [1, 0, 1, 0, 1, 0, 1, 0];
let byte2 = [0, 1, 1, 0, 1, 0, 1, 0];
let byte3 = [1, 1, 0, 0, 1, 0, 1, 0];
let byte4 = [0, 1, 1, 0, 1, 0, 1, 0];
let byte5 = [0, 1, 1, 0, 0, 0, 0, 0];

let dati = [byte1, byte2, byte3, byte4, byte5];
//====================================================


//prova della funzione
error_control(dati);

//funzione per il controllo degli errori all'interno dei 5 bytes
function error_control(data) {

    //dichiarazioni variabili
    var err_col;
    var err_riga;
    var somma_riga;
    var corrupted=0;

    //controllo righe
    for (let i = 0; i < 5; i++) {

        for (let j = 0; j < 7; j++) {
            somma_riga += data[i][j];
        }

        if (somma_riga % 2 == 1) {
            console.log("Errore riga " + (i + 1));
            err_riga = i;

        }

        somma_riga = 0;

    }

    //controllo colonne
    for (i = 1; i < data.length; i++) {
        if (byte1[i] ^ byte2[i] ^ byte3[i] ^ byte4[i] != byte5[i]) {
            console.log("Errore colonna " + (i + 1));
            err_col = i;
        }

    }


    //controllo presenza errori
    if (err_col == null && err_riga==null) {
        console.log("Nessun errore presente");
        console.log(Boolean(corrupted));
    }
    else {
        //coordinate del bit sbagliato
        var wrong_bit = data[err_col][err_riga];
        console.log("Il bit sbagliato conteneva: " + wrong_bit);

        //correzione il bit sbagliato invertendone il valore
        if (wrong_bit == 1) {
            wrong_bit = 0;
        }
        else {
            wrong_bit = 1;
        }
        //sostituzione del bit sbagliato con quello corretto
        data[err_col][err_riga]=wrong_bit;
        console.log("Il bit è stato corretto con: " + wrong_bit);
        console.log(data);

        //feedback che notifica tramite boleana che il byte è corrotto
        corrupted=1;
        console.log(Boolean(corrupted));
    }

}
