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
    unsigned int:1;
} bit;

bit readGateway=0;
char dataFromGateway[5];
char timerReadFromGateway=0;
int ColorsTime[3]; 	//array of colors time, 0 is red, 1 is green, 2 is yellow
char time=0; 	//variable to define which color is to light up, 0 is red, 1 is green, 2 is yellow

void UART_Init(int baudrate);                                     //Inizializzazione della seriale con uno specifico baudrate
char UART_Read();                                                 //Lettura dalla seriale
void ReadFromGateway();

void main(void)
{
    TRISB = 0xBF;
    TRISC = 0x80;
    TRISD = 0x00;
    TRISE = 0x00;
    INTCON = 0xA0;
    OPTION_REG = 0x05;
    TMR0 = 6;

    while (1)
    {
        if(readGateway)
        {
            ReadFromGateway();
        }
        PORTB=dataFromGateway[0];
    }


    return;
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
    //ricevo il dato dal terminale
    if (RCIF)
    {
        readGateway=1;
    }
}


void ReadFromGateway()
{
	char i=0, error=0;
	timerReadFromGateway=0;

	while(i<5 || timerReadFromGateway<2000)
	{
		dataFromGateway[i]=UART_Read();
		timerReadFromGateway=0,
		i++;
	}
	
	if(timerReadFromGateway>=2000)
	{
		error=1;
	}
}