#include <FS.h>
#include <SPIFFS.h>

void setup() {
    Serial.begin(115200);
    if (!SPIFFS.begin(true)) {
        Serial.println("An error occurred while mounting SPIFFS");
        return;
    }

    // Escrever strings no mesmo arquivo
    appendToFile("/dados.txt", "Primeira string de exemplo.\n");
    appendToFile("/dados.txt", "Segunda string de exemplo.\n");

    float oi = 5;
    char variavel[100];
    sprintf(variavel, "Esta é uma string de exemplo para escrita no arquivo %.0f.\n", oi);

    appendToFile("/dados.txt", variavel);

    // Ler e imprimir o conteúdo do arquivo
    String fileContent = readFile("/dados.txt");
    Serial.println("Conteúdo do arquivo:");
    Serial.println(fileContent);
}

void loop() {
    // Nada a fazer no loop neste exemplo
}

void appendToFile(const char *filename, const char *data) {
    File file = SPIFFS.open(filename, "a");
    if (!file) {
        Serial.println("Failed to open file for appending");
        return;
    }
    file.print(data);
    file.close();
}

String readFile(const char *filename) {
    File file = SPIFFS.open(filename, "r");
    if (!file) {
        Serial.println("Failed to open file for reading");
        return "";
    }
    String fileContent = file.readString();
    Serial.println("");
    file.close();
    return fileContent;
}
