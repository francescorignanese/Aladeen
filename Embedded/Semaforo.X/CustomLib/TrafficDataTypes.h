typedef struct
{
    int times[3];
    int new_times[3];
    unsigned char lux_select;
    unsigned char new_lux_select;
} Semaforo;

typedef struct
{
    unsigned char Bit :1;
} Bit;


typedef struct
{
    char centinaia;
    char decine;
    char unita;
} Digits;

typedef unsigned char ProtocolBytes[15];
typedef Semaforo *_Semafori[2];
typedef Digits *_Digits[2];