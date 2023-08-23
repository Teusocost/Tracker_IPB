#ifndef SPIFFS_UTILS_H
#define SPIFFS_UTILS_H

#include <Arduino.h>
#include <FS.h>
#include <SPIFFS.h>
#include <vector>

class SPIFFS_Utils {
public:
    SPIFFS_Utils();
    void appendToFile(const char *filename, const char *data);
    String readLastValue(const char *filename);
    void removeLastValue(const char *filename);
    void listFiles();
    void format();
    void bytes();

private:
    String readAllValuesExceptLast(const char *filename);
    void replaceFile(const char *filename, const String &newContent);
    String readFile(const char *filename);
};

#endif
