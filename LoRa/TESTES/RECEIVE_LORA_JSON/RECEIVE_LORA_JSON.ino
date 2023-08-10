#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <HardwareSerial.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

#define rxGPS 25
#define txGPS 26

HardwareSerial lora(1);
String incomingString;
//---------------------------------------------------------
// Configurações da rede Wi-Fi
const char* ssid = "NOS-2E40";
const char* password = "2TJA5RZ9";

//---------------------------------------------------------
// Configurações do broker MQTT
const char* mqttServer = "broker.mqtt-dashboard.com";
const int mqttPort = 1883;
const char* mqttUser = "USUARIO_DO_BROKER";
const char* mqttPassword = "SENHA_DO_BROKER";

WiFiClient espClient;
PubSubClient client(espClient);
//--------data
char *data;

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

void setup()
{
  Serial.begin(115200); // connect serial
  lora.begin(115200, SERIAL_8N1, rxGPS, txGPS); // connect gps sensor
  //------------------------------------
  // definições WIFI/MQTT
  setup_wifi();
  client.setServer(mqttServer, mqttPort);
}

void loop()
{
  if (lora.available()) 
    {

        incomingString = lora.readString();
        Serial.println(incomingString);
    
        char dataArray[70]; 
        incomingString.toCharArray(dataArray,70);
        data = strtok(dataArray, ",");
        data = strtok(NULL, ",");
        data = strtok(NULL, ",");
        
        //Serial.println(incomingString);

        Serial.println(data);
    
    }
    DynamicJsonDocument doc(1024); // Tamanho do buffer JSON
    float outro = 5.52;
    doc["outro"] = outro;
    doc["dado"] = data;
    // Serializar o objeto JSON em uma string
    String jsonData;
    serializeJson(doc, jsonData);
      if (!client.connected()) {
    reconnect();
    }
    client.loop();

  //------------------------------------
    // Publicar no tópico especificado
    if (client.publish("IPB/TESTE/TRACKER/01", jsonData.c_str())) 
    {
        Serial.println("Message published successfully");
        delay(2000);
    } else 
    {
        Serial.println("Failed to publish message");
        delay(200);
    }

}