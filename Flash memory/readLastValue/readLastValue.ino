#include <FS.h>
#include <SPIFFS.h>

void setup() {
    Serial.begin(115200);
    if (!SPIFFS.begin(true)) {
        Serial.println("An error occurred while mounting SPIFFS");
        return;
    }

    // Ler e imprimir o último valor salvo
    String lastValue = readLastValue();
    Serial.println("Último valor gravado:");
    Serial.println(lastValue);
}

void loop() {
    // Nada a fazer no loop neste exemplo
}

String readLastValue() {
    String lastValue = "";

    File file = SPIFFS.open("/dados.txt", "r");
    if (!file) {
        Serial.println("Failed to open file for reading");
        return lastValue;
    }

    while (file.available()) {
        lastValue = file.readStringUntil('\n');
    }
    
    file.close();

    return lastValue;
}
