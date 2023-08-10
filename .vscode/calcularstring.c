#include <stdio.h>

int main() {
    double lat = -41.123456;
    double lon = 6.123456;
    int vel = 2;
    float temp = 42.22;
    float hum = 53.44;
    float x = -10.02;
    float y = -9.04;
    float z = 8.41;
    float bat = 80;

    int requiredBufferSize = snprintf(NULL, 0, "A%.6fB%.6fC%iD%.2fE%.2fF%.2fG%.2fH%.2fI%.0f", lat, lon, vel, temp, hum, x, y, z, bat);

    // Adicione 1 para o caractere nulo de terminação da string
    requiredBufferSize++; 

    printf("Tamanho necessário do buffer: %d\n", requiredBufferSize);

    return 0;
}
