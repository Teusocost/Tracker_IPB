#include <FS.h>
#include <SPIFFS.h>

void setup() {
    Serial.begin(115200);
    if (!SPIFFS.begin(true)) {
        Serial.println("An error occurred while mounting SPIFFS");
        return;
    }

    // Obtém informações sobre a memória disponível
    uint32_t totalBytes = SPIFFS.totalBytes();
    uint32_t usedBytes = SPIFFS.usedBytes();
    uint32_t freeBytes = totalBytes - usedBytes;

    Serial.print("Tamanho total da memória SPIFFS: ");
    Serial.print(totalBytes);
    Serial.println(" bytes");
    Serial.print("Espaço usado na memória SPIFFS: ");
    Serial.print(usedBytes);
    Serial.println(" bytes");
    Serial.print("Espaço livre disponível na memória SPIFFS: ");
    Serial.print(freeBytes);
    Serial.println(" bytes");
}

void loop() {
    // Nada a fazer no loop neste exemplo
}
