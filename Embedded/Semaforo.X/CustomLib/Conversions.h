int map(int x, int in_min, int in_max, int out_min, int out_max) //Mappare nuovamente un numero da un intervallo a un altro
{
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

double pow(double b, double e) //Funzioneper fare la potenza
{
    int i;
    double number = 1;
    for (i = 0; i < e; i++)
    {
        number *= b;
    }
    return (number);
}

void intToString(int valore, char *str) //funzione per convertire un intero in una stringa
{
    int i;
    int num = 0;
    for (i = 0; i < 3; i++)
    {
        str[2 - i] = '0' + ((valore % (char)pow(10, 1 + i)) / (char)pow(10, i));
    }
    str[3] = '\0';
}

void GetDigits(unsigned char *centinaia, unsigned char *decine, unsigned char *unita, int Time)
{
    while (Time / 1000 > 0)
    {
        Time = Time / 10;
    }

    *centinaia = Time / 100;     //Il tempo totale vine scomposto nelle varie parti per essere poi riportato nei display 7 segmenti (le centinaia)
    *decine = (Time % 100) / 10; //Il tempo totale vine scomposto nelle varie parti per essere poi riportato nei display 7 segmenti (le decine)
    *unita = (Time % 100) % 10;  //Il tempo totale vine scomposto nelle varie parti per essere poi riportato nei display 7 segmenti (le unita)
}