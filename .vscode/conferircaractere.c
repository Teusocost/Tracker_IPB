#include <stdio.h>

char mensagem[30] = "A-c23456B123456C123456";

int main()  
{   
    // Convertendo o ponteiro para um ponteiro para unsigned char
    unsigned char *msg = (unsigned char *)mensagem;
    // Tamanho da sua variável char ou char array em bytes

    size_t size = sizeof(mensagem);
    for(size_t i=0; i<size;i++)
    {
        printf("\nByte %zu: %c", i, msg[i]);
        if (msg[i] == 'A')
        {
            int cont = 1;
            for (cont;cont<7;cont++)
            {
                if(msg[i+cont] != '-' && msg[i+cont] != '0' && msg[i+cont] != '1' && msg[i+cont] != '2' && msg[i+cont] != '3' && msg[i+cont] != '4' && msg[i+cont] != '5' && msg[i+cont] != '6' && msg[i+cont] != '7' && msg[i+cont] != '8' && msg[i+cont] != '9')
                {
                    printf("\nErro encontrado na sequencia de  %c",msg[i]);
                    printf("\nErro = %c",msg[i+cont]);
                }
            }
        }
    }

return 0;
}
