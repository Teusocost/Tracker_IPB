
// Define o pino GPIO que será usado para a interrupção
#include "EEPROM.h"
#define EEPROM_SIZE 1
const int pinInterrupt = 23; // Use o pino GPIO 2 como exemplo
// Função de tratamento da interrupção
void handleInterrupt() {
  EEPROM.write(0, 1);
  EEPROM.commit();
  //esp_restart();
}

void setup() {
  EEPROM.begin(EEPROM_SIZE);
  // Inicializa a comunicação serial
  Serial.begin(115200);
  // Configura o pino como entrada e habilita o resistor de pull-up interno
  pinMode(pinInterrupt, INPUT_PULLUP);

  // Configura a função de tratamento da interrupção para o pino especificado
  attachInterrupt(digitalPinToInterrupt(pinInterrupt), handleInterrupt, CHANGE);
}

void loop() {
  if(EEPROM.read(0) >= 1){
    Serial.println("HELLO");
    EEPROM.write(0, 0);
    EEPROM.commit();
    delay(100);
  }
  while(1){
  Serial.println(millis());
  delay(500);
  }
}
