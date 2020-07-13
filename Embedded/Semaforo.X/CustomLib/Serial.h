//funzione per inviare dati al raspberry in cui aggiunge gli eventuali bit di paritÃ 
char *BuildByte(char byte0, char byte1, char valore)
{
    char txByte[5];

    txByte[0] = byte0 & 0x7F;         //primo byte di comando
    txByte[1] = byte1 & 0x7F;         //secondo byte di comando
    txByte[2] = valore & 0x7F;        //valore da mandare
    txByte[3] = (valore >> 7) & 0x7F; //valore da mandare sul secondo byte in caso fosse piÃ¹ grande
    char sommaRow = 0;                //Tiene la somma dei bit per la riga
    char sommaColumn = 0;             //Tiene la somma dei bit per la colonna
    
    for (int i = 0; i < 4; i++) //controlla i byte e gli aggiunge il bit di paritÃ  sul ottavo bit se necesario
    {
        for (int y = 0; y < 8; y++) //Ciclo per fare la somma di tutti i bit sulla riga
        {
            sommaRow += (txByte[i] >> y) & 1; //cicla sulle riga del byte
        }
        if (sommaRow % 2 == 1) //Controlla se la somma Ã¨ pari o dispari
        {
            txByte[i] += 0x01 << 7; //aggiunge il bit alla fine
            sommaRow = 0;
        }
        else
        {
            sommaRow = 0;
        }
    }

    //costruisce la matrice di paritÃ 
    for (int i = 0; i < 8; i++) //Ciclo per controllare tutte le colonne dei byte
    {
        for (int y = 0; y < 4; y++)
        // Ciclo per fare la somma di tutti i bit della colonna
        {
            sommaColumn += (txByte[y] >> i) & 1; //cicla sulle colonne
        }
        if (sommaColumn % 2 == 1) //Controlla se la somma Ã¨ pari o dispari
        {
            txByte[4] += 0x01 << i; //aggiunge il bit solo nel posto specifico
            sommaColumn = 0;
        }
        else
        {
            sommaColumn = 0;
        }
    }
    
    return txByte;
}