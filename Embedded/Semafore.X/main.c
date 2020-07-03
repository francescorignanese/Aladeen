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
#include <stdlib.h>
#include "CustomLib/Conversions.h"
#include "CustomLib/BitsFlow.h"
#include "CustomLib/Serial.h"
#include "CustomLib/TrafficLight.h"

#define _XTAL_FREQ 32000000

#define ADON 0
#define CHS0 3
#define ADFM 7

#define Disp1 PORTAbits.RA2 //display1 7 segmenti
#define Disp2 PORTAbits.RA3 //display2 7 segmenti
#define Disp3 PORTAbits.RA4 //display3 7 segmenti

//*Conteggio veicoli -->
#define Road1 PORTBbits.RB3 //Sensore per il rivelamento dei mezzi su strada 1
#define Road2 PORTBbits.RB4 //Sensore per il rivelamento dei mezzi su strada 2
#define Road3 PORTBbits.RB5 //Sensore per il rivelamento dei mezzi su strada 3
#define Road4 PORTAbits.RA5 //Sensore per il rivelamento dei mezzi su strada 4
//* end <--

//*Inizializzazione delle luci -->
#define Lux_Sem1 PORTBbits.RB0  //ledRGB per il primo incrocio
#define Lux_Sem_1 PORTBbits.RB1 //ledRGB per il primo incrocio
#define Lux_Sem2 PORTBbits.RB6  //ledRGB per il secondo incrocio
#define Lux_Sem_2 PORTBbits.RB7 //ledRGB per il secondo incrocio
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

Bit readGateway, secondPassed, cycled;
//Array per la visualizzazione dei numeri sui display
const char display[11] = {0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x07, 0x7F, 0x6F};
unsigned char unita, decine, centinaia;                                        //varibile per scomporre il numero per il countdown e stamparlo sui display
unsigned char old_disp, disp;                                                  //varibile per fare lo switch in loop tra i dislpay
unsigned int count = 0;                                                        //variabile per il conteggio del tempo di pressione del tasto
unsigned char count_lux = 0;                                                   //conteggio per il tempo delle luci
unsigned char comando = 0;                                                     //Prende il dato dalla seriale
unsigned char time = 0;                                                        //variabile per contare i secondi
unsigned char motorcycle[4];                                                   //variabile per contare i motocicli
unsigned char car[4];                                                          //variabile per contare le macchine
unsigned char truck[4];                                                        //variabile per contare i camion
unsigned char dataFromGatewayIndex = 0;                                        //indice array dati da seriale
ProtocolBytes dataFromGateway;                                                 //array dati da seriale
Semaforo s0, s1, s2, s3, s4, s5, s6, s7, s8, s9, s10, s11, s12, s13, s14, s15; //definisco i vari semafori
Semaforo *Semafori[16] = {&s0, &s1, &s2, &s3, &s4, &s5, &s6, &s7, &s8, &s9, &s10, &s11, &s12, &s13, &s14, &s15};
unsigned char timerReadFromGateway; //timer per definire se la lettura dati eccede un tempo limite
unsigned char id_semaforo = 0;
Update to_update;

void init_ADC();                                    //Inizializza l'adc
int ADC_Read(char canale);                          //Lettura da un ingresso analogico
void UART_Init(int baudrate);                       //Inizializzazione della seriale con uno specifico baudrate
void UART_TxChar(char ch);                          //Scrittura di un carattere sulla seriale
char UART_Read();                                   //Lettura dalla seriale
void sendByte(char byte0, char byte1, char valore); //Funzione per inviare dati in cui vengono aggiunti i bit di parità
void conteggioVeicoli();                            //Conteggio mezzi
void sendByte(char byte0, char byte1, char valore);
void SetDisplay(char d1, char d2, char d3, char value);

void main(void)
{
    TRISB = 0x00; //gli utlimi tre bit per le luci, gli altri come ingresso
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

    int time;                     //0 � rosso, 1 � verde, 2 � giallo
    unsigned char lux_select = 0; //selezione luce per il semaforo
    disp = 0;                     //variabile per definire quale display deve accendersi, inizializzo a 0
    unsigned char temp = 0;       //Variabile per salvare la temperatura sul pin RA0
    unsigned char umidita = 0;    //Variabile per salvare l'umidita sul pin RA1
    unsigned char pressione = 0;  //Variabile per salvare la pressione sul pin RE0

    init_ADC();                          //Inizializzazione adc
    UART_Init(9600);                     //Inizializzazione seriale a 9600 b
    SetDefaultTimers(1, 1, 1, Semafori); //Inizializzazione tempi luci semaforo

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

                for (int i = 0; i < 4; i++) //Invio tutti i valori
                {
                    sendByte((0x01 << (i + 1)) | 0x01, 0x01, motorcycle[i]);
                    sendByte((0x01 << (i + 1)) | 0x01, 0x10, car[i]);
                    sendByte((0x01 << (i + 1)) | 0x01, 0x11, truck[i]);
                }

                for (int i = 0; i < 4; i++) //Reseto le variabili
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

                for (unsigned char i = 0; i < 3; i++)
                {
                    unsigned char tmp;
                    unsigned char index = i * 5;
                    //tmp=index;
                    //(*(Semafori[((index >> 1) & 0x0F)])).new_times[(((index >> 5) & 0x03) - 1)] = GetTime(index, dataFromGateway);
                    //Spiegazione riga precedente:
                    /*
                    unsigned char semaforoId = (index >> 1) & 0x0F;
                    unsigned char colorId = ((index >> 5) & 0x03) - 1;
                    */
                    tmp = index;
                    unsigned char semaforoId = (tmp >> 1) & 0x0F;
                    tmp = index;
                    unsigned char colorId = ((tmp >> 5) & 0x03) - 1;

                    if (semaforoId != id_semaforo)
                    {
                        (*Semafori[semaforoId]).times[colorId] = GetTime(index, dataFromGateway);
                    }
                    else
                    {
                        to_update.id = semaforoId;
                        to_update.new_times[colorId] = GetTime(index, dataFromGateway);
                    }
                }
            }
        }

        //ACCENSIONE LED IN BASE AL TEMPO
        //Cambiamento del timer ed eventuale cambio luci ogni secondo
        if (secondPassed.Bit && cycled.Bit)
        {
            time++;

            if ((*Semafori[id_semaforo]).times[lux_select] - time < 0) //se il timer ha raggiunto il tempo della luce, quindi il countdown � terminato...
            {
                lux_select++; //...si incrementa il contatore delle luci...
                time = 1;     //...si resetta il timer

                if (lux_select >= 3) //se il contatore delle luci � arrivato a 3, ovvero � finito il giallo...
                {
                    lux_select = 0; //...resetta il contatore delle luci, tornando al verde...

                    //AGGIORNAMENTO TEMPI LUCI E CAMBIO SEMAFORO
                    UpdateTimes(Semafori, &to_update);          //...aggiorna i tempi delle luci...
                    ChangeTrafficLight(Semafori, &id_semaforo); //...incrementa l'id del semaforo
                }
            }

            GetDigits(&centinaia, &decine, &unita, (*Semafori[id_semaforo]).times[lux_select] - time); //ottiene le cifre delle centinaia, decine e unit� del countdown
        }

        //MOSTRA TIMER SU DISPLAY
        if (disp != old_disp) //Lo esegue solo quando "disp" cambia (cio� ad ogni ciclo while))
        {
            old_disp = disp;
            switch (disp) //fa lo scambio tra i display partendo dalle unita per arrivare alle centinaia per poi ricominciare
            {
            case 0:                //==> desplay delle centinaia, porta RA2
                if (centinaia > 0) //mostra la cifra delle centinaia solo se � consistente (maggiore di 0)
                {
                    SetDisplay(1, 0, 0, display[centinaia]); //Scrive su "PORTD" i pin che andranno a 1 per far vedere il numero che è presente nel array "display[*n]"
                }
                //SetDisplay(1, 0, 0, display[(*Semafori[id_semaforo]).times[lux_select]]);
                SetDisplay(1, 0, 0, display[id_semaforo]);
                break;
            case 1:                              //==> desplay delle dedcine, porta RA3
                if (decine > 0 || centinaia > 0) //mostra la cifra delle decine e delle centinaia solo se sono consistenti (maggiore di 0), si considerano anche le centinaia per numeri come 102, in cui le decine non sono consistenti ma le centinaia si
                {
                    SetDisplay(0, 1, 0, display[decine]); //Scrive su "PORTD" i pin che andranno a 1 per far vedere il numero che è presente nel array "display[*n]"
                }
                //SetDisplay(0, 1, 0, display[lux_select]);
                break;
            case 2:                                  //==> desplay delle unit�, porta RA4
                SetDisplay(0, 0, 1, display[unita]); //Scrive su "PORTD" i pin che andranno a 1 per far vedere il numero che è presente nel array "display[*n]"
                //SetDisplay(0, 0, 1, display[id_semaforo]);
                break;
            }
        }
        disp = (disp + 1) % 3; //disp viene incrementato e ha valori tra 0 e 2

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

void SetDisplay(char d1, char d2, char d3, char value)
{
    Disp1 = d1;
    Disp2 = d2;
    Disp3 = d3;
    PORTD = value;
}

void conteggioVeicoli()
{
    //Controllo per la strada 1
    if (!Road1) //controllo pressione del tasto per il verificare se sia un motociclo macchina o un camion
    {
        count++;
    }
    else if (Road1) //al rilascio controlla e incrementa il mezzo che è passato
    {
        Conteggio(count, motorcycle, car, truck, 0);
        count = 0;
    }
    //Controllo per la strada 2
    if (!Road2) //controllo pressione del tasto per il verificare se sia un motociclo macchina o un camion
    {
        count++;
    }
    else if (Road2) //al rilascio controlla e incrementa il mezzo che è passato
    {
        Conteggio(count, motorcycle, car, truck, 1);
        count = 0;
    }
    //Controllo per la strada 3
    if (!Road3) //controllo pressione del tasto per il verificare se sia un motociclo macchina o un camion
    {
        count++;
    }
    else if (Road3) //al rilascio controlla e incrementa il mezzo che è passato
    {
        Conteggio(count, motorcycle, car, truck, 2);
        count = 0;
    }
    //Controllo per la strada 4
    if (!Road4) //controllo pressione del tasto per il verificare se sia un motociclo macchina o un camion
    {
        count++;
    }
    else if (Road4) //al rilascio controlla e incrementa il mezzo che è passato
    {
        Conteggio(count, motorcycle, car, truck, 3);
        count = 0;
    }
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
