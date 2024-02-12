#include <EEPROM.h>

// Defina o endereço na EEPROM
int address = 0;
static int newValue;
void setup() {
  Serial.begin(115200);

  // Inicialize a EEPROM com o tamanho desejado (por exemplo, 1 byte)
  EEPROM.begin(1);

  // Leitura do valor atual na EEPROM
  int storedValue = EEPROM.read(address);
  Serial.print("Valor atual na EEPROM: ");
  Serial.println(storedValue);
}

void loop() {
  // Simulando leitura de um valor (pode ser 0 ou 1)
   newValue = !newValue;
  // Se o valor lido for diferente do armazenado, atualize na EEPROM
  if (newValue != EEPROM.read(address)) {
    EEPROM.write(address, newValue);
    EEPROM.commit();
    Serial.print("Valor atualizado na EEPROM: ");
    Serial.println(newValue);
  }

  // Aguarde um pouco antes de verificar novamente
  delay(1000);
}
