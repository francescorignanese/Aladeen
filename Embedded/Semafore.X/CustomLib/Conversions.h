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