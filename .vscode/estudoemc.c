#include <stdio.h>

int main()
{

    float valor_float = 3.14159;
    float humidade = 4.2343;
    float temperatura = 42.5654;
    char valor_char[2]; // Tamanho suficiente para armazenar a string
    // Usando sprintf() para converter o float em uma string
    sprintf(valor_char, "%.2f", valor_float);

    // Imprimindo a string resultante
    printf("Valor em formato de string: %s\n", valor_char);
    printf(sizeof(valor_char));
    char mensagem[40];
    sprintf(mensagem, "Humidade: %.2f%%, Temperatura: %.2f°C", humidade, temperatura);
    printf(mensagem);
    int tam = sizeof(mensagem);
    printf("%c",tam);
    return 0;
}
