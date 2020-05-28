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

#define _XTAL_FREQ 8000000

#define ADON 0
#define CHS0 3
#define ADFM 7

struct
{
    unsigned int Bit:1;
} readGateway;

char dataFromGateway[5];
char time=0; 	//variable to define which color is to light up, 0 is red, 1 is green, 2 is yellow
int timer=0;
int i=0;
int timerReadFromGateway;

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
    INTCON = 0xA0;
    OPTION_REG = 0x05;
    TMR0 = 6;
    UART_Init(9600);
    
    readGateway.Bit=0;

    time=0;
    
    while (1)
    {
        if(readGateway.Bit)
        {
            if(i>=5)
            {
                readGateway.Bit=0;
                i=0;
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
    //aumenta il timer dei semafori per limitare il ritardo generato dal richiamo dell'interrupt
    timer++;
    
    //RICEVE DATI DA SERIALE
    if(RCIF && readGateway.Bit==0)
    {
        readGateway.Bit=1;
        i=0;
    }
    if(RCIF && readGateway.Bit==1)
    {
        dataFromGateway[i]=UART_Read();
        i++;
    }
    
    
    
    
    if (INTCON & 0x04)
    {
        INTCON &= ~0x04;
        TMR0 = 6;
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