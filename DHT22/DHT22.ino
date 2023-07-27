
// Biblioteca Sensor de Temperatura
#include <Adafruit_Sensor.h>
#include "DHT.h"





/**PINOS DIGITAIS DE ACESSO AO -- ESP32 MASTER -- ********************************************************************************************************/
const int DHTPIN   =              4;                                 // Sensor DHT11 Pode ser usado as
                                                                     // seguintes portas 3, 4, 5, 12, 13 ou 14




/***INSTANCIANDO OBJETOS***********************************************************************************************************************************/

// Define Modelo do DHT
#define DHTTYPE    DHT22       // DHT 11
//#define DHTTYPE    DHT22     // DHT 22 
//#define DHTTYPE    DHT21     // DHT 21 


// Define Variáveis do DHT
DHT dht(DHTPIN, DHTTYPE);
float humidity;     float humidity_atual;
float temperature;  float temperature_atual;




/*** SETUP DO PROGRAMA ***********************************************************************************************************************************/
void setup() {
  Serial.begin(115200);
  dht.begin();

}

void loop() {

  // Leitura do DHT ******************************************************************
  humidity =    dht.readHumidity();
  temperature = dht.readTemperature();

  // Atuaização do DHT LCD ***********************************************************
  if (isnan(humidity) || isnan(temperature)) {
    Serial.println("Falha ao ler DHT!!");
  }
  else if (humidity_atual != humidity || temperature_atual != temperature) {
      humidity_atual = humidity;  temperature_atual = temperature;
      Serial.print("Temp.");  
      Serial.print(temperature);
      Serial.print("°");
      Serial.println("C");

      Serial.print("Umid.");
      Serial.print(humidity);
      Serial.println("%");
      Serial.println("====================");
    }
    
}
