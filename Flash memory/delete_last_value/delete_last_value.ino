#include <FS.h>
#include <SPIFFS.h>
#include <vector>

void setup()
{
    Serial.begin(115200);
    if (!SPIFFS.begin(true))
    {
        Serial.println("An error occurred while mounting SPIFFS");
        return;
    }

    // Ler e imprimir o conteúdo antes da remoção
    String originalContent = readFile("/dados.txt");
    Serial.println("Conteúdo original:");
    Serial.println(originalContent);

    // Remover o último valor
    removeLastValue();

    // Ler e imprimir o conteúdo após a remoção
    String updatedContent = readFile("/dados.txt");
    Serial.println("Conteúdo após a remoção do último valor:");
    Serial.println(updatedContent);
}

void loop()
{
    // Nada a fazer no loop neste exemplo
}

void removeLastValue()
{
    std::vector<String> lines;

    File file = SPIFFS.open("/dados.txt", "r");
    if (!file)
    {
        Serial.println("Failed to open file for reading");
        return;
    }

    while (file.available())
    {
        String line = file.readStringUntil('\n');
        if (line.length() > 0)
        {
            lines.push_back(line);
        }
    }

    file.close();

    File newFile = SPIFFS.open("/dados.txt", "w");
    if (!newFile)
    {
        Serial.println("Failed to open file for writing");
        return;
    }

    for (size_t i = 0; i < lines.size() - 1; i++)
    {
        newFile.println(lines[i]);
    }

    newFile.close();
}

String readFile(const char *filename)
{
    File file = SPIFFS.open(filename, "r");
    if (!file)
    {
        Serial.println("Failed to open file for reading");
        return "";
    }
    String fileContent = file.readString();
    file.close();
    return fileContent;
}
