/*
 * File:   main.c
 * Author: Rigna
 *
 * Created on 15 maggio 2020, 16.07
 * Project Work
 TODO: Gestione sensori,controlli (integrare funzioni e migliorare il codice)
 */

/*
*PRIMO BYTE 1:
SENSORE BYTE1.bit0=0;
ATTUATORE BYTE1.bit0=1;

*ID SENSORI:
Temperatura: 0x02
Umidità: 0x04
Pressione: 0x06
Traffico (INVIO): 0x08
Sensori (INVIO): 0x0A


*ID SEMAFORI:
Semaforo 1: 00011
Semaforo 2: 00101
Semaforo 3: 00111
Semaforo 4: 01001

*CODICE COLORI:
Rosso: 01 
Giallo: 10
Verde: 11

*SECONDO BYTE 2:
Tipologia veicoli
Motocicli: 0x01
Macchine: 0x02
Camion: 0x03

*TERZO E QUARTO BYTE 3-4:
Invio valori il massimo valore che si può inviare è 2^14

*QUINTO BYTE 5:
Byte di parità per trovare l'errore
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
#include "CustomLib/BitsFlow.h"
#include "CustomLib/Serial.h"
#include "CustomLib/TrafficLight.h"

#define _XTAL_FREQ 32000000

#define ADON 0
#define CHS0 3
#define ADFM 7

#define Disp1s0 PORTAbits.RA2 //display1 7 segmenti
#define Disp2s0 PORTAbits.RA3 //display2 7 segmenti
#define Disp3s0 PORTAbits.RA4 //display3 7 segmenti
#define Disp1s1 PORTAbits.RA5 //display1 7 segmenti
#define Disp2s1 PORTBbits.RB0 //display2 7 segmenti
#define Disp3s1 PORTBbits.RB0 //display3 7 segmenti
#define Disp1s2 PORTBbits.RB0 //display1 7 segmenti
#define Disp2s2 PORTBbits.RB0 //display2 7 segmenti
#define Disp3s2 PORTBbits.RB0 //display3 7 segmenti
#define Disp1s3 PORTBbits.RB0 //display1 7 segmenti
#define Disp2s3 PORTBbits.RB0 //display2 7 segmenti
#define Disp3s3 PORTBbits.RB0 //display3 7 segmenti
#define DISP0 PORTD           //registro display semaforo 0
//#define DISP1 PORTF         //registro display semaforo 1

//*Conteggio veicoli -->
#define Road1 PORTBbits.RB3 //Sensore per il rivelamento dei mezzi su strada 1
#define Road2 PORTBbits.RB4 //Sensore per il rivelamento dei mezzi su strada 2
#define Road3 PORTBbits.RB5 //Sensore per il rivelamento dei mezzi su strada 3
#define Road4 PORTAbits.RA5 //Sensore per il rivelamento dei mezzi su strada 4
//* end <--

//*Inizializzazione delle luci -->
#define Red1 PORTCbits.RC0    //Rosso primo semaforo
#define Green1 PORTCbits.RC1  //verde primo semaforo
#define Yellow1 PORTCbits.RC2 //giallo primo semaforo (da mettere sia su pin1 che pin2)
#define Red2 PORTBbits.RB7    //Rosso secondo semaforo
#define Green2 PORTBbits.RB1  //verde secondo semaforo
#define Yellow2 PORTBbits.RB6 //giallo primo semaforo (da mettere sia su pin1 che pin2)
//* end <--

struct
{
    unsigned int Bit : 1;
    unsigned int Timeout : 1;
} readGatewayDone;

Bit readGateway, secondPassed, cycled;
unsigned char disp = 0;      //varibile per fare lo switch in loop tra i dislpay
unsigned int count = 0;      //variabile per il conteggio del tempo di pressione del tasto
unsigned char count_lux = 0; //conteggio per il tempo delle luci
int time[2] = {0, 0};        //variabile per contare i secondi
unsigned char motorcycle[4]; //variabile per contare i motocicli
unsigned char car[4];        //variabile per contare le macchine
unsigned char truck[4];      //variabile per contare i camion
char RoadsSensors[4];
unsigned char dataFromGatewayIndex = 0; //indice array dati da seriale
ProtocolBytes dataFromGateway;          //array dati da seriale
Semaforo s0, s1;                        //definisco i vari semafori
Semaforo *Semafori[2] = {&s0, &s1};
Digits digits0, digits1;
Digits *DigitsArr[2] = {&digits0, &digits1};
unsigned char timerReadFromGateway; //timer per definire se la lettura dati eccede un tempo limite
char temp = 0;                      //Variabile per salvare la temperatura sul pin RA0
unsigned char umidita = 0;          //Variabile per salvare l'umidita sul pin RA1
unsigned char pressione = 0;        //Variabile per salvare la pressione sul pin RE0

void init_ADC();                                    //Inizializza l'adc
int ADC_Read(char canale);                          //Lettura da un ingresso analogico
void UART_Init(int baudrate);                       //Inizializzazione della seriale con uno specifico baudrate
void UART_TxChar(char ch);                          //Scrittura di un carattere sulla seriale
char UART_Read();                                   //Lettura dalla seriale
void sendByte(char byte0, char byte1, char valore); //Funzione per inviare dati in cui vengono aggiunti i bit di parità
void conteggioVeicoli();                            //Conteggio mezzi
void sendByte(char byte0, char byte1, char valore);
void SetDisplay(unsigned char display_index, char d1, char d2, char d3, char value);
void luciSemaforo(unsigned char index, unsigned char lux);
void ShowDigitsOnDisplay();

void main(void)
{
    TRISB = 0x38; //gli utlimi tre bit per le luci, gli altri come ingresso
    TRISC = 0x80;
    TRISD = 0x00;      //Porta per i 7 segmenti (Output)
    TRISE = 0x01;      //Utilizzo l'ingresso RE0 per misurare la pressione
    INTCON = 0xE0;     //abilito le varie variabili per chiamare gli interrupt
    OPTION_REG = 0x04; //imposto il prescaler a 1:32 del timer0
    TMR0 = 6;          //imposto il tempo iniziale a 6 per farlo attivare ogni 0,001 secondi
    T1CON = 0x31;      //Imposto il prescaler a 1:8 e attivo il timer1
    //imposto il tempo iniziale a 15536 di timer1 per farlo attivare ogni 0, 050 secondi
    TMR1H = 60;  // preset for timer1 MSB register
    TMR1L = 176; // preset for timer1 LSB register
    //?PIE1 = 0x01;

    init_ADC();                          //Inizializzazione adc
    UART_Init(9600);                     //Inizializzazione seriale a 9600 b
    SetDefaultTimers(6, 4, 2, Semafori); //Inizializzazione tempi luci semaforo

    Red1 = 0;    //azzero le luci
    Red2 = 0;    //azzero le luci
    Yellow1 = 0; //azzero le luci
    Yellow2 = 0; //azzero le luci
    Green1 = 0;  //azzero le luci
    Green2 = 0;  //azzero le luci

    while (1)
    {
        //se si stanno ricevendo dati dalla seriale
        if (readGateway.Bit)
        {
            switch ((dataFromGateway[0] & 0x7F)) //Controllo se devo ricevere o inviare in base alla ricezione e i comandi
            {
            case 0x08: //Se vi è il comando 0x08 nel primo byte allora prendo solo un pachetto di dati
                readGatewayDone.Bit = 1;
                readGateway.Bit = 0;

                for (unsigned char i = 0; i < 4; i++) //Invio tutti i valori
                {
                    //*Parte di debug mezzi con funzione pseudo casuale
                    unsigned char randomMoto = (char)rand();  //Aggiunta funzione random per mandadare dei valori di veicoli pseudo casuali
                    unsigned char randomCar = (char)rand();   //Aggiunta funzione random per mandadare dei valori di veicoli pseudo casuali
                    unsigned char randomTruck = (char)rand(); //Aggiunta funzione random per mandadare dei valori di veicoli pseudo casuali
                    if (randomMoto < 255)                     //Controlla che il numero sia più piccolo del massimo che si può inserire
                    {
                        motorcycle[i] = randomMoto; //Assegna il valore generato
                    }
                    if (randomCar < 255) //Controlla che il numero sia più piccolo del massimo che si può inserire
                    {
                        car[i] = randomCar; //Assegna il valore generato
                    }
                    if (randomTruck < 255) //Controlla che il numero sia più piccolo del massimo che si può inserire
                    {
                        truck[i] = randomTruck; //Assegna il valore generato
                    }
                    //* end <--

                    switch (i)
                    {
                    case 0:
                        sendByte(0x03, 0x01, motorcycle[i]);
                        sendByte(0x03, 0x02, car[i]);
                        sendByte(0x03, 0x03, truck[i]);
                        break;
                    case 1:
                        sendByte(0x05, 0x01, motorcycle[i]);
                        sendByte(0x05, 0x02, car[i]);
                        sendByte(0x05, 0x03, truck[i]);
                        break;
                    case 2:
                        sendByte(0x07, 0x01, motorcycle[i]);
                        sendByte(0x07, 0x02, car[i]);
                        sendByte(0x07, 0x03, truck[i]);
                        break;
                    case 3:
                        sendByte(0x09, 0x01, motorcycle[i]);
                        sendByte(0x09, 0x02, car[i]);
                        sendByte(0x09, 0x03, truck[i]);
                        break;
                    }
                }

                for (int i = 0; i < 4; i++) //Resetto le variabili
                {
                    motorcycle[i] = 0;
                    car[i] = 0;
                    truck[i] = 0;
                }

                for (unsigned char i = 0; i < 5; i++) //Resetto i byte che ho ricevuto così da non continuare ad inviare
                {
                    dataFromGateway[i] = 0;
                }
                break;
            case 0x0A: //Se vi è il comando 0x0A nel primo byte allora prendo solo un pachetto di dati
                readGatewayDone.Bit = 1;
                readGateway.Bit = 0;

                temp = (char)map((ADC_Read(0) >> 2), 0, 255, -20, 60);     //legge la temperatura e la mappa su quei valori
                umidita = (char)map((ADC_Read(1) >> 2), 0, 255, 0, 100);   //legge l'umidità e la mappa su quei valori
                pressione = (char)map((ADC_Read(5) >> 2), 0, 255, 0, 100); //legge la pressione e la mappa su quei valori
                sendByte(0x02, 0x00, temp);                                //Invio dati di temperatura
                sendByte(0x04, 0x00, umidita);                             //Invio dati di umidita
                sendByte(0x06, 0x00, pressione);                           //Invio dati di pressione

                for (unsigned char i = 0; i < 5; i++) //Resetto i byte che ho ricevuto così da non continuare ad inviare
                {
                    dataFromGateway[i] = 0;
                }
                break;
                //Se non vi è un comando di invio allora mi aspetto i tempi e quindi 15 byte
            default:
                if (timerReadFromGateway >= 4) //if scatta dopo un timer di 4s
                {
                    readGatewayDone.Bit = 1;
                    readGatewayDone.Timeout = 1;
                    readGateway.Bit = 0;
                }

                if (dataFromGatewayIndex >= 15)
                {
                    readGatewayDone.Bit = 1;
                    readGatewayDone.Timeout = 0;
                    readGateway.Bit = 0;
                }
                break;
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
                //se il readgatewaydone non � stato richiamato dal timeout inizia la modifica dei dati
                else
                {
                    //bitParita(dataFromGateway); //controllo correttezza dati
                    SetReceivedTimes(dataFromGateway, Semafori);
                }
            }

            //ACCENSIONE LED IN BASE AL TEMPO
            //Cambiamento del timer ed eventuale cambio luci ogni secondo
            if (secondPassed.Bit && cycled.Bit)
            {
                for (unsigned char i = 0; i < n_semafori; i++) //Per ogni semaforo calcoler� il countdown per le luci in base alla luce
                {
                    if ((*Semafori[i]).times[0] != 0) //se per l'i-esimo semaforo � stato impostato un tempo diverso da 0 allora non � utilizzato e viene saltato
                    {
                        time[i]++; //incrementa il timer per il calcolo del countdown
                        unsigned char lux_select = (*Semafori[i]).lux_select;

                        if ((*Semafori[i]).times[lux_select] - time[i] < 0) //se il timer ha raggiunto il tempo della luce, quindi il countdown � terminato...
                        {
                            lux_select++; //...si incrementa il contatore delle luci...
                            time[i] = 1;  //...si resetta il timer

                            if (lux_select >= 3) //Se il contatore delle luci � arrivato a 3, ovvero � finito il giallo...
                            {
                                lux_select = 0; //...resetta il contatore delle luci, tornando al rosso...

                                if (i == 0) //...e se il ciclo terminato � quello del primo semaforo tutto l'incrocio ha terminato un ciclo...
                                {
                                    UpdateTimes(Semafori); //...e aggiorna i tempi delle luci...
                                }
                            }
                        }

                        luciSemaforo(i, lux_select);
                        (*Semafori[i]).lux_select = lux_select;                              //aggiorna il valore di l�ux_select nel caso sia cambiato
                        GetDigits(DigitsArr, i, (*Semafori[i]).times[lux_select] - time[i]); //ottiene le cifre delle centinaia, decine e unit� del countdown
                    }
                }
            }

            ShowDigitsOnDisplay();

            //reset variabili
            //Se � passato un secondo viene impostata a 1 la variabile "cycled" e il timer viene resettato solo al ciclo successivo, quando il codice entra in questo if.
            //in questo modo anche se l'interrupt imposta a 1 secondPassed dopo che il codice ha oltrepassato la parte di codice che attende
            //il timer, verr� effettuato un ciclo prima di resettare il timer cos� da assicurare che quelle porzioni di codice rilevino secondPassed
            if (secondPassed.Bit && cycled.Bit)
            {
                secondPassed.Bit = 0;
                cycled.Bit = 0;
            }
            if (secondPassed.Bit && !cycled.Bit)
            {
                cycled.Bit = 1;
            }
        }

        return;
    }
}
//inizializzo ADC (potenziometro)
void init_ADC()
{
    TRISA = 0xE3;   //imposto i pin come ingressi trane RA2 RA3 RA4
    ADCON0 = 0x00;  // setto ADCON0 00000000
    ADCON1 = 0x80;  // SETTO ADCON1 (ADFM) a 1 --> risultato giustificato verso dx 10000000
    __delay_us(10); //delay condensatore 10us
}

//leggo il valore del potenziometro
int ADC_Read(char canale)
{
    ADCON0 = (1 << ADON) | (canale << CHS0);
    __delay_us(2); //attendo 1.6 uS
    GO_nDONE = 1;  // avvio la conversione ADGO GO
    while (GO_nDONE)
        ;                          //attendo la fine della conversione
    return ADRESL + (ADRESH << 8); // preparo il dato (valore = ADRESL + (ADREAH << 8)
}

void UART_Init(int baudrate)
{
    TRISCbits.TRISC6 = 0; //TRISC= 0x80;   //10000000
    TXSTAbits.TXEN = 1;   //TXSTA= 0x20;   //00100000
    RCSTAbits.SPEN = 1;   //RCSTA= 0x90;   //10010000
    RCSTAbits.CREN = 1;   //RCSTA= 0x90;   //10010000
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
    unsigned char i;
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

void sendByte(char byte0, char byte1, char valore)
{
    char *txByte;
    txByte = BuildByte(byte0, byte1, valore);

    for (unsigned char i = 0; i < 5; i++)
    {
        UART_TxChar(*(txByte++)); //Invia un byte per volta
    }
}

void conteggioVeicoli()
{
    RoadsSensors[0] = Road1;
    RoadsSensors[1] = Road2;
    RoadsSensors[2] = Road3;
    RoadsSensors[3] = Road4;

    for (unsigned char i = 0; i < 4; i++)
    {
        if (!RoadsSensors[i]) //controllo pressione del tasto per il verificare se sia un motociclo macchina o un camion
        {
            count++;
        }
        else if (RoadsSensors[i]) //al rilascio controlla e incrementa il mezzo che è passato
        {
            Conteggio(count, motorcycle, car, truck, i);
            count = 0;
        }
    }
}

void luciSemaforo(unsigned char index, unsigned char lux) //Funzione per il cambio delle luci per la rappresentazione sui led rgb
{
    switch (index)
    {
    case 0: //Parte del primo semaforo (ID di esso)
        switch (lux)
        {
        case 0: //Accende luce rossa
            Green1 = 0;
            Yellow1 = 0;
            Red1 = 1;
            break;
        case 1: //Accende luce verde
            Red1 = 0;
            Yellow1 = 0;
            Green1 = 1;
            break;
        case 2: //Accende luce gialla
            Red1 = 0;
            Green1 = 0;
            Yellow1 = 1;
            break;
        }
        break;

    case 1: //Parte del secondo semaforo (ID di esso)
        switch (lux)
        {
        case 0: //Accende luce rossa
            Green2 = 0;
            Yellow2 = 0;
            Red2 = 1;
            break;
        case 1: //Accende luce verde
            Red2 = 0;
            Yellow2 = 0;
            Green2 = 1;
            break;
        case 2: //Accende luce gialla
            Red2 = 0;
            Green2 = 0;
            Yellow2 = 1;
            break;
        }
        break;
    }
}

void SetDisplay(unsigned char display_index, char d1, char d2, char d3, char value)
{
    switch (display_index)
    {
    case 0:
        Disp1s0 = d1;
        Disp2s0 = d2;
        Disp3s0 = d3;
        Disp1s2 = d1;
        Disp2s2 = d2;
        Disp3s2 = d3;
        DISP0 = value;
        break;
    case 1:
        Disp1s1 = d1;
        Disp2s1 = d2;
        Disp3s1 = d3;
        Disp1s3 = d1;
        Disp2s3 = d2;
        Disp3s3 = d3;
        //DISP1 = value;
        break;
    }
}

void ShowDigitsOnDisplay() //MOSTRA TIMER SU DISPLAY
{
    for (unsigned char display_index = 0; display_index < n_semafori; display_index++)
    {
        switch (disp) //fa lo scambio tra i display partendo dalle unita per arrivare alle centinaia per poi ricominciare
        {
        case 0:                                            //==> desplay delle centinaia, porta RA2
            if ((*DigitsArr[display_index]).centinaia > 0) //mostra la cifra delle centinaia solo se � consistente (maggiore di 0)
            {
                SetDisplay(display_index, 1, 0, 0, display[(*DigitsArr[display_index]).centinaia]); //Scrive su "PORTD" i pin che andranno a 1 per far vedere il numero che è presente nel array "display[*n]"
            }
            break;
        case 1:                                                                                      //==> desplay delle dedcine, porta RA3
            if ((*DigitsArr[display_index]).decine > 0 || (*DigitsArr[display_index]).centinaia > 0) //mostra la cifra delle decine e delle centinaia solo se sono consistenti (maggiore di 0), si considerano anche le centinaia per numeri come 102, in cui le decine non sono consistenti ma le centinaia si
            {
                SetDisplay(display_index, 0, 1, 0, display[(*DigitsArr[display_index]).decine]); //Scrive su "PORTD" i pin che andranno a 1 per far vedere il numero che è presente nel array "display[*n]"
            }
            break;
        case 2:                                                                             //==> desplay delle unit�, porta RA4
            SetDisplay(display_index, 0, 0, 1, display[(*DigitsArr[display_index]).unita]); //Scrive su "PORTD" i pin che andranno a 1 per far vedere il numero che è presente nel array "display[*n]"
            break;
        }
    }
    disp = (disp + 1) % 3; //disp viene incrementato e ha valori tra 0 e 2
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

    //TIMERS
    //se timer0 finisce di contare attiva l'interrupt ed esegue questo codice
    if (TMR0IF) //timer0 "TMR0IF"
    {
        TMR0IF = 0; //resetto timer0
        conteggioVeicoli();
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
