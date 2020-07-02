typedef struct
{
    int times[3];
    int new_times[3];
    unsigned char lux_select;
    unsigned char new_lux_select;
} Semaforo;

/*
typedef struct
{
    int new_times[3];
    unsigned char id;
} Update;
*/

typedef unsigned char ProtocolBytes[15];
typedef Semaforo *_Semafori[16];