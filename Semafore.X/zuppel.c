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

struct
{
    unsigned int Bit:1;
} readGateway;

struct
{
    unsigned int Bit:1;
    unsigned int Timeout:1;
} readGatewayDone;

char dataFromGateway[5];
char time=0; 	//variable to define which color is to light up, 0 is red, 1 is green, 2 is yellow
int elapsed=0;
int timer=0;
int dataFromGatewayIndex=0;
int timerReadFromGateway=0;

void UART_Init(int baudrate);                                     //Inizializzazione della seriale con uno specifico baudrate
char UART_Read();                                                 //Lettura dalla seriale
void ReadFromGateway();
void UART_TxChar(char ch);

void main(void)
{
    TRISB = 0x00;
    TRISC = 0x80;
    TRISD = 0x00;
    TRISE = 0x00;
    INTCON = 0xE0;
    T1CON = 0x31;
    TMR1H = 60;             // preset for timer1 MSB 
    TMR1L = 176;             // preset for timer1 LSB 
    PIE1 = 0x01;
    OPTION_REG = 0x05;
    TMR0 = 6;
    
    UART_Init(9600);
    
    readGateway.Bit=0;

    time=0;
    
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
                PORTB=31;
                //cose da fare...
            }
            else
            {
                //cose da fare...
            }
        }
    }
    
    return;
}

void UART_TxChar(char ch)
{
    while (!TXIF);      //se TXIF ? a 0 la trasmissione ? ancora in corso
    TXIF = 0;           //lo resetto
    TXREG = ch;
}


void UART_Init(int baudrate)
{
    TRISCbits.TRISC6 = 0;   //TRISC= 0x80;   //10000000

    TXSTAbits.TXEN = 1;     //TXSTA= 0x20;   //00100000

    RCSTAbits.SPEN = 1;     //RCSTA= 0x90;   //10010000
    RCSTAbits.CREN = 1;     //RCSTA= 0x90;   //10010000

    SPBRG = (_XTAL_FREQ / (long)(64UL * baudrate)) - 1;
    INTCONbits.GIE = 1;     //abilito global interrupt
    INTCONbits.PEIE = 1;    //peripherial interrupt
    PIE1bits.RCIE = 1;      //uart rx interrupt
}


char UART_Read()
{
    while (!RCIF);
    RCIF = 0;
    return RCREG;
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
        dataFromGatewayIndex++;
        timerReadFromGateway=0;
    }
    
    
        if (TMR0IF) //timer0 "TMR0IF"
    {
        TMR0IF = 0; //resetto timer0
        TMR0 = 6;
    }
    
    
    //se timer1 finisce di contare attiva l'interrupt ed esegue questo codice
    if (TMR1IF) //timer1 "TMR1IF", DURATA: 0.05s
    {
        TMR1IF = 0;
        timerReadFromGateway++;
        TMR1H = 60;             // preset for timer1 MSB 
        TMR1L = 176;            // preset for timer1 LSB 
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