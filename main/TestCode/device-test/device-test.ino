//---------------------------------------------------------
// biblitoecas e variáveis para o GPS = UART
#include <TinyGPS++.h>
#include <HardwareSerial.h>
#define rxGPS 16
#define txGPS 17

// variávies para GPS
double lat = 0, lon = 0;
int sat = 0, vel = 0;
HardwareSerial gpsSerial(2);
TinyGPSPlus gps;
//---------------------------------------------------------
// biblitoecas e definições para o Rylr 998 (LoRa) = UART
#define rxLORA 25
#define txLORA 26
HardwareSerial lora(1);
String incomingString;
char LED_BUILTIN_MQTT_SEND = 2; //pisca led quando envia LoRa
char end_to_send = '2';   // endereço do lora que vai receber esse pacote
//---------------------------------------------------------
// Bibliotecas e definições para sht21 - I2C
#include <Wire.h>
#include "sht21.h"
#define SDA_PIN 21 // Pino SDA (conexão com o SHT21)
#define SCL_PIN 22 // Pino SCL (conexão com o SHT21)

// variávies para sht21
float temperature = 0, humidity = 0;
//---------------------------------------------------------
// Acelerometro - I2C
#include <Adafruit_Sensor.h>
#include <Adafruit_ADXL345_U.h>

// Variáveis para acelerômetro
Adafruit_ADXL345_Unified accel = Adafruit_ADXL345_Unified();
//---------------------------------------------------------
// Leitura de status da bateria
#include "batterystatus.h"

// variávies para leitura da bateria
float Voltage;
int Percentage;

//---------------------------------------------------------
// Variáveis para entrar em deep sleep mode
// fator de conversão de microsegundos para segundos
#define uS_TO_S_FACTOR 1000000
// tempo que o ESP32 ficará em modo sleep (em segundos)
#define TIME_TO_SLEEP 4.3

//---------------------------------------------------------
//---------------------------------------------------------
//funções instanciadas antes que o sistema passe a funcionar

void setup()
{
  // definições placa
  Serial.begin(115200);
  //------------------------------------
  // gps definições
  gpsSerial.begin(9600, SERIAL_8N1, rxGPS, txGPS); // connect gps sensor
  //------------------------------------
  // sht21 definições
  Wire.begin(SDA_PIN, SCL_PIN); // Inicializa a comunicação I2C
  //------------------------------------
  // pinos de leds
  pinMode(LED_BUILTIN_MQTT_SEND, OUTPUT);
  //------------------------------------
  // Acelerômetro
  if (!accel.begin()){
    Serial.println("No ADXL345 sensor detected.");
    while (1)
      ;
  }
  //------------------------------------
  // LoRa
  lora.begin(115200, SERIAL_8N1, rxLORA, txLORA); // connect gps sensor
}

void loop(){
  esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR); //saí do modo sleep mode quando time * factor
  Serial.println("======================="); //debug serial.print
  //------------------------------------
  // status da bateria (funççao externa)
  batterystatus(Voltage, Percentage);
  //------------------------------------
  // leitura de temperatura e humidade SHT21
  readSHT21Data(temperature, humidity); // Chama a função para ler os dados do sensor SHT21       
  sensors_event_t event;
  accel.getEvent(&event);
  //------------------------------------
  // Capturando dados GPS
  while (gpsSerial.available())
  {
    if (gps.encode(gpsSerial.read())) // encode gps data
    {
      gps.encode(gpsSerial.read()); // processar dados brutos
      //-----------------------------------
      // atribuição e plot de resultados de gps
      sat = gps.satellites.value(); //número de satélites
      lat = gps.location.lat(); //latitude 
      lon = gps.location.lng(); // longitude
      vel = gps.speed.mps(); //velocidade
      break;
      //-----------------------------------
    }
  }
  //-----------------------------------
  //organizar e enviar LoRa
    char mensagem[120]; //mensagem completa
    char data[70]; //apenas variáveis
    sprintf(data, "A%.6fB%.6fC%iD%.2fE%.2fF%.2fG%.2fH%3.2fI%.0fJ",lat, lon, vel, temperature, humidity, event.acceleration.x, event.acceleration.y, event.acceleration.z, Percentage); //atribui e organiza as informações em data
    //o caractere J indica o fim da mensagem
    int requiredBufferSize = snprintf(NULL, 0, "%s",data); //calcula tamanho string
    sprintf(mensagem, "AT+SEND=%c,%i,%s",end_to_send,requiredBufferSize,data); // junta as informações em "mensagem"
    lora.println(mensagem); //manda a mensagem montada para o módulo
    Serial.println(mensagem); //imprime no monitor a mensagem montada 
    Serial.println(lora.readString()); //lê a resposta do módulo
    //-----------------------------------
    //led para indicar envio
    digitalWrite(LED_BUILTIN_MQTT_SEND, HIGH);
    delay(200);
    digitalWrite(LED_BUILTIN_MQTT_SEND, LOW);
  // esp_deep_sleep_start(); //força o ESP32 entrar em modo SLEEP
}

