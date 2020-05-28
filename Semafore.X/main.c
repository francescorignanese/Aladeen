/*
 * File:   main.c
 * Author: Rigna
 *
 * Created on 15 maggio 2020, 16.07
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

char str[4]; //stringa di salvatagio per la conversione da int to string
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

void init_ADC();                                                  //Inizializza l'adc
int ADC_Read(char canale);                                        //Lettura da un ingresso analogico
void intToString(int valore);                                     //Funzione di conversione da intero a una stringa
double pow(double x, double n);                                   //Funzione per fare la potenza
void UART_Init(int baudrate);                                     //Inizializzazione della seriale con uno specifico baudrate
void UART_TxChar(char ch);                                        //Scrittura di un carattere sulla seriale
void UART_Write_Text(char *text);                                 //Scrittura di una stringa sulla seriale
char UART_Read();                                                 //Lettura dalla seriale
int map(int x, int in_min, int in_max, int out_min, int out_max); //Funzione per mappare dei valori

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
    //richiesta dati al raspberry
    //atendi un tempo
    //oltre ciò se non ha ricevuto niente mette dei dati standard
    char Lux_Red = 1;
    char Lux_Yellow = 0;
    char Lux_Green = 0;
    while (1)
    {
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
        switch (lux_select)
        {
        case 0:
            Lux_Yellow = 0;
            Lux_Green = 0;
            Lux_Red = 1;
            countdown = Time_Red - time;
            break;
        case 1:
            Lux_Yellow = 0;
            Lux_Red = 0;
            Lux_Green = 1;
            countdown = Time_Green - time;
            break;
        case 2:
            Lux_Green = 0;
            Lux_Red = 0;
            Lux_Yellow = 1;
            countdown = Time_Yellow - time;
            break;
        }
    }
    return;
}

//inizializzo ADC (potenziometro)
void init_ADC()
{
    TRISA = 0xFF;   //imposto i pin come ingressi
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

double pow(double base, double exponent) //Funzioneper fare la potenza
{
    int i;
    double number = 1;
    for (i = 0; i < exponent; i++)
    {
        number *= base;
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
        UART_TxChar(text[i]);
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

void __interrupt() ISR()
{
    //ricevo il dato dal terminale
    if (RCIF)
    {
        comando = UART_Read();
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
            if (count >= 1000)
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
    if (TMR1IF) //timer1 "TMR1IF"
    {
        TMR1IF = 0; //resetto timer1
        count_lux++;
        if (count_lux >= 20)
        {
            time++;
            count_lux = 0;
        }

        TMR1H = 60;  // preset for timer1 MSB register
        TMR1L = 176; // preset for timer1 LSB register
    }
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
        //correction = (char)pow(2, errorColumn - 1);
        // rx[errorRow] = correction;
        rx[errorRow] = bitChage(rx[errorRow], errorColumn);
    }
}
