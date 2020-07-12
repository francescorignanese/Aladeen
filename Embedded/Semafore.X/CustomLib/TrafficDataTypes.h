#include "Constants.h"

typedef struct
{
    int times[3];
    int new_times[3];
    unsigned char lux_select;
    unsigned char new_lux_select;
} Semaforo;

<<<<<<< HEAD
=======
typedef struct
{
    unsigned char Bit :1;
} Bit;

/*
>>>>>>> 776c670e5ab1a19894496c77a144d1d906280a1b
typedef struct
{
    unsigned int Bit : 1;
} Bit;

typedef unsigned char ProtocolBytes[15];
<<<<<<< HEAD
typedef Semaforo *_Semafori[8];
=======
typedef Semaforo *_Semafori[2];
>>>>>>> 776c670e5ab1a19894496c77a144d1d906280a1b
