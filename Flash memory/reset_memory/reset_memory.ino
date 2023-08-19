#include <FS.h>
#include <SPIFFS.h>

void setup() {
    Serial.begin(115200);
    if (!SPIFFS.begin(true)) {
        Serial.println("An error occurred while mounting SPIFFS");
        return;
    }

    Serial.println("Formatando SPIFFS...");
    SPIFFS.format(); // Apaga todo o conteúdo do SPIFFS
    Serial.println("SPIFFS formatado com sucesso!");
}

void loop() {
    // Nada a fazer no loop neste exemplo
}
