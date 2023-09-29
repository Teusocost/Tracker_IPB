#include <FS.h>
#include <SPIFFS.h>

void setup() {
    Serial.begin(115200);
    if (!SPIFFS.begin(true)) {
        Serial.println("An error occurred while mounting SPIFFS");
        return;
    }

    listFiles();
}

void loop() {
    // Nada a fazer no loop neste exemplo
}

void listFiles() {
    File root = SPIFFS.open("/");
    File file = root.openNextFile();
    
    if (!file) {
        Serial.println("Nenhum arquivo encontrado na memória Flash (SPIFFS)");
        return;
    }

    Serial.println("Arquivos encontrados na memória Flash (SPIFFS):");
    while (file) {
        Serial.print("Nome: ");
        Serial.println(file.name());
        Serial.println("Conteúdo:");
        while (file.available()) {
            Serial.write(file.read());
        }
        Serial.println("---");
        file = root.openNextFile();
    }
    root.close();
}
