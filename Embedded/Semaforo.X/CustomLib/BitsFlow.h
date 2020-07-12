char bitChange(char dato, char n) //funzione per la negazione di un bit dentro a un byte
{
    if (dato & (1 << (n)))
    {
        return dato |= (1 << (n));
    }
    else
    {
        return dato &= ~(1 << (n));
    }
}

//funzione per il controllo dei dati ricevuti e del eventuale correzione di uno
void bitParita(char *rx)
{
    char error = 0;             //Memorizza se c'Ã¨ un errore
    char sommaRow = 0;          //Tiene la somma dei bit per la riga
    char errorRow = 0;          //Salva la riga con l'errore
    char sommaColumn = 0;       //Tiene la somma dei bit per la colonna
    char errorColumn = 0;       //Salva la colonna con l'errore
    for (int i = 0; i < 5; i++) //Ciclo per controllare tutte le righe dei byte ricevuti
    {
        for (int y = 0; y < 8; y++) //Ciclo per fare la somma di tutti i bit sulla riga
        {
            sommaRow += (rx[i] >> y) & 1; //cicla sulle riga del byte
        }
        if (sommaRow % 2 == 1) //Controlla se la somma Ã¨ pari o dispari
        {
            error = 1;
            errorRow = i;
        }
    }
    for (int i = 0; i < 8; i++) //Ciclo per controllare tutte le colonne dei byte
    {
        for (int y = 0; y < 4; y++)
        // Ciclo per fare la somma di tutti i bit della colonna
        {
            sommaColumn += (rx[y] >> i) & 1; //cicla sulle colonne
        }
        if (sommaColumn % 2 == 1) //Controlla se la somma Ã¨ pari o dispari
        {
            error = 1;
            errorColumn = i;
        }
    }
    if (error != 0) //se Ã¨ stato trovato un errore passerÃ  alla sue correzione
    {
        ////correction = (char)pow(2, errorColumn - 1);
        ////rx[errorRow] = correction;
        rx[errorRow] = bitChange(rx[errorRow], errorColumn);
    }
}