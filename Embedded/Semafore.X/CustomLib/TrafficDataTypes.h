#include "Constants.h"

typedef struct
{
    int times[3];
    int new_times[3];
    unsigned char lux_select;
    unsigned char new_lux_select;
} Semaforo;

typedef struct
{
    unsigned int Bit : 1;
} Bit;

typedef unsigned char ProtocolBytes[15];
typedef Semaforo *_Semafori[8];