/*
 * File:   main.c
 * Author: Rigna
 *
 * Created on 15 maggio 2020, 16.07
 * Project Work
 TODO: Gestione sensori,controlli (integrare funzioni e migliorire al codice)
 */

#pragma config FOSC = HS  // Oscillator Selection bits (RC oscillator)
#pragma config WDTE = OFF // Watchdog Timer Enable bit (WDT enabled)
#pragma config PWRTE = ON // Power-up Timer Enable bit (PWRT disabled)
#pragma config BOREN = ON // Brown-out Reset Enable bit (BOR enabled)
#pragma config LVP = ON   // Low-Voltage (Single-Supply) In-Circuit Serial Programming Enable bit (RB3/PGM pin has PGM function; low-voltage programming enabled)
#pragma config CPD = OFF  // Data EEPROM Memory Code Protection bit (Data EEPROM code protection off)
#pragma config WRT = OFF  // Flash Program Memory Write Enable bits (Write protection off; all program memory may be written to by EECON control)
#pragma config CP = OFF   // Flash Program Memory Code Protection bit (Code protection off)

#include <xc.h>

#define _XTAL_FREQ 32000000

#define ADON 0
#define CHS0 3
#define ADFM 7

#define Disp1 PORTAbits.RA2 //display1 7 segmenti
#define Disp2 PORTAbits.RA3 //display2 7 segmenti
#define Disp3 PORTAbits.RA4 //display3 7 segmenti
#define Disp4 PORTAbits.RA5 //display4 7 segmenti
//*Inizializzazione delle luci -->
#define Lux_Red PORTBbits.RB5    //luce rossa
#define Lux_Yellow PORTBbits.RB6 //luce gialla
#define Lux_Green PORTBbits.RB7  //luce verde
//* end <--
typedef struct
{
    unsigned int Bit : 1;
} Bit;

struct
{
    unsigned int Bit : 1;
    unsigned int Timeout : 1;
} readGatewayDone;

/*
typedef struct
{
   unsigned int Value:14;
}Time;
*/

Bit readGateway, secondPassed;
char str[4]; //stringa di salvatagio per la conversione da int to string
//Array per la visualizzazione dei numeri sui display
const char display[11] = {0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x07, 0x7F, 0x6F};
char unita, decine, centinaia; //varibile per scomporre il numero per il countdown e stamparlo sui display
unsigned char old_disp, disp;  //varibile per fare lo switch in loop tra i dislpay
unsigned int count = 0;        //variabile per il conteggio del tempo di pressione del tasto
unsigned char count_lux = 0;   //conteggio per il tempo delle luci
char comando = 0;              //Prende il dato dalla seriale
char by1 = 0;                  //Primo byte ricevuto
char by2 = 0;                  //Secondo byte ricevuto
unsigned char Time_Red = 10;   //tempo luce rossa (pre-impostato a 10s)
unsigned char Time_Yellow = 5; //tempo luce gialla (pre-impostato a 10s)
unsigned char Time_Green = 10; //tempo luce verde (pre-impostato a 10s)
unsigned char time = 0;        //variabile per contare i secondi
unsigned char countdown = 0;   //variabile per il conto alla rovescia
unsigned char car = 0;         //variabile per contare le macchine
unsigned char truck = 0;       //variabile per contare i camion
char dataFromGatewayIndex = 0; //indice array dati da seriale
typedef char ProtocolBytes[5]; //array dati da seriale
char dataFromGateway[15];
ProtocolBytes *Bytes[3];
int timerReadFromGateway; //timer per definire se la lettura dati eccede un tempo limite
int colorsTime[3];        //0 � rosso, 1 � verde, 2 � giallo
char colorIndex;

void init_ADC();                                                  //Inizializza l'adc
int ADC_Read(char canale);                                        //Lettura da un ingresso analogico
void UART_Init(int baudrate);                                     //Inizializzazione della seriale con uno specifico baudrate
void UART_TxChar(char ch);                                        //Scrittura di un carattere sulla seriale
void UART_Write_Text(char *text);                                 //Scrittura di una stringa sulla seriale
char UART_Read();                                                 //Lettura dalla seriale
void intToString(int valore);                                     //Funzione di conversione da intero a una stringa
double pow(double b, double e);                                   //Funzione per fare la potenza
int map(int x, int in_min, int in_max, int out_min, int out_max); //Funzione per mappare dei valori
char bitChage(char dato, char n);
void bitParita(char *rx);
int GetTime(int index);
void GetDigits(int Time);

void main(void)
{
    TRISB = 0x1F; //gli utlimi tre bit per le luci, gli altri come ingresso
    TRISC = 0x80;
    TRISD = 0x00; //Porta per i 7 segmenti (Output)
    TRISE = 0x00;
    INTCON = 0xE0;     //abilito le varie variabili per chiamare gli interrupt
    OPTION_REG = 0x04; //imposto il prescaler a 1:32 del timer0
    TMR0 = 6;          //imposto il tempo iniziale a 6 per farlo attivare ogni 0,001 secondi
    T1CON = 0x31;      //Imposto il prescaler a 1:8 e attivo il timer1
    //TMR1 = 0x00;
    //?PIE1 = 0x01;
    //imposto il tempo iniziale a 15536 di timer1 per farlo attivare ogni 0, 050 secondi
    TMR1H = 60;      // preset for timer1 MSB register
    TMR1L = 176;     // preset for timer1 LSB register
    init_ADC();      //Inizializzazione adc
    UART_Init(9600); //Inizializzazione seriale a 9600 b
    /* 
    ?richiesta dati al raspberry 
    ?atendi un tempo oltre ciò se non ha ricevuto niente mette dei dati standard 
    */
    int colorsTime[3], time;    //0 � rosso, 1 � verde, 2 � giallo
    char lux_select = 0;        //selezione luce per il semaforo
    char old_lux_select = 9;    //salva il vecchio stato
    disp = 0;                   //inizializzo a 0
    char old_disp = 9;          //salva il vecchio stato
    char temp = 0;              //Variabile per salvare la temperatura sul pin RA0
    char umidita = 0;           //Variabile per salvare l'umidita sul pin RA1
    unsigned char old_time = 1; //serve per far leggere i valori dei sensori ogni secondo
    colorsTime[0] = 5;
    colorsTime[1] = 2;
    colorsTime[2] = 11;

    while (1)
    {
        //se si stanno ricevendo dati dalla seriale
        if (readGateway.Bit)
        {
            if (timerReadFromGateway >= 4) //if scatta dopo un timer di 4s
            {
                readGatewayDone.Bit = 1;
                readGatewayDone.Timeout = 1;
                readGateway.Bit = 0;
            }

            if (dataFromGatewayIndex >= 14)
            {
                readGatewayDone.Bit = 1;
                readGatewayDone.Timeout = 0;
                readGateway.Bit = 0;
            }
        }

        //cose da fare terminata la lettura dalla seriale
        if (readGatewayDone.Bit)
        {
            //resetta le variabili per la lettura
            readGateway.Bit = 0;
            dataFromGatewayIndex = 0;
            readGatewayDone.Bit = 0;
            timerReadFromGateway = 0;

            //se c'� stato un timeout resetta la lettura del timeout
            if (readGatewayDone.Timeout)
            {
                readGatewayDone.Timeout = 0;
            }
            //se il readgatewaydonenon � stato richiamato dal timeout inizia la modifica dei dati
            else
            {
                //bitParita(dataFromGateway); //controllo correttezza dati
                for (int i = 0; i < 4; i++)
                {
                    //colorIndex=((*Bytes[i])[0]>>5)&0x03;
                    //colorsTime[colorIndex]=GetTime(*Bytes[i]);
                    int index = i * 5;
                    colorIndex = (dataFromGateway[index] >> 5) & 0x03;
                    colorsTime[colorIndex] = GetTime(index);
                }
            }
        }

        //se avviene qualche cambiamento allora aggornero i tempi
        if ((Time_Red != colorsTime[0]) || (Time_Green != colorsTime[1]) || (Time_Yellow != colorsTime[2]))
        {
            Time_Red = colorsTime[0];
            Time_Green = colorsTime[1];
            Time_Yellow = colorsTime[2];
        }

        //ACCENSIONE LED IN BASE AL TEMPO
        //Cambiamento del timer ed eventuale cambio luci ogni secondo
        if (secondPassed.Bit)
        {
            secondPassed.Bit = 0;
            time++;

            if (colorsTime[lux_select] - time < 0)
            {
                lux_select = (lux_select + 1) % 3;
                time = 0;
            }

            GetDigits(colorsTime[lux_select] - time);
        }

        //Mostra il timer sul display
        if (disp != old_disp) //Lo esegue solo quando "disp" cambia (cio� ad ogni ciclo while))
        {
            old_disp = disp;
            switch (disp) //fa lo scambio tra i display partendo dalle unita per arrivare alle centinaia per poi ricominciare
            {
            case 0:                //==> desplay delle centinaia, porta RA2
                if (centinaia > 0) //mostra la cifra delle centinaia solo se � consistente (maggiore di 0)
                {
                    Disp2 = 0;
                    Disp3 = 0;
                    Disp4 = 0;
                    Disp1 = 1;
                    PORTD = display[centinaia]; //Scrive su "PORTD" i pin che andranno a 1 per far vedere il numero che è presente nel array "display[*n]"
                }
                break;
            case 1:                              //==> desplay delle dedcine, porta RA3
                if (decine > 0 && centinaia > 0) //mostra la cifra delle decine e delle centinaia solo se sono consistenti (maggiore di 0), si considerano anche le centinaia per numeri come 102, in cui le decine non sono consistenti ma le centinaia si
                {
                    Disp1 = 0;
                    Disp3 = 0;
                    Disp4 = 0;
                    Disp2 = 1;
                    PORTD = display[decine]; //Scrive su "PORTD" i pin che andranno a 1 per far vedere il numero che è presente nel array "display[*n]"
                }
                break;
            case 2: //==> desplay delle unit�, porta RA4
                Disp1 = 0;
                Disp2 = 0;
                Disp4 = 0;
                Disp3 = 1;
                PORTD = display[unita]; //Scrive su "PORTD" i pin che andranno a 1 per far vedere il numero che è presente nel array "display[*n]"
                break;
            case 3: //==> desplay opzionale per mostrare il colore del semaforo, porta RA5 (attualmente spento)
                Disp1 = 0;
                Disp2 = 0;
                Disp3 = 0;
                Disp4 = 1;
                PORTD = display[lux_select]; //Scrive su "PORTD" i pin che andranno a 1 per far vedere il numero che è presente nel array "display[*n]"
                break;
            }
        }
        disp = (disp + 1) % 4; //disp viene incrementato e ha valori tra 0 e 3

        //*Gestione sensori -->
        if (time != old_time) //legge i sensori ogni secondo
        {
            old_time = time;
            temp = (char)map((ADC_Read(0) >> 2), 0, 255, -20, 60);   //legge la temperatura e la mappa su quei valori
            umidita = (char)map((ADC_Read(1) >> 2), 0, 255, 0, 100); //legge l'umidità e la mappa su quei valori
        }
        //*end <--
    }

    return;
}
//inizializzo ADC (potenziometro)
void init_ADC()
{
    TRISA = 0xC3;   //imposto i pin come ingressi trane RA2 RA3 RA4
    ADCON0 = 0x00;  // setto ADCON0 00000000
    ADCON1 = 0x80;  // SETTO ADCON1 (ADFM) a 1 --> risultato giustificato verso dx 10000000
    __delay_us(10); //delay condensatore 10us
}

//leggo il valore del potenziometro
int ADC_Read(char canale)
{
    ADCON0 = (1 << ADON) | (canale << CHS0);
    //ADCON0bits.ADON = 1;   //accendo il convertitore (ADCON0)
    //ADCON0 |= canale << 3; //e setto il canale da convertire (ADCON0)
    __delay_us(2); //attendo 1.6 uS
    GO_nDONE = 1;  // avvio la conversione ADGO GO
    while (GO_nDONE)
        ;                          //attendo la fine della conversione
    return ADRESL + (ADRESH << 8); // preparo il dato (valore = ADRESL + (ADREAH << 8)
}

void UART_Init(int baudrate)
{
    TRISCbits.TRISC6 = 0; //TRISC= 0x80;   //10000000

    TXSTAbits.TXEN = 1; //TXSTA= 0x20;   //00100000

    RCSTAbits.SPEN = 1; //RCSTA= 0x90;   //10010000
    RCSTAbits.CREN = 1; //RCSTA= 0x90;   //10010000

    SPBRG = (_XTAL_FREQ / (long)(64UL * baudrate)) - 1;
    INTCONbits.GIE = 1;  //abilito global interrupt
    INTCONbits.PEIE = 1; //peripherial interrupt
    PIE1bits.RCIE = 1;   //uart rx interrupt
}

void UART_TxChar(char ch)
{
    while (!TXIF)
        ;     //se TXIF ? a 0 la trasmissione ? ancora in corso
    TXIF = 0; //lo resetto
    TXREG = ch;
}

void UART_Write_Text(char *text)
{
    int i;
    for (i = 0; text[i] != '\0'; i++)
    {
        UART_TxChar(text[i]);
    }
}

char UART_Read()
{
    while (!RCIF)
        ;
    RCIF = 0;
    return RCREG;
}

void intToString(int valore) //funzione per convertire un intero in una stringa
{
    int i;
    int num = 0;
    for (i = 0; i < 3; i++)
    {
        str[2 - i] = '0' + ((valore % (char)pow(10, 1 + i)) / (char)pow(10, i));
    }
    str[3] = '\0';
}

double pow(double b, double e) //Funzioneper fare la potenza
{
    int i;
    double number = 1;
    for (i = 0; i < e; i++)
    {
        number *= b;
    }
    return (number);
}

int map(int x, int in_min, int in_max, int out_min, int out_max) //Mappare nuovamente un numero da un intervallo a un altro
{
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

char bitChage(char dato, char n) //funzione per la negazione di un bit dentro a un byte
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
    char error = 0;             //Memorizza se c'è un errore
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
        if (sommaRow % 2 == 1) //Controlla se la somma è pari o dispari
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
        if (sommaColumn % 2 == 1) //Controlla se la somma è pari o dispari
        {
            error = 1;
            errorColumn = i;
        }
    }
    if (error != 0) //se è stato trovato un errore passerà alla sue correzione
    {
        ////correction = (char)pow(2, errorColumn - 1);
        ////rx[errorRow] = correction;
        rx[errorRow] = bitChage(rx[errorRow], errorColumn);
    }
}

int GetTime(int index)
{
    int time;
    struct
    {
        unsigned int Val : 7;
    } shortInt;

    shortInt.Val = dataFromGateway[index + 3] & 0x7F;
    time = shortInt.Val;
    time = (time << 7) & 0x7F;

    shortInt.Val = dataFromGateway[index + 2] & 0x7F;
    time = time | shortInt.Val;

    return time;
}

void GetDigits(int Time)
{
    countdown = Time;
    centinaia = countdown / 100;     //Il tempo totale vine scomposto nelle varie parti per essere poi riportato nei display 7 segmenti (le centinaia)
    decine = (countdown % 100) / 10; //Il tempo totale vine scomposto nelle varie parti per essere poi riportato nei display 7 segmenti (le decine)
    unita = (countdown % 100) % 10;  //Il tempo totale vine scomposto nelle varie parti per essere poi riportato nei display 7 segmenti (le unita)
}

void __interrupt() ISR()
{
    //RICEVE DATI DA SERIALE
    if (RCIF && readGateway.Bit == 0)
    {
        readGateway.Bit = 1;
        readGatewayDone.Bit = 0;
        readGatewayDone.Timeout = 0;
        dataFromGatewayIndex = 0;
        timerReadFromGateway = 0;
    }
    if (RCIF && readGateway.Bit == 1)
    {
        dataFromGateway[dataFromGatewayIndex] = UART_Read();
        dataFromGatewayIndex++;
        timerReadFromGateway = 0;
    }

    //se timer0 finisce di contare attiva l'interrupt ed esegue questo codice
    if (TMR0IF) //timer0 "TMR0IF"
    {
        TMR0IF = 0;         //resetto timer0
        if (!PORTBbits.RB3) //controllo pressione del tasto per il verificare se sia una macchina o un camion
        {
            count++;
        }
        if (PORTBbits.RB3) //al rilascio del tasto eseguo il conteggio in base alla pressione
        {
            if (count >= 500)
            {
                car++;
            }
            if (count >= 3000)
            {
                truck++;
            }
            count = 0;
        }
        TMR0 = 6;
    }
    //se timer1 finisce di contare attiva l'interrupt ed esegue questo codice
    if (TMR1IF) //timer1 "TMR1IF", CHIAMATO OGNI: 50ms
    {
        TMR1IF = 0; //resetto timer1
        count_lux++;

        if (count_lux >= 20) //conteggio per arrivare ad un secondo
        {
            secondPassed.Bit = 1;
            count_lux = 0;
            timerReadFromGateway++;
        }

        TMR1H = 60;  // preset for timer1 MSB register
        TMR1L = 176; // preset for timer1 LSB register
    }
}
