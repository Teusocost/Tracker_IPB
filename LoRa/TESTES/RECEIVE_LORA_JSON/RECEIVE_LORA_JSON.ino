#include <ArduinoJson.h>
#include <HardwareSerial.h>
#include <WiFi.h>
#include <PubSubClient.h>

#define rxLoRa 16
#define txLoRA 17

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
  lora.begin(115200, SERIAL_8N1, rxLoRa, txLoRA); // connect lora  modulo
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
    //----------------------------------------------
    // Dividir data em pacotes 
    char markers[] = "ABCDEFGHIJ";
    char extractedStrings[9][15]; // 9 caracteres de A a I e tamanho suficiente para armazenar os valores
                                  //J indica o fim da string

    int i, j = 0, startPos = -1, count = 0;

    for (i = 0; data[i] != '\0'; i++) {
        if (strchr(markers, data[i])) { { // se input contem markers
            if (startPos != -1) 
                extractedStrings[count][j] = '\0';
                count++;
                j = 0;
            }
            extractedStrings[count][j++] = data[i];
            startPos = i;
        } else if (startPos != -1) {
            extractedStrings[count][j++] = data[i];
        }
    }

    for (i = 0; i < count; i++) {
        printf("Marker %c: %s\n", extractedStrings[i+1][0], extractedStrings[i+1] + 1);
    }
    //------------------------------------------------------
    DynamicJsonDocument doc(1024); // Tamanho do buffer JSON
    doc["latitude"] = extractedStrings[1]+1;
    doc["longitude"] = extractedStrings[2]+1;
    doc["vel"] = extractedStrings[3]+1;
    doc["temperatura"] = extractedStrings[4]+1;
    doc["umidade"] = extractedStrings[5]+1;
    doc["X"] = extractedStrings[6]+1;
    doc["Y"] = extractedStrings[7]+1;
    doc["Z"] = extractedStrings[8]+1;
    doc["Bat_Perc"] = extractedStrings[9]+1;
    // Serializar o objeto JSON em uma string
    String jsonData;
    serializeJson(doc, jsonData);
    //-------------------------
    //confere conexão
    if (!client.connected()) {
    reconnect();
    }
    client.loop();

  //-------------------------------------------
    // Publicar no tópico especificado
    if (client.publish("IPB/TESTE/TRACKER/01", jsonData.c_str())) //encaminha json montado!
    {
        Serial.println("Message published successfully");
        delay(2000);
    } else 
    {
        Serial.println("Failed to publish message");
        delay(200);
    }

}