#include <Arduino.h>
#include <Wire.h>
#include "sht21.h"

#define SDA_PIN 21  // Pino SDA (conexão com o SHT21)
#define SCL_PIN 22  // Pino SCL (conexão com o SHT21)

void readSHT21Data(float& temperature, float& humidity) {
  // Inicia uma nova medição de temperatura e umidade
  Wire.beginTransmission(0x40);  // Endereço do sensor SHT21
  Wire.write(0xF3);  // Comando para medição de temperatura
  Wire.endTransmission();

  delay(500);  // Aguarda a conclusão da medição

  // Lê os dados da temperatura
  Wire.requestFrom(0x40, 3);  // Endereço do sensor SHT21 e quantidade de bytes a serem lidos
  if (Wire.available() == 3) {
    byte msb = Wire.read();
    byte lsb = Wire.read();
    byte crc = Wire.read();

    temperature = ((msb << 8) | lsb);
    temperature *= 175.72;
    temperature /= 65536;
    temperature -= 46.85;
  }

  // Inicia uma nova medição de umidade
  Wire.beginTransmission(0x40);  // Endereço do sensor SHT21
  Wire.write(0xF5);  // Comando para medição de umidade
  Wire.endTransmission();

  delay(500);  // Aguarda a conclusão da medição

  // Lê os dados da umidade
  Wire.requestFrom(0x40, 3);  // Endereço do sensor SHT21 e quantidade de bytes a serem lidos
  if (Wire.available() == 3) {
    byte msb = Wire.read();
    byte lsb = Wire.read();
    byte crc = Wire.read();

    humidity = ((msb << 8) | lsb);
    humidity *= 125;
    humidity /= 65536;
    humidity -= 6;
  }
}
