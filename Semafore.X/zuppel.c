/*
 * File:   main.c
 * Author: Rigna, Zuppel
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

struct
{
    unsigned int Bit:1;
} readGateway;

struct
{
    unsigned int Bit:1;
    unsigned int Timeout:1;
} readGatewayDone;

/*
typedef struct
{
   unsigned int Value:14;
}Time;
*/

char str[4]; //stringa di salvatagio per la conversione da int to string
unsigned int count = 0, count_lux = 0;
char comando = 0; //Prende il dato dalla seriale
char by1 = 0;     //Primo byte ricevuto
char by2 = 0;     //Secondo byte ricevuto
unsigned char count_delay = 0, Time_Red = 10, Time_Yellow = 5, Time_Green = 10;
char time = 0, car = 0, truck = 0, colorIndex=0;
char dataFromGatewayIndex=0;            //indice array dati da seriale
typedef char ProtocolBytes[5];          //array dati da seriale
ProtocolBytes dataFromGateway;
ProtocolBytes Bytes[3];
int timerReadFromGateway;               //timer per definire se la lettura dati eccede un tempo limite

void init_ADC();                                                  //Inizializza l'adc
int ADC_Read(char canale);                                        //Lettura da un ingresso analogico
void intToString(int valore);                                     //Funzione di conversione da intero a una stringa
double pow(double x, double n);                                   //Funzione per fare la potenza
void UART_Init(int baudrate);                                     //Inizializzazione della seriale con uno specifico baudrate
void UART_TxChar(char ch);                                        //Scrittura di un carattere sulla seriale
void UART_Write_Text(char *text);                                 //Scrittura di una stringa sulla seriale
char UART_Read();                                                 //Lettura dalla seriale
int map(int x, int in_min, int in_max, int out_min, int out_max); //Funzione per mappare dei valori
void bitParita(char *rx);
int GetTime();

void main(void)
{
    TRISB = 0xBF;
    TRISC = 0x80;
    TRISD = 0x00;
    TRISE = 0x00;
    INTCON = 0xE0;
    T1CON = 0x31;
    TMR1H = 60;             // preset for timer1 MSB 
    TMR1L = 176;            // preset for timer1 LSB 
    PIE1 = 0x01;
    OPTION_REG = 0x05;
    TMR0 = 6;
    //richiesta dati al raspberry
    //atendi un tempo
    //oltre ciÃ² se non ha ricevuto niente mette dei dati standard
    int colorsTime[3], time;     //0 è rosso, 1 è verde, 2 è giallo
    char tmp;
    
    while (1)
    {
        //se si stanno ricevendo dati dalla seriale
        if(readGateway.Bit)
        {
            if(timerReadFromGateway>=80)
            {
                readGatewayDone.Bit=1;
                readGatewayDone.Timeout=1;
            }
            
            if(dataFromGatewayIndex>=5)
            {
                readGatewayDone.Bit=1;
                readGatewayDone.Timeout=0;
            }
        }
        
        //cose da fare terminata la lettura dalla seriale
        if(readGatewayDone.Bit)
        {
            //resetta le variabili per la lettura
            readGateway.Bit=0;
            dataFromGatewayIndex=0;
            
            //se c'è stato un timeout
            if(readGatewayDone.Timeout)
            {
                //cose da fare...
            }
            else
            {
                bitParita(dataFromGateway);
                
                tmp=(dataFromGateway[0]>>5)&0x60;
                time=GetTime();
                
                colorsTime[tmp]=time;
            }
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
    while (GO_nDONE);                          //attendo la fine della conversione
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
    while (!TXIF);     //se TXIF ? a 0 la trasmissione ? ancora in corso
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
    while (!RCIF);
    RCIF = 0;
    return RCREG;
}

int map(int x, int in_min, int in_max, int out_min, int out_max) //Mappare nuovamente un numero da un intervallo a un altro
{
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

int GetTime()
{
    int time;
    struct
    {
        unsigned int Val:7;
    }shortInt;
    
    shortInt.Val=dataFromGateway[3];
    time=shortInt.Val;
    time=time<<7;
    
    shortInt.Val=dataFromGateway[4];
    time=shortInt.Val;
    
    return time;
}

void __interrupt() ISR()
{
   //RICEVE DATI DA SERIALE
    if(RCIF && readGateway.Bit==0)
    {
        readGateway.Bit=1;
        readGatewayDone.Bit=0;
        readGatewayDone.Timeout=0;
        dataFromGatewayIndex=0;
        timerReadFromGateway=0;
    }
    if(RCIF && readGateway.Bit==1)
    {
        dataFromGateway[dataFromGatewayIndex]=UART_Read();
        
        if(dataFromGatewayIndex==0)
        {
            colorIndex=(dataFromGateway[dataFromGatewayIndex]>>5)&0x60;
        }
        
        dataFromGatewayIndex++;
        timerReadFromGateway=0;
    }
    
    
    
    
    
    //se timer0 finisce di contare attiva l'interrupt ed esegue questo codice
    if (TMR0IF) //timer0 "TMR0IF"
    {
        TMR0IF = 0; //resetto timer0
        if (!PORTBbits.RB3)
        {
            count++;
        }
        if (PORTBbits.RB3)
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
    if (TMR1IF) //timer1 "TMR1IF", DURATA: 1s
    {
        TMR1IF = 0;
        timerReadFromGateway++;
        
        TMR1H = 60;             // preset for timer1 MSB 
        TMR1L = 176;            // preset for timer1 LSB 
    }
}

char bitChage(char dato, char n)
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

void bitParita(char *rx)
{
    // a = (rx[0] ^ rx[1] ^ rx[2] ^ rx[3]);
    // if ((rx[0] ^ rx[1] ^ rx[2] ^ rx[3]) != rx[4])
    // {
    // }
    // for (int i = 0; i < 8; i++)
    // {
    //     if ((a & (1 << i)) != (d[4] & (1 << i)))
    //     {
    //         char errore;
    //     }
    // }
    // #define bitRead(value, bit) (((value) >> (bit)) & 0x01)
    // #define bitSet(value, bit) ((value) |= (1UL << (bit)))
    // #define bitClear(value, bit) ((value) &= ~(1UL << (bit)))
    // #define bitWrite(value, bit, bitvalue) (bitvalue ? bitSet(value, bit) : bitClear(value, bit))
    char sommaRow = 0;
    char error = 0;
    char errorRow = 0;
    char sommaColumn = 0;
    char errorColumn = 0;
    char correction = 0;
    for (int i = 0; i < 5; i++)
    {
        for (int y = 0; y < 8; y++)
        {
            sommaRow += (rx[i] >> y) & 1;
        }
        if (sommaRow % 2 == 1)
        {
            error = 1;
            errorRow = i;
        }
    }
    for (int i = 0; i < 8; i++)
    {
        for (int y = 0; y < 4; y++)
        {
            sommaColumn += (rx[y] >> i) & 1;
        }
        if (sommaColumn % 2 == 1)
        {
            error = 1;
            errorColumn = i;
        }
    }
    if (error != 0)
    {
        //correction = (char)pow(2, errorColumn - 1);
        // rx[errorRow] = correction;
        rx[errorRow] = bitChage(rx[errorRow], errorColumn);
    }
}


/* NON SERVE MA NON MI SENTO DI CANCELLARLO ANCORA
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