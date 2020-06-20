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

#define _XTAL_FREQ 32000000

#define ADON 0
#define CHS0 3
#define ADFM 7

#define Disp1 PORTAbits.RA2 //display1 7 segmenti
#define Disp2 PORTAbits.RA3 //display2 7 segmenti
#define Disp3 PORTAbits.RA4 //display3 7 segmenti
//*Inizializzazione delle luci -->
#define Lux_Red PORTBbits.RB5    //luce rossa
#define Lux_Yellow PORTBbits.RB6 //luce gialla
#define Lux_Green PORTBbits.RB7  //luce verde
//* end <--

typedef struct
{
    unsigned int Bit : 1;
} Bit;

typedef int Times[3];
typedef struct
{
    Times new_times;
    Times times;
} Semaforo;

struct
{
    unsigned int Bit : 1;
    unsigned int Timeout : 1;
} readGatewayDone;

Bit readGateway, secondPassed, cycled;
char str[4]; //stringa di salvatagio per la conversione da int to string
//Array per la visualizzazione dei numeri sui display
const char display[11] = {0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x07, 0x7F, 0x6F};
char unita, decine, centinaia; //varibile per scomporre il numero per il countdown e stamparlo sui display
unsigned char old_disp, disp;  //varibile per fare lo switch in loop tra i dislpay
unsigned int count = 0;        //variabile per il conteggio del tempo di pressione del tasto
unsigned char count_lux = 0;   //conteggio per il tempo delle luci
char comando = 0;              //Prende il dato dalla seriale
unsigned char time = 0;        //variabile per contare i secondi
unsigned char countdown = 0;   //variabile per il conto alla rovescia
unsigned char motorcycle[4];   //variabile per contare le macchine
unsigned char car[4];          //variabile per contare le macchine
unsigned char truck[4];        //variabile per contare i camion
char dataFromGatewayIndex = 0; //indice array dati da seriale
typedef char ProtocolBytes[15];
ProtocolBytes dataFromGateway;                                                 //array dati da seriale
Semaforo s0, s1, s2, s3, s4, s5, s6, s7, s8, s9, s10, s11, s12, s13, s14, s15; //definisco i vari semafori
Semaforo *Semafori[16] = {&s0, &s1, &s2, &s3, &s4, &s5, &s6, &s7, &s8, &s9, &s10, &s11, &s12, &s13, &s14, &s15};
char timerReadFromGateway; //timer per definire se la lettura dati eccede un tempo limite
//Times colorsTime, new_colorsTime;            //0 � rosso, 1 � verde, 2 � giallo
char colorIndex; //variabile per stabilire il colore da accendere
int n_semafori;

void init_ADC();                                    //Inizializza l'adc
int ADC_Read(char canale);                          //Lettura da un ingresso analogico
void UART_Init(int baudrate);                       //Inizializzazione della seriale con uno specifico baudrate
void UART_TxChar(char ch);                          //Scrittura di un carattere sulla seriale
char UART_Read();                                   //Lettura dalla seriale
int GetTime(int index);                             //Fonde un numero separato in due bit in un singolo int
void GetDigits(int Time);                           //Suddivide un numero secondo centinaia, decine e unit�
void sendByte(char byte0, char byte1, char valore); //Invia un blocco da 5 byte al raspberry
void conteggioVeicoli();
void SetDisplay(char d1, char d2, char d3, char value);  //Seleziona quale display accendere
void SetDefaultTimers(int rosso, int verde, int giallo); //setta i tempi di default delle luci del semaforo

void main(void)
{
    //Init
    TRISB = 0x1F; //gli utlimi tre bit per le luci, gli altri come ingresso
    TRISC = 0x80;
    TRISD = 0x00;      //Porta per i 7 segmenti (Output)
    TRISE = 0x01;      //Utilizzo l'ingresso RE0 per misurare la pressione
    INTCON = 0xE0;     //abilito le varie variabili per chiamare gli interrupt
    OPTION_REG = 0x04; //imposto il prescaler a 1:32 del timer0
    TMR0 = 6;          //imposto il tempo iniziale a 6 per farlo attivare ogni 0,001 secondi
    T1CON = 0x31;      //Imposto il prescaler a 1:8 e attivo il timer1
    //?PIE1 = 0x01;
    /* 
    ?richiesta dati al raspberry 
    ?atendi un tempo oltre ciò se non ha ricevuto niente mette dei dati standard 
    */
    init_ADC();                //Inizializzazione adc
    UART_Init(9600);           //Inizializzazione seriale a 9600 b
    SetDefaultTimers(0, 0, 0); //Inizializzazione tempi luci semaforo
    //imposto il tempo iniziale a 15536 di timer1 per farlo attivare ogni 0, 050 secondi
    TMR1H = 60;  // preset for timer1 MSB register
    TMR1L = 176; // preset for timer1 LSB register

    int colorsTime[3], time; //0 � rosso, 1 � verde, 2 � giallo
    char lux_select = 0;     //selezione luce per il semaforo
    disp = 0;                //variabile per definire quale display deve accendersi, inizializzo a 0
    char temp = 0;           //Variabile per salvare la temperatura sul pin RA0
    char umidita = 0;        //Variabile per salvare l'umidita sul pin RA1
    char pressione = 0;      //Variabile per salvare la pressione sul pin RE0
    Bit endCiclo;            //variabile per il controllo del ciclo così da cambiare i tempi solo a fine del ciclo
    endCiclo.Bit = 1;

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
                break;
            case 0x0A: //Se vi è il comando 0x0A nel primo byte allora prendo solo un pachetto di dati
                readGatewayDone.Bit = 1;
                readGateway.Bit = 0;
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
            //se il readgatewaydonenon � stato richiamato dal timeout inizia la modifica dei dati
            else
            {
                //bitParita(dataFromGateway); //controllo correttezza dati

                for (int i = 0; i < 3; i++)
                {
                    int index = i * 5;
                    int colorId = ((dataFromGateway[i * 5] >> 5) & 0x03) - 1;
                    int semaforoId = (dataFromGateway[0] >> 1) & 0x07;

                    (*(Semafori[semaforoId])).new_times[colorId] = GetTime(index);
                }
            }
        }

        //AGGIORNAMENTO TEMPI LUCI
        //se avviene qualche cambiamento allora aggornero i tempi
        if (endCiclo.Bit)
        {
            n_semafori = (n_semafori + 1) % 16;
            while ((*(Semafori[n_semafori])).times[0] == 0)
            {
                n_semafori++;
            }

            for (int l = 0; l < 16; l++)
            {
                for (int i = 0; i < 3; i++)
                {
                    if ((*(Semafori[l])).times[i] != (*(Semafori[l])).new_times[i])
                    {
                        (*(Semafori[l])).times[i] = (*(Semafori[l])).new_times[i];
                    }
                }
            }
        }

        //ACCENSIONE LED IN BASE AL TEMPO
        //Cambiamento del timer ed eventuale cambio luci ogni secondo
        if (secondPassed.Bit && cycled.Bit)
        {
            time++;
            endCiclo.Bit = 0;

            if ((*Semafori[n_semafori]).times[lux_select] - time < 0)
            {
                endCiclo.Bit = 1;
            }

            if (lux_select == 2 && time == (*Semafori[n_semafori]).times[2])
            {
                endCiclo.Bit = 1;
            }

            GetDigits((*Semafori[n_semafori]).times[lux_select] - time);
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
                break;
            case 1:                              //==> desplay delle dedcine, porta RA3
                if (decine > 0 || centinaia > 0) //mostra la cifra delle decine e delle centinaia solo se sono consistenti (maggiore di 0), si considerano anche le centinaia per numeri come 102, in cui le decine non sono consistenti ma le centinaia si
                {
                    SetDisplay(0, 1, 0, display[decine]); //Scrive su "PORTD" i pin che andranno a 1 per far vedere il numero che è presente nel array "display[*n]"
                }
                break;
            case 2:                                  //==> desplay delle unit�, porta RA4
                SetDisplay(0, 0, 1, display[unita]); //Scrive su "PORTD" i pin che andranno a 1 per far vedere il numero che è presente nel array "display[*n]"
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

        //!Parte di invio mezzi ad ogni richiesta del raspberry da completare la ricezione del comando -->
        if ((dataFromGateway[0] & 0x7F) == 0x08)
        {
            for (int i = 0; i < 4; i++) //Invio tutti i valori
            {
                sendByte((0x01 << (i + 1)), 0x01, motorcycle[i]);
                sendByte((0x01 << (i + 1)), 0x10, car[i]);
                sendByte((0x01 << (i + 1)), 0x11, truck[i]);
            }
            for (int i = 0; i < 4; i++) //Reseto le variabili
            {
                motorcycle[i] = 0;
                car[i] = 0;
                truck[i] = 0;
            }
            // for (unsigned char i = 0; i < 5; i++) //Resetto i byte che ho ricevuto così da non continuare ad inviare
            // {
            //     dataFromGateway[i] = 0;
            // }
        }
        //!end <--

        //*Parte di invio dei sensori ad ogni richiesta del raspberry
        if ((dataFromGateway[0] & 0x7F) == 0x0A)
        {
            temp = (char)map((ADC_Read(0) >> 2), 0, 255, -20, 60);     //legge la temperatura e la mappa su quei valori
            umidita = (char)map((ADC_Read(1) >> 2), 0, 255, 0, 100);   //legge l'umidità e la mappa su quei valori
            pressione = (char)map((ADC_Read(5) >> 2), 0, 255, 0, 100); //legge la pressione e la mappa su quei valori
            sendByte(0x02, 0x00, temp);                                //Invio dati di temperatura
            sendByte(0x04, 0x00, umidita);                             //Invio dati di umidita
            sendByte(0x06, 0x00, pressione);                           //Invio dati di pressione
            // for (unsigned char i = 0; i < 5; i++)                      //Resetto i byte che ho ricevuto così da non continuare ad inviare
            // {
            //     dataFromGateway[i] = 0;
            // }
        }
        //*end <--
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

void sendByte(char byte0, char byte1, char valore)
{
    char *txByte;
    txByte = BuildByte(byte0, byte1, valore);

    for (int i = 0; i < 5; i++)
    {
        UART_TxChar(txByte++); //Invia un byte per volta
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
    time = (time << 7) & 0x80;

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
    if (!PORTBbits.RB3) //controllo pressione del tasto per il verificare se sia un motociclo macchina o un camion
    {
        count++;
    }
    else if (PORTBbits.RB3) //al rilascio controlla e incrementa il mezzo che è passato
    {
        if (count >= 500)
        {
            motorcycle[0]++;
        }
        if (count >= 1500)
        {
            car[0]++;
        }
        if (count >= 3000)
        {
            truck[0]++;
        }
        count = 0;
    }
    //Controllo per la strada 2
    if (!PORTBbits.RB4) //controllo pressione del tasto per il verificare se sia un motociclo macchina o un camion
    {
        count++;
    }
    else if (PORTBbits.RB4) //al rilascio controlla e incrementa il mezzo che è passato
    {
        if (count >= 500)
        {
            motorcycle[1]++;
        }
        if (count >= 1500)
        {
            car[1]++;
        }
        if (count >= 3000)
        {
            truck[1]++;
        }
        count = 0;
    }
    //Controllo per la strada 3
    if (!PORTBbits.RB5) //controllo pressione del tasto per il verificare se sia un motociclo macchina o un camion
    {
        count++;
    }
    else if (PORTBbits.RB5) //al rilascio controlla e incrementa il mezzo che è passato
    {
        if (count >= 500)
        {
            motorcycle[2]++;
        }
        if (count >= 1500)
        {
            car[2]++;
        }
        if (count >= 3000)
        {
            truck[2]++;
        }
        count = 0;
    }
    //Controllo per la strada 4
    if (!PORTAbits.RA5) //controllo pressione del tasto per il verificare se sia un motociclo macchina o un camion
    {
        count++;
    }
    else if (PORTAbits.RA5) //al rilascio controlla e incrementa il mezzo che è passato
    {
        if (count >= 500)
        {
            motorcycle[3]++;
        }
        if (count >= 1500)
        {
            car[3]++;
        }
        if (count >= 3000)
        {
            truck[3]++;
        }
        count = 0;
    }
}

void SetDefaultTimers(int rosso, int verde, int giallo)
{
    for (int l = 0; l < 16; l++)
    {
        for (int i = 0; i < 3; i++)
        {
            switch (i)
            {
            case 0:
                (*(Semafori[l])).new_times[i] = rosso;
                break;
            case 1:
                (*(Semafori[l])).new_times[i] = verde;
                break;
            case 2:
                (*(Semafori[l])).new_times[i] = giallo;
                break;
            }
        }
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
