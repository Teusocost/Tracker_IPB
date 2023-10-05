#include <stdio.h>
#include <string.h>

int main() {
    char minhaVariavel[100];
    
    // Criando a variável com o caractere indesejado
    strcpy(minhaVariavel, "Esta é uma mensagem com um caractere indesejado:  Aqui está o restante.\r");
    
    // Mostrando a mensagem original e seu tamanho
    printf("Mensagem original:\n%s\n", minhaVariavel);
    printf("Tamanho da mensagem original: %lu bytes\n", strlen(minhaVariavel));
    
    // Removendo o caractere indesejado ('\r')
    for (int i = 0; i < strlen(minhaVariavel); i++) {
        if (minhaVariavel[i] == '\r') {
            for (int j = i; j < strlen(minhaVariavel); j++) {
                minhaVariavel[j] = minhaVariavel[j + 1];
            }
        }
    }
    
    // Mostrando a mensagem filtrada e seu tamanho
    printf("\nMensagem filtrada:\n%s\n", minhaVariavel);
    printf("Tamanho da mensagem filtrada: %lu bytes\n", strlen(minhaVariavel));
    
    return 0;
}
