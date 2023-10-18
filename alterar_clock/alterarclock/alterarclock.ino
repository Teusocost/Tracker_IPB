#include <Arduino.h>
uint32_t current_freq;
void setup() {
  Serial.begin(115200);
  delay(1000); // Aguarde um momento para estabilização da comunicação serial
  Serial.println("Iniciando com a frequência padrão da CPU.");
}

void loop() {
  // Obtenha e imprima a frequência atual da CPU em MHz
  //current_freq = getCpuFrequencyMhz();
  Serial.print("Frequencia da CPU atual: ");
  //Serial.print(current_freq);
  Serial.println(" MHz");

  // Altere a frequência da CPU para 80 MHz
  setCpuFrequencyMhz(20);
  delay(10);
  Serial.println("Alterando a frequência da CPU para 80 MHz.");

  //current_freq = getCpuFrequencyMhz();
  Serial.print("Frequencia da CPU atual: ");
  //Serial.print(current_freq);
  Serial.println(" MHz");

  delay(5000); // Aguarde 5 segundos

  // Restaure a frequência da CPU para 240 MHz
  setCpuFrequencyMhz(80);
  delay(10);
  Serial.println("Restaurando a frequência da CPU para 240 MHz.");
  Serial.println("HELO");

  delay(5000); // Aguarde mais 5 segundos
}
