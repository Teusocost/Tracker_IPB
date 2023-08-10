#include <stdio.h>
#include <string.h>

int main() {
    char inputString[] = "A-41.123456B6.123456C2D42.22E53.44F-10.02G-10.04H8.41I80";
    char markers[] = "ABCDEFGHI";
    char extractedStrings[9][15]; // 9 caracteres de A a I e tamanho suficiente para armazenar os valores

    int i, j = 0, startPos = -1, count = 0;

    for (i = 0; inputString[i] != '\0'; i++) {
        if (strchr(markers, inputString[i])) { { // se input contem markers
            if (startPos != -1) 
                extractedStrings[count][j] = '\0';
                count++;
                j = 0;
            }
            extractedStrings[count][j++] = inputString[i];
            startPos = i;
        } else if (startPos != -1) {
            extractedStrings[count][j++] = inputString[i];
        }
    }

    for (i = 0; i < count; i++) {
        printf("Marker %c: %s\n", extractedStrings[i][0], extractedStrings[i] + 1);
    }

    return 0;
}
