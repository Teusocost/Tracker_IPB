#include "SPIFFS_Utils.h"

SPIFFS_Utils::SPIFFS_Utils() {
  // Inicalização
  if (!SPIFFS.begin(true)) {
    Serial.println("An error occurred while mounting SPIFFS");
    return;
  }
}

void SPIFFS_Utils::appendToFile(const char *filename, const char *data) {
    File file = SPIFFS.open(filename, "a");
    if (!file) {
        Serial.println("Failed to open file for appending");
        return;
    }
    file.print(data);
    file.close();
}

String SPIFFS_Utils::readLastValue(const char *filename) {
    String lastValue = "";

    File file = SPIFFS.open(filename, "r");
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

void SPIFFS_Utils::removeLastValue(const char *filename) {
    std::vector<String> lines;

    File file = SPIFFS.open(filename, "r");
    if (!file) {
        Serial.println("Failed to open file for reading");
        return;
    }

    while (file.available()) {
        String line = file.readStringUntil('\n');
        if (line.length() > 0) {
            lines.push_back(line);
        }
    }

    file.close();

    File newFile = SPIFFS.open(filename, "w");
    if (!newFile) {
        Serial.println("Failed to open file for writing");
        return;
    }

    for (size_t i = 0; i < lines.size() - 1; i++) {
        newFile.println(lines[i]);
    }

    newFile.close();
}

void SPIFFS_Utils::listFiles() {
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

void SPIFFS_Utils::format(){
Serial.println("Formatando SPIFFS...");
SPIFFS.format(); // Apaga todo o conteúdo do SPIFFS
Serial.println("SPIFFS formatado com sucesso!");
}

void SPIFFS_Utils::bytes(){
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

