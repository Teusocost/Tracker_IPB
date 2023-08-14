// biblitoecas e variáveis para WIfi/MQTT
#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

// Configurações da rede Wi-Fi
const char *ssid = "NOS-2E40";
const char *password = "2TJA5RZ9";
WiFiClient espClient;
// const char* ssid = "iPhone de Mateus";
// const char* password = "12345678";

// const char* ssid = "agents";
// const char* password = "QgC9O8VucAByqvVu5Rruv1zdpqM66cd23KG4ElV7vZiJND580bzYvaHqz5k07G2";

// Configurações do broker MQTT
const char *mqttServer = "broker.mqtt-dashboard.com";
const int mqttPort = 1883;
const char *mqttUser = "USUARIO_DO_BROKER";
const char *mqttPassword = "SENHA_DO_BROKER";
PubSubClient client(espClient);
//---------------------------------------------------------
// Lora - Uart
#include <HardwareSerial.h>

#define rxLoRa 16
#define txLoRA 17

HardwareSerial lora(1); // objeto Lora
String incomingString;  // string que vai receber as informações

char markers[] = "ABCDEFGHIJ"; // J indica o fim da string
char *data; // para armazenar as informações que chegam
char extractedStrings[9][15]; // 9 caracteres de A a I e tamanho suficiente para armazenar os valores
void zerar_extractedStrings(); //para zerar a matriz

void zerar_extractedStrings(){ //função para zerar string que armazena os dados
    for (int i = 0; i < 9; ++i) {
        for (int j = 0; j < 15; ++j) {
            extractedStrings[i][j] = '\0';
        }
    }
}

bool serialEnabled = true; // Variável de controle para a comunicação serial

void toggleSerial(bool enable)
{
  if (enable)
  {
    lora.begin(115200, SERIAL_8N1, rxLoRa, txLoRA);
  }
  else
  {
    lora.end();
  }
}


void setup_wifi()
{
  delay(10);
  Serial.println();
  Serial.print("Conectando-se a ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("Conectado à rede Wi-Fi");
  Serial.println("Endereço IP: " + WiFi.localIP().toString());
}

void reconnect()
{
  while (!client.connected())
  { 
    client.setServer(mqttServer, mqttPort);
    Serial.print("Conectando ao broker MQTT...");

    //if (client.connect("ESP32Client", mqttUser, mqttPassword))
    if (client.connect("ESP32Client")){
      Serial.println("Conectado");
      // client.subscribe("TOPICO_SUBSCRIBER");
    }
    else
    {
      Serial.print("Falha na conexão - Estado: ");
      Serial.print(client.state());
      Serial.println(" Tentando novamente em 5 segundos");
      delay(5000);
    }
  }
}

void setup()
{
  Serial.begin(115200);                           // connect serial
  lora.begin(115200, SERIAL_8N1, rxLoRa, txLoRA); // connect lora  modulo
  //------------------------------------
  // definições WIFI/MQTT
  setup_wifi();
  client.setServer(mqttServer, mqttPort);
}

void loop(){
  //Serial.println("Aguardando mensagem LoRa");
  if (lora.available()){
    Serial.println("======================="); //debug serial
    incomingString = lora.readString();
    Serial.println(incomingString);
    //----------------------------------------------
    toggleSerial(false); // Desliga a comunicação serial para não haver interrupções indesejadas
    //----------------------------------------------

    char dataArray[70];
    incomingString.toCharArray(dataArray, 70);
    data = strtok(dataArray, ",");
    data = strtok(NULL, ",");
    data = strtok(NULL, ",");
    // Serial.println(incomingString);
    Serial.println(data);
  
    //----------------------------------------------
    // Dividir data em pacotes
    int i, j = 0, startPos = -1, count = 0;

    for (i = 0; data[i] != '\0'; i++){
      if (strchr(markers, data[i]))
      {
        { // se input contem markers
          if (startPos != -1)
            extractedStrings[count][j] = '\0';
          count++;
          j = 0;
        }
        extractedStrings[count][j++] = data[i];
        startPos = i;
      }
      else if (startPos != -1)
      {
        extractedStrings[count][j++] = data[i];
      }
    }

    for (i = 0; i < count; i++){
      printf("dado %c: %s\n", extractedStrings[i + 1][0], extractedStrings[i + 1] + 1);
    }
    //------------------------------------------------------
    DynamicJsonDocument doc(256); // Tamanho do buffer JSON
    doc["latitude"] = extractedStrings[1] + 1;    // -- A
    doc["longitude"] = extractedStrings[2] + 1;   // -- B
    doc["vel"] = extractedStrings[3] + 1;         // -- C
    doc["temperatura"] = extractedStrings[4] + 1; // -- D
    doc["umidade"] = extractedStrings[5] + 1;     // -- E
    doc["X"] = extractedStrings[6] + 1;           // -- F
    doc["Y"] = extractedStrings[7] + 1;           // -- G
    doc["Z"] = extractedStrings[8] + 1;           // -- H
    doc["Bat_Perc"] = extractedStrings[9] + 1;    // -- I
    // Serializar o objeto JSON em uma string
    String jsonData;
    serializeJson(doc, jsonData);
    //-------------------------
    // confere conexão
    if (!client.connected()){
      reconnect();
    }
    client.loop();
    //------------------------------------
    // imprimir nível da conexão com Wifi
    int32_t rssi = WiFi.RSSI();
    Serial.print("Nível de sinal Wi-Fi: ");
    Serial.print(rssi);
    Serial.println(" dBm");

    //-------------------------------------------
    // Publicar no tópico especificado
    if (client.publish("IPB/TESTE/TRACKER/01", jsonData.c_str())){ // encaminha json montado!
      Serial.println("Message published successfully");
      delay(200);
    }
    else{
      Serial.println("Failed to publish message");
      delay(200);
    }
    //-------------------------------------------
    void zerar_extractedStrings(); //para zerar a matriz de dados
    Serial.println("string zerada");
    //-------------------------------------------
    toggleSerial(true); // Liga a comunicação serial novamente
    Serial.println("Serial ligada");
    //-------------------------------------------
  }
}