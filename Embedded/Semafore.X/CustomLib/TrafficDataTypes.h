typedef struct
{
    int times[3];
} Semaforo;

typedef struct
{
    int new_times[3];
    unsigned char id;
} Update;

typedef unsigned char ProtocolBytes[15];
typedef Semaforo *_Semafori[16];