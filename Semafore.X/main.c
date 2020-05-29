/*
 * File:   main.c
 * Author: Rigna
 *
 * Created on 15 maggio 2020, 16.07
 TODO: Gestione sensori e controlli
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
#define Disp1 PORTAbits.RA2
#define Disp2 PORTAbits.RA3
#define Disp3 PORTAbits.RA4

struct
{
    unsigned int Bit : 1;
} readGateway;

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

char str[4]; //stringa di salvatagio per la conversione da int to string
const char display[11] = {0xEE, 0x28, 0xCD, 0x6D, 0x2B, 0x67, 0xE7, 0x2C, 0xEF, 0x6F};
char unita, decine, centinaia;
unsigned char disp = 0;
unsigned int count = 0;
unsigned char count_lux = 0;
char comando = 0; //Prende il dato dalla seriale
char by1 = 0;     //Primo byte ricevuto
char by2 = 0;     //Secondo byte ricevuto
unsigned char count_delay = 0;
unsigned char Time_Red = 10;
unsigned char Time_Yellow = 5;
unsigned char Time_Green = 10;
unsigned char time = 0;
char lux_select = 0;
unsigned char countdown = 0;
unsigned char car = 0;
unsigned char truck = 0;
char dataFromGatewayIndex = 0; //indice array dati da seriale
typedef char ProtocolBytes[5]; //array dati da seriale
ProtocolBytes dataFromGateway;
ProtocolBytes *Bytes[3];
int timerReadFromGateway; //timer per definire se la lettura dati eccede un tempo limite
int colorsTime[3];        //0 � rosso, 1 � verde, 2 � giallo
char colorIndex;

void init_ADC();                                                  //Inizializza l'adc
int ADC_Read(char canale);                                        //Lettura da un ingresso analogico
void intToString(int valore);                                     //Funzione di conversione da intero a una stringa
double pow(double b, double e);                                   //Funzione per fare la potenza
void UART_Init(int baudrate);                                     //Inizializzazione della seriale con uno specifico baudrate
void UART_TxChar(char ch);                                        //Scrittura di un carattere sulla seriale
void UART_Write_Text(char *text);                                 //Scrittura di una stringa sulla seriale
char UART_Read();                                                 //Lettura dalla seriale
int map(int x, int in_min, int in_max, int out_min, int out_max); //Funzione per mappare dei valori
char bitChage(char dato, char n);
void bitParita(char *rx);
int GetTime(ProtocolBytes data);

void main(void)
{
    TRISB = 0xBF;
    TRISC = 0x80;
    TRISD = 0x00;
    TRISE = 0x00;
    INTCON = 0xE0;     //abilito le varie variabili per chiamare gli interrupt
    OPTION_REG = 0x04; //imposto il prescaler a 1:32 del timer0
    TMR0 = 6;          //imposto il tempo iniziale a 6 per farlo attivare ogni 0,001 secondi
    T1CON = 0x31;      //Imposto il prescaler a 1:8 e attivo il timer1
    //TMR1 = 0x00;
    PIE1 = 0x01;
    //imposto il tempo iniziale a 15536 di timer1 per farlo attivare ogni 0, 050 secondi
    TMR1H = 60;  // preset for timer1 MSB register
    TMR1L = 176; // preset for timer1 LSB register
    /* 
    ?richiesta dati al raspberry 
    ?atendi un tempo oltre ciò se non ha ricevuto niente mette dei dati standard 
    */
    int colorsTime[3], time; //0 � rosso, 1 � verde, 2 � giallo
    char tmp;
    //*Inizializzazione delle luci -->
    char Lux_Red = 1;
    char Lux_Yellow = 0;
    char Lux_Green = 0;
    //* end <--

    while (1)
    {
        //se si stanno ricevendo dati dalla seriale
        if (readGateway.Bit)
        {
            if (timerReadFromGateway >= 80) //1s ogni 20 => if scatta dopo un timer di 4s
            {
                readGatewayDone.Bit = 1;
                readGatewayDone.Timeout = 1;
            }
            if (dataFromGatewayIndex >= 15)
            {
                readGatewayDone.Bit = 1;
                readGatewayDone.Timeout = 0;
                PORTB = 31;
            }
        }

        if (dataFromGatewayIndex >= 15)
        {
            readGatewayDone.Bit = 1;
            readGatewayDone.Timeout = 0;
        }

        //cose da fare terminata la lettura dalla seriale
        if (readGatewayDone.Bit)
        {
            //resetta le variabili per la lettura
            readGateway.Bit = 0;
            dataFromGatewayIndex = 0;

            //se c'� stato un timeout
            if (readGatewayDone.Timeout)
            {
                //cose da fare...
            }
            else
            {
                bitParita(dataFromGateway);

                for (int i = 0; i < 3; i++)
                {
                    //tmp=(dataFromGateway[0]>>5)&0x60;
                    colorIndex = ((*Bytes[i])[0] >> 5) & 0x60;
                    colorsTime[colorIndex] = GetTime(*Bytes[i]);
                }
            }
        }

        Time_Red = colorsTime[0];
        Time_Green = colorsTime[1];
        Time_Yellow = colorsTime[2];

        //cose da fare terminata la lettura dalla seriale
        if (readGatewayDone.Bit)
        {
            //resetta le variabili per la lettura
            readGateway.Bit = 0;
            dataFromGatewayIndex = 0;

            //se c'� stato un timeout
            if (readGatewayDone.Timeout)
            {
                //cose da fare...
            }
            else
            {
                bitParita(dataFromGateway);

                tmp = (dataFromGateway[0] >> 5) & 0x60;
                time = GetTime();

                colorsTime[tmp] = time;
            }
        }

        //! Parte da rivedere (Gestione delle luci e tempistica) -->
        if ((time >= Time_Red) && lux_select == 0)
        {
            time = 0;
            lux_select = 1;
        }
        if ((time >= Time_Yellow) && lux_select == 2)
        {
            lux_select = 0;
            time = 0;
        }
        if ((time >= Time_Green) && lux_select == 1)
        {
            lux_select = 2;
            time = 0;
        }
        //! end <--
        switch (lux_select) //Gestione delle luci del semaforo e del countdown per il display
        {                   //TODO: trasformare in una funzione che verrà poi richiamata
        case 0:
            /* 
            Vengono spente le luci degli altri colori e viene accesa solo quella che deve essere accesa
             */
            Lux_Yellow = 0;
            Lux_Green = 0;
            Lux_Red = 1;
            countdown = Time_Red - time;     //nella Variabile viene preso il tempo del colore e sottratto con il tempo che avanza
            centinaia = countdown / 100;     //Il tempo totale vine scomposto nelle varie parti per essere poi riportato nei display 7 segmenti (le centinaia)
            decine = (countdown % 100) / 10; //Il tempo totale vine scomposto nelle varie parti per essere poi riportato nei display 7 segmenti (le decine)
            unita = (countdown % 100) % 10;  //Il tempo totale vine scomposto nelle varie parti per essere poi riportato nei display 7 segmenti (le unita)
            break;
        case 1:
            Lux_Yellow = 0;
            Lux_Red = 0;
            Lux_Green = 1;
            countdown = Time_Green - time;
            centinaia = countdown / 100;
            decine = (countdown % 100) / 10;
            unita = (countdown % 100) % 10;
            break;
        case 2:
            Lux_Green = 0;
            Lux_Red = 0;
            Lux_Yellow = 1;
            countdown = Time_Yellow - time;
            centinaia = countdown / 100;
            decine = (countdown % 100) / 10;
            unita = (countdown % 100) % 10;
            break;
        }

        switch (disp) //fa lo scambio tra i display partendo dalle unita per arrivare alle centinaia per poi ricominciare
        {
        case 0:
            Disp2 = 0;
            Disp3 = 0;
            Disp1 = 1;
            PORTD = display[unita]; //Scrive su "PORTD" i pin che andranno a 1 per far vedere il numero che è presente nel array "display[*n]"
            break;
        case 1:
            Disp1 = 0;
            Disp3 = 0;
            Disp2 = 1;
            PORTD = display[decine];
            break;
        case 2:
            Disp1 = 0;
            Disp2 = 0;
            Disp3 = 1;
            PORTD = display[centinaia];
            break;
        }
    }
    return;
}
//inizializzo ADC (potenziometro)
void init_ADC()
{
    TRISA = 0xF3;   //imposto i pin come ingressi
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
            sommaRow += (rx[i] >> y) & 1;
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
            sommaColumn += (rx[y] >> i) & 1;
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

int GetTime(ProtocolBytes data)
{
    int time;
    struct
    {
        unsigned int Val : 7;
    } shortInt;

    shortInt.Val = dataFromGateway[3];
    time = shortInt.Val;
    time = time << 7;

    shortInt.Val = dataFromGateway[4];
    time = shortInt.Val;

    return time;
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
        disp++;
        if (count_lux >= 20)
        {
            time++;
            count_lux = 0;
        }
        if (disp == 3)
        {
            disp = 0;
        }
        TMR1H = 60;  // preset for timer1 MSB register
        TMR1L = 176; // preset for timer1 LSB register
    }
}

/*NON SERVE MA NON MI SENTO DI CANCELLARLO ANCORA
void ReadFromGateway()
{
	int i=1, error=0;
	timerReadFromGateway=0;

    PORTB=16+2;
	while(i<5)
	{
        dataFromGateway[i]=UART_Read();
        PORTB=16+i;
		timerReadFromGateway=0;
		i++;
	}
	
	if(timerReadFromGateway>=2000)
	{
		error=1;
	}
}
*/