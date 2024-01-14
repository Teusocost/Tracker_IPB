#include <Wire.h>
#include "sht21.h"

#define SDA_PIN 21  // Pino SDA (conexão com o SHT21)
#define SCL_PIN 22 // Pino SCL (conexão com o SHT21)

void setup() {
  pinMode(2,OUTPUT);
  pinMode(18,OUTPUT);
  digitalWrite(18,HIGH);
  Serial.begin(115200);

  Wire.begin(SDA_PIN, SCL_PIN);  // Inicializa a comunicação I2C
}

void loop() {
  float temperature, humidity;

  readSHT21Data(temperature, humidity);  // Chama a função para ler os dados do sensor SHT21

  // Exibe as leituras no monitor serial
  Serial.print("Temperature: ");
  Serial.print(temperature, 2);
  Serial.println(" °C");

  Serial.print("Humidity: ");
  Serial.print(humidity, 2);
  Serial.println(" %");
  digitalWrite(2,!digitalRead(2));
  delay(1000);
}
