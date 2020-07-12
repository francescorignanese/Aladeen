#include "TrafficDataTypes.h"
#include "Constants.h"

void UpdateTimes(_Semafori _semafori)
{
    //for su tutti i semafori per aggiornare eventuali modifiche ai timers
    for (unsigned char l = 0; l < n_semafori; l++)
    {
        for (unsigned char i = 0; i < 3; i++)
        {
            if ((*(_semafori)[l]).times[i] != (*(_semafori)[l]).new_times[i])
            {
                (*(_semafori)[l]).times[i] = (*(_semafori)[l]).new_times[i];
            }
        }

        if ((*(_semafori)[l]).lux_select != (*(_semafori)[l]).new_lux_select)
        {
            (*(_semafori)[l]).lux_select = (*(_semafori)[l]).new_lux_select;
        }
    }
}

void ChangeTrafficLight(_Semafori _semafori, unsigned char *_n_semafori)
{
    //incrementando n_semafori quando il tempo ï¿½ 0; assicura che vengano saltati i semafori che non vengoo inizializzati dal raspberry, quindi inesistenti nell'incrocio
    //si usa il do while così da incrementare almeno una volta
    do
    {
        *_n_semafori = ((*_n_semafori) + 1);
    } while ((*(_semafori)[*_n_semafori]).times[0] == 0 && *_n_semafori < 10);

    *_n_semafori = (*_n_semafori) % 10;
}

//Compone in un int un numero scomposto in due byte
int GetTime(unsigned char index, ProtocolBytes _dataFromGateway)
{
    int tmp;
    struct
    {
        unsigned int Val : 7;
    } shortInt;

    shortInt.Val = _dataFromGateway[index + 3] & 0x7F;
    tmp = shortInt.Val;
    tmp = (tmp << 7) & 0x80;

    shortInt.Val = _dataFromGateway[index + 2] & 0x7F;
    tmp = tmp | shortInt.Val;

    return tmp;
}

//setta i timer dei semafori
void SetDefaultTimers(int rosso, int verde, int giallo, _Semafori _semafori)
{
    for (unsigned char l = 0; l < n_semafori; l++)
    {
        for (unsigned char i = 0; i < 3; i++)
        {
            switch (i)
            {
            case 0:
                (*(_semafori)[l]).times[i] = rosso;
                (*(_semafori)[l]).new_times[i] = rosso;
                break;
            case 1:
                (*(_semafori)[l]).times[i] = verde;
                (*(_semafori)[l]).new_times[i] = verde;
                break;
            case 2:
                (*(_semafori)[l]).times[i] = giallo;
                (*(_semafori)[l]).new_times[i] = giallo;
                break;
            }
        }
    }

    (*(_semafori)[0]).lux_select = 0;
    (*(_semafori)[0]).new_lux_select = 0;
    (*(_semafori)[1]).lux_select = 1;
    (*(_semafori)[1]).new_lux_select = 1;
}

void SetReceivedTimes(ProtocolBytes _dataFromGateway, _Semafori _semafori)
{
    for (unsigned char i = 0; i < 3; i++)
    {
        unsigned char index = i * 5;
        unsigned char semaforoId = (_dataFromGateway[index] >> 1) & 0x0F;
        unsigned char colorId = ((_dataFromGateway[index] >> 5) & 0x03) - 1;

        (*(_semafori)[semaforoId]).new_times[colorId] = GetTime(index, _dataFromGateway);
    }
}

//CONTEGGIO VEICOLI
void Conteggio(unsigned int _count, unsigned char _motorcycle[4], unsigned char _car[4], unsigned char _truck[4], unsigned char index)
{
    if (_count >= 500)
    {
        _motorcycle[index] = _motorcycle[index] + 1;
    }
    if (_count >= 1500)
    {
        _car[index] = _car[index] + 1;
    }
    if (_count >= 3000)
    {
        _truck[index] = _truck[index] + 1;
    }
}