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

char dataFromGateway[5];
char str[4];                    //stringa di salvatagio per la conversione da int to string
unsigned int count = 0;
unsigned char update = 0;       //variabile per aggiornare i valori sul men�
char comando = 0;               //Prende il dato dalla seriale
unsigned char count_pwm = 0;
char by1 = 0;                   //Primo byte ricevuto
char by2 = 0;                   //Secondo byte ricevuto
char timerReadFromGateway=0;
unsigned char count_delay = 0;
unsigned char Time_Red = 10;
unsigned char Time_Yellow = 5;
unsigned char Time_Green = 10;

int ColorsTime[3]; 	//array of colors time, 0 is red, 1 is green, 2 is yellow
char time=0; 	//variable to define which color is to light up, 0 is red, 1 is green, 2 is yellow

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
    INTCON = 0xA0;
    OPTION_REG = 0x05;
    TMR0 = 6;
    char Lux_Red = 1;
    char Lux_Yellow = 0;
    char Lux_Green = 0;


    while (1)
    {
	/*
	switch (time)
        {
        case 0:
		color=time
            break;
        case 1:
            Lux_Yellow = 0;
            Lux_Red = 0;
            Lux_Green = 1;

            break;
        case 2:
            Lux_Green = 0;
            Lux_Red = 0;
            Lux_Yellow = 1;
            break;
        case 3:
            time = 0;
            count_lux = 0;
            break;
        }
	*/


	if(readGateway)
	{
		ReadFromGateway();
	}
    }


    return;
}

//inizializzo ADC (potenziometro)
void init_ADC()
{
    TRISA = 0xFF;   //imposto i pin come ingressi
    ADCON0 = 0x00;  //setto ADCON0 00000000
    ADCON1 = 0x80;  //SETTO ADCON1 (ADFM) a 1 --> risultato giustificato verso dx 10000000
    __delay_us(10); //delay condensatore 10us
}

//leggo il valore del potenziometro
int ADC_Read(char canale)
{
    ADCON0 = (1 << ADON) | (canale << CHS0);
    //ADCON0bits.ADON = 1;               //accendo il convertitore (ADCON0)
    //ADCON0 |= canale << 3;             //e setto il canale da convertire (ADCON0)
    __delay_us(2);                       //attendo 1.6 uS
    GO_nDONE = 1;                        // avvio la conversione ADGO GO
    while (GO_nDONE);                    //attendo la fine della conversione
    return ADRESL + (ADRESH << 8);       // preparo il dato (valore = ADRESL + (ADREAH << 8)
}

void intToString(int valore)
{
    int i;
    int num = 0;
    for (i = 0; i < 3; i++)
    {
        str[2 - i] = '0' + ((valore % (char)pow(10, 1 + i)) / (char)pow(10, i));
    }
    str[3] = '\0';
}

double pow(double Base, double potenza)
{
    int i;
    double number = 1;
    for (i = 0; i < potenza; i++)
    {
        number *= Base;
    }
    return (number);
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

void UART_TxChar(char ch)
{
    while (!TXIF);      //se TXIF ? a 0 la trasmissione ? ancora in corso
    TXIF = 0;           //lo resetto
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

int map(int x, int in_min, int in_max, int out_min, int out_max)
{
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

void __interrupt() ISR()
{
    //ricevo il dato dal terminale
    if (RCIF)
    {
	readGateway=1;
    }
    if (INTCON & 0x04)
    {
        INTCON &= ~0x04;
        count++;
	timerReadFromGateway++; 	//increase timer fro read data from gateway by one
        if (count >= Time_Red)
        {
            update = 1;
        }

        TMR0 = 6;
    }
    if (TMR1IF == 1)
    {
        //value = ~value;   // complement the value for blinking the LEDs
        TMR1H = 0x0B;       // Load the time value(0xBDC) for 100ms delay
        TMR1L = 0xDC;
        TMR1IF = 0;         // Clear timer interrupt flag
    }
}

char mem[5]=
check(byte)
{
if (byte%2)
{
    
}

}
bool getParity(unsigned int n)
{
    bool parity = 0;
    while (n)
    {
        parity = !parity;
        n = n & (n - 1);
    }
    return parity;
}
/* Driver program to test getParity() */
int main()
{
    unsigned int n = 7;
    printf("Parity of no %d = %s", n,
           (getParity(n) ? "odd" : "even"));

    getchar();
    return 0;
}


void ReadFromGateway()
{
	char i=0, error=0;
	timerReadFromGateway=0,

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
}



while(time<ColorTimes[i])
{
	Colors[i]=1;
	time++;
}
Colors[i]=0;
time=0;
i++;

if (TMR1IF) //timer1 "TMR1IF"
    {
        TMR1IF = 0;
        count_lux++;
        if (count_lux >= Times[0] && time==0)
        {
            time = 1;
	count_lux=0;
        }
        if ((count_lux >= Times[2]) && time == 2)
        {
            time = 0;
	count_lux=0;
        }
        if ((count_lux >= Times[1]) && time == 1)
        {
            time = 2;
	count_lux=0;
        }
    }


char rx[5] = 0;
bitParita(byte)
{
    char columnNumber = 0;
    char mem = 0;
    char row[5] = 0;
    char column = 0;
    char z = 0;
 
	char ones=0;
    for (int i = 0; i < 6; i++)
    {
	
        row[i] = rx[i] % 2;
    }
    for (int i = 0; i < 6; i++)
    {
      for(int n=0; n<8; n++)
      {
        if (row[i][n] == 1)
        {
            	while (check == 0)
            	{
                	for (int y = 0; y < 5; y++)
                	{
                    	column += (rx[y] >> z) & 0x01;
               	 }
                	if (column % 2 == 1)
                	{
                    	check = 1;
                	}
                	else
                	{
                    	columnNumber += 1;
                    	z += 1;
                	}
            	}
          }
 
        }
    }
}

