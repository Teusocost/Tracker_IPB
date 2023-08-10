#include <stdio.h>
#include <string.h>
char input[70] = "A-41.123456B6.123456C2D42.22E53.44F-10.02G-10.04H8.41I80";
char lat[11];
char lon[9];
char vel[2];
char temp[2];
char hum[2];
char x[4];
char y[4];
char z[4];
char bat[3];

int main()
{     

    memcpy(lat, input + 1, 10);
    // lat[] = '\0'; // Adiciona o caractere nulo no final da string
    memcpy(lon, input + 12, 8);
    // lon[] = '\0'; // Adiciona o caractere nulo no final da string

    printf("Parte 1: %s\n", lat);
    printf("Parte 2: %s\n\n", lon);
    return 0;
}