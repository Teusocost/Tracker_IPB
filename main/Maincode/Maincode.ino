//---------------------------------------------------------
//biblitoecas e definições para o WIfi/MQTT
#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
//---------------------------------------------------------
//biblitoecas e definições para o GPS = UART
#include <TinyGPS++.h>
#include <HardwareSerial.h>
#define rxGPS 16
#define txGPS 17
//---------------------------------------------------------
//Bibliotecas e definições para sht21 - I2C
#include <Wire.h>
#include "sht21.h"
#define SDA_PIN 21  // Pino SDA (conexão com o SHT21)
#define SCL_PIN 22 // Pino SCL (conexão com o SHT21)
//---------------------------------------------------------
// Acelerometro - I2C
#include <Adafruit_Sensor.h> 
#include <Adafruit_ADXL345_U.h>
//---------------------------------------------------------
// Leitura de status da bateria
#include "batterystatus.h"
//=========================================================
//Varíaveis
//---------------------------------------------------------
// Configurações da rede Wi-Fi
const char* ssid = "NOS-2E40";
const char* password = "2TJA5RZ9";

//const char* ssid = "iPhone de Mateus";
//const char* password = "12345678";

//const char* ssid = "agents";
//const char* password = "QgC9O8VucAByqvVu5Rruv1zdpqM66cd23KG4ElV7vZiJND580bzYvaHqz5k07G2";
//---------------------------------------------------------
// Configurações do broker MQTT
const char* mqttServer = "broker.mqtt-dashboard.com";
const int mqttPort = 1883;
const char* mqttUser = "USUARIO_DO_BROKER";
const char* mqttPassword = "SENHA_DO_BROKER";

WiFiClient espClient;
PubSubClient client(espClient);  

char LED_BUILTIN_MQTT_SEND = 2;
//---------------------------------------------------------
//variávies para GPS
double lat = 0, lon = 0;
int sat= 0,vel = 0;
HardwareSerial gpsSerial(1);
TinyGPSPlus gps;
//---------------------------------------------------------
//variávies para sht21
float temperature = 0, humidity = 0;
//---------------------------------------------------------
//Variáveis para acelerômetro
Adafruit_ADXL345_Unified accel = Adafruit_ADXL345_Unified();
//---------------------------------------------------------
//variávies para leitura da bateria
float Voltage; 
int Percentage;
//---------------------------------------------------------
// Variáveis para entrar em deep sleep mode
//fator de conversão de microsegundos para segundos
#define uS_TO_S_FACTOR 1000000
//tempo que o ESP32 ficará em modo sleep (em segundos)
#define TIME_TO_SLEEP 4.3
//
void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Conectando-se a ");
  Serial.println(ssid);
  
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  
  Serial.println("");
  Serial.println("Conectado à rede Wi-Fi");
  Serial.println("Endereço IP: " + WiFi.localIP().toString());
}
/*
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.println("Mensagem recebida do tópico: " + String(topic));

  // Lidar com a mensagem recebida, se necessário
}
*/

void reconnect() {
  while (!client.connected()) {
    Serial.print("Conectando ao broker MQTT...");

    if (client.connect("ESP32Client", mqttUser, mqttPassword)) {
      Serial.println("Conectado");
      //client.subscribe("TOPICO_SUBSCRIBER");
    } else {
      Serial.print("Falha na conexão - Estado: ");
      Serial.print(client.state());
      Serial.println(" Tentando novamente em 5 segundos");
      delay(5000);
    }
  }
}

void setup() {

  //definições placa
  Serial.begin(115200);
  //------------------------------------
  //definições WIFI/MQTT
  setup_wifi();
  client.setServer(mqttServer, mqttPort);
  // client.setCallback(callback);
  //------------------------------------
  //gps definições
  gpsSerial.begin(9600, SERIAL_8N1, rxGPS, txGPS); // connect gps sensor
  //------------------------------------
  //sht21 definições
  Wire.begin(SDA_PIN, SCL_PIN);  // Inicializa a comunicação I2C

  //------------------------------------
  //pinos de leds
  pinMode(LED_BUILTIN_MQTT_SEND, OUTPUT);
  //------------------------------------
  //Acelerômetro
     if(!accel.begin())
   {
      Serial.println("No ADXL345 sensor detected.");
      while(1);
   }
}

void loop() {
  esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);
  Serial.println("=======================");
  //------------------------------------
  //status da bateria
  batterystatus(Voltage,Percentage);
  //------------------------------------
  //imprimir nível da conexão com Wifi
  int32_t rssi = WiFi.RSSI();
  Serial.print("Nível de sinal Wi-Fi: ");
  Serial.print(rssi);
  Serial.println(" dBm");
  //------------------------------------
  //leitura de temperatura e humidade SHT21
  readSHT21Data(temperature, humidity);  // Chama a função para ler os dados do sensor SHT21
//------------------------------------
  sensors_event_t event; 
  accel.getEvent(&event);
  //------------------------------------
  // Construir o objeto JSON
  DynamicJsonDocument doc(256); // Tamanho do buffer JSON
  //------------------------------------
  // Adicionar variáveis ao objeto JSON
  doc["latitude"] = lat,6;
  doc["longitude"] = lon,6;
  doc["sat"] = sat;
  doc["vel"] = vel;
  doc["temperatura"] = temperature,2;
  doc["umidade"] = humidity,2;
  doc["X"] = event.acceleration.x;
  doc["Y"] = event.acceleration.y;
  doc["Z"] = event.acceleration.z;
  doc["Bat_Volt"] = Voltage;
  doc["Bat_Perc"] = Percentage;
  // Serializar o objeto JSON em uma string
  String jsonData;
  serializeJson(doc, jsonData);
  //------------------------------------
  //conectando ao wifi
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  //------------------------------------
    // Publicar no tópico especificado
    if (client.publish("IPB/TESTE/TRACKER/01", jsonData.c_str())) {
    Serial.println("Message published successfully");
    digitalWrite(LED_BUILTIN_MQTT_SEND, HIGH);
    delay(200);
    digitalWrite(LED_BUILTIN_MQTT_SEND, LOW);
  } else {
    Serial.println("Failed to publish message");
    delay(200);
  }
  /*
  client.publish("IPB/TESTE/TRACKER", jsonData.c_str());
  Serial.println(jsonData.c_str());
  Serial.println("arquivo enviado");
  */
  //------------------------------------
    //Capturando dados GPS
  while (gpsSerial.available())
  {
    if (gps.encode(gpsSerial.read()))   // encode gps data
    {
      gps.encode(gpsSerial.read()); //processar dados brutos
      //-----------------------------------
      //atribuição e plot de resultados de gps
      sat = gps.satellites.value();
      lat = gps.location.lat();
      lon = gps.location.lng();
      vel = gps.speed.mps();
      break;
      //-----------------------------------
    }
  }
  esp_deep_sleep_start(); //força o ESP32 entrar em modo SLEEP
}
