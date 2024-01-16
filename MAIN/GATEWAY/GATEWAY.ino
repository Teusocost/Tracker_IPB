/* ============================================================================
 *
 *   Code to Gateway
 *   This code went developer to last work in IPB - BRAGANÇA 
 * 
 *   Autor: Mateus Costa de Araujo 
 *   Data:  Fevereiro, 2024
 *
============================================================================ */
// Lybrares
// #include <WiFi.h>
#include <Arduino.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <WiFiManager.h> 
#include "esp_task_wdt.h"
#include <HardwareSerial.h>  // LoRa
//==========================================================================
// Defines
#define rxLoRa 16
#define txLoRA 17
#define led_to_receive 25
#define pin_led 12
//---------------------------------------------------------
// Wifi and MQTT
WiFiClient espClient;
int32_t rssi; // RSSI wifi
const char *mqttServer = "broker.mqtt-dashboard.com";
bool flag_mqtt = false; // confirmation flag package.
const short mqttPort = 1883;
const char *mqttUser = "IPB";
const char *mqttPassword = "noiottracker";
const char *topic = "IPB/TESTE/TRACKER/01";
const char *topic2 = "IPB/TESTE/GATWAY/01";

PubSubClient client(espClient);
short time_to_resend_msg_status_gatway = 30000; // Time to resend gateway status (ms) (EDITABLE)
char *RSSI_LoRA = 0;

String jsonData = "";
String jsonStatus = "";       // (gateway status)
DynamicJsonDocument doc(256); // Package buffer
DynamicJsonDocument gat(64);  // gatway vuffer status
//---------------------------------------------------------
// Lora - Uart
HardwareSerial lora(1);
String incomingString;
char end_to_send = '1'; // Device Address
char markers[12]; // = "ABCDEFGHIJK";  // J -> end to string
char *data;                    // raw data
char extractedStrings[12][20]; // Vector to package
char utctime[25];              // vetor que vai receber time          ******
char latlon[2][10];
char last_data[] = "K";        // Character which packet is memory 
char find_gatway[] = "O";      // Character that indicates that the device is looking for the gateway
char type_data = 0;            //  [0] - current; [1] - memory
//---------------------------------------------------------
// FLOW
unsigned short break_line = 60000;        
unsigned short time_to_show_point = 1000; 
unsigned long time_break_line;          
unsigned long time_show_msg;            
//bool serialEnabled = true; // Variável de controle para a comunicação serial
//---------------------------------------------------------
// FREERTOS
static uint8_t taskCoreZero = 0;
static uint8_t taskCoreOne = 1;
//==========================================================================
// Functions
void PINS_AND_SYSTEM();
void LORA_SETTINGS();
void WIFI_SETTINGS();
void TASKS();
void zerar_extractedStrings(); 
void publishMQTTstatus (void *pcParameters);
void debug_led (void *pvParameters);
void processing(void *pvParameters);
void separate_lat_and_lon();
void led_to_send();
void zerar_extractedStrings();
void toggleSerial(bool enable);
void reconnect();
//==========================================================================
void setup(){
  PINS_AND_SYSTEM();                         
  LORA_SETTINGS();
  WIFI_SETTINGS();      
  TASKS();
} 
//==========================================================================
void loop(){} 

void TASKS(){
  xTaskCreatePinnedToCore(
        publishMQTTstatus,   /* função que implementa a tarefa */
        "coreTaskOne", /* task name */
        5000,          /* words */
        NULL,          /* entrance parameter */
        2,             /* Priority */
        NULL,          /* Reference */
        taskCoreZero);  /* Core */

    delay(500); // tempo para a tarefa iniciar
  //------------------------------------
    xTaskCreatePinnedToCore(
        debug_led,     
        "coreTaskTwo", 
        1000,          
        NULL,          
        1,             
        NULL,          
        taskCoreZero);  
    delay(500); 
  //------------------------------------
    xTaskCreatePinnedToCore(
        processing,   
        "coreTaskThree", 
        10000,          
        NULL,          
        3,             
        NULL,          
        taskCoreOne); 
    delay(500);   
}

void publishMQTTstatus (void *pcParameters){
  while(true){
      if (!client.connected()){
        reconnect();
      }
      client.loop();           // cliente em loop (MQTT)
      rssi = WiFi.RSSI();      // recebe Rssi WiFi
      gat["RSSI_WIFI"] = rssi; // atribui ao Json gat

      jsonStatus = "";                // Apaga informações antiga da string
      serializeJson(gat, jsonStatus); // Serializa o pacote json

      client.publish(topic2, jsonStatus.c_str());            // publica json no tópico especificado
      Serial.print("\nSTATUS GATWAY ENVIADO - RSSI WIFI: "); // mostra msg de envio
      Serial.println(rssi);                                  // valor do rssi
      vTaskDelay(time_to_resend_msg_status_gatway);          // taks dorme por tempo definido
  }
}

void debug_led (void *pvParameters){

  String taskMessage = "Task running on core ";
  taskMessage = taskMessage + xPortGetCoreID();

  int control = 0;
  while (true){
    if(WiFi.status() != WL_CONNECTED){
      digitalWrite(pin_led, HIGH);
      vTaskDelay(500);
      digitalWrite(pin_led, LOW);
    }
    else{
      digitalWrite(pin_led, LOW);
    }
    vTaskDelay(1000);
  }
}

void processing(void *pvParameters){
  String taskMessage = "Task running on core ";
  taskMessage = taskMessage + xPortGetCoreID();
  while (true){
    //----------------------------------------------
    //reseta whatchdogs para indicar que o sistema está funcionando normalmente
    //esp_task_wdt_reset();
    //----------------------------------------------
    // mostra "." para indicar que está aguardando pacote LORA
    if (millis() - time_break_line >= break_line){
      // Reinicie o contador millis()
      Serial.print("\n"); 
      time_break_line = millis();
    }
    if (millis() - time_show_msg >= time_to_show_point){
      Serial.print(".");
      time_show_msg = millis();
    }
    if (lora.available()){
      Serial.println("\n========[PACOTE CHEGOU]=========="); // debug serial
      incomingString = lora.readString();
      Serial.println(incomingString);
      //----------------------------------------------
      toggleSerial(false); // Desliga a comunicação serial para não haver interrupções indesejadas
      //----------------------------------------------
      led_to_send(); // led para indicar que chegou algo
      //----------------------------------------------
      char dataArray[100];                        // vetor que vai receber string
      incomingString.toCharArray(dataArray, 100); // método para atribui string a vetor
      data = strtok(dataArray, ",");
      data = strtok(NULL, ",");
      data = strtok(NULL, ",");
      RSSI_LoRA = &*data; // para pegar o RSSI do sinal lora
      RSSI_LoRA = strtok(NULL, ",");
      //Serial.println(data);  // imprime pacote recebido (sem outros detalhes)

      //----------------------------------------------
      // Dividir data em pacotes
      type_data = 0; // tipo de dado pre estabelecido
      int i, j = 0, startPos = -1, count = 0;
      // conferir se há T no vetor

      for (i = 0; data[i] != '\0'; i++){
        if (strchr(find_gatway, data[i])){                // se device estver procurando o gatwat
          type_data = 1;                                  // flag para indicar que o pacote é passado
          break;
        }
        if (strchr(last_data, data[i])){ // exsitir "Z" no pacote o dado é passado
          sprintf(markers, "ABCDEFGHIK"); // atribui K como ultimo caracter (depois do horario)
          type_data = 2;                  // flag para indicar que o pacote é passado
          break;
        }
        else{

          type_data = 3;                 // flag para pacote em tempo real
          sprintf(markers, "ABCDEFGHI"); // caso contrario o pacote é em tempo real
        }
      }
      //----------------------------------------------
      if (type_data == 1){
        Serial.println("TIPO -> [Device procurando gatway]");
      }
      else // debug
        if (type_data == 2){
          Serial.println("TIPO -> [Pacote da memoria]");
        }
        else // debug
          if (type_data == 3){
            Serial.println("TIPO -> [Pacote atual]");
          } // debug
      //----------------------------------------------
      // pré processamento - lógica para quebrar o pacote em partes se pacote requiser isso
      if (type_data == 1){
        int size_data = strlen(data); // confere o tamanho do pacote
        data[size_data - 1] = '\0';   // quebra o ultimo valor ( caractere "O")

        for (i = 0; i < size_data; i++){    // laço para preencger extractedStrings com a informação de interesse                
          extractedStrings[0][i] = data[i]; // atribuição de cada caractere
        }
      }
      if (type_data == 2 || type_data == 3){ // se o pacote for do tipo 2 ou 3
        for (i = 0; data[i] != '\0'; i++){
          if (strchr(markers, data[i])) // analisa se o caractere
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
        //----------------------------------------------
        // Organizar formato de hora: 2023-10-05T11:30:44Z
        // Separar Lat e lon
        separate_lat_and_lon();
      }
      //------------------------------------
      // imprimir nível da conexão com Wifi
      rssi = WiFi.RSSI();
      Serial.print("Nível de sinal Wi-Fi: ");
      Serial.print(rssi);
      Serial.println(" dBm");
      //------------------------------------
      // imprimir dados recebidos após pré processamento
      for (int i = 0; i < count - 1; i++){ // -1 por que J não conta
        printf("dado %c: %s\n", extractedStrings[i + 1][0], extractedStrings[i + 1] + 1);
      }
      //------------------------------------------------------
      //atribuição de dado ao pacote Json
      if (type_data == 1){
        doc["Bat_Perc"] = atoi(extractedStrings[0]);
      }
      if (type_data == 2){
        doc["time"] = (extractedStrings[9] + 1);
      }
      if (type_data == 2 || type_data == 3){
        doc["latitude"] = atof(latlon[0]);
        doc["longitude"] = atof(latlon[1]);
        doc["vel"] = atoi(extractedStrings[2] + 1);
        doc["temperatura"] = atoi(extractedStrings[3] + 1);
        doc["umidade"] = atoi(extractedStrings[4] + 1);
        doc["X"] = atof(extractedStrings[5] + 1);
        doc["Y"] = atof(extractedStrings[6] + 1);
        doc["Z"] = atof(extractedStrings[7] + 1);
        doc["Bat_Perc"] = atoi(extractedStrings[8] + 1);
      }
      if (type_data == 1 || type_data == 2 || type_data == 3){
        doc["RSSI_LoRa"] = atoi(RSSI_LoRA);
        doc["RSSI_WIFI"] = rssi;
      }
      /*
      // Serializar o objeto JSON em uma string
      if (incomingString.indexOf('\r') != -1) {
      Serial.println("Caracteres \\r encontrados na incomingString.");
      }
      */
      jsonData = "";
      serializeJson(doc, jsonData); // Serializa o pacote json
      Serial.println(jsonData);     // imprime o json montado que será encaminhado via MQTT
      //---------------------------
      // confere conexão
      if (!client.connected())
      {
        reconnect();
      }
      client.loop();
      //-------------------------------------------
      // Publicar no tópico especificado
      if (client.publish(topic, jsonData.c_str())){ // encaminha json montado!
        Serial.println("Message published successfully");
        delay(200);
        flag_mqtt = true; // se foi publicado a mensagem de confirmação será enviada
      }
      else{
        Serial.println("Failed to publish message");
        // adicionar aqui o salvamento da mensagem
        delay(200);
        flag_mqtt = false; // se não foi publicado a mensagem de confirmação não será enviada
      }
      client.endPublish();        //finaliza publicação MQTT
      //-------------------------------------------
      zerar_extractedStrings(); // para zerar a matriz de dados
      Serial.println("string zerada");
      //-------------------------------------------
      if (flag_mqtt){
        send_confirmation(); // se o pacote foi publicado a mensagem de confirmação é enviada para o device
      }
      else{
        toggleSerial(true); // Liga a comunicação serial novamente
      }
      //-------------------------------------------
    }
  }
}

void PINS_AND_SYSTEM(){
  Serial.begin(115200);  
  time_break_line = millis();
  time_show_msg = millis();

  pinMode(led_to_receive, OUTPUT);
  digitalWrite(led_to_receive, LOW); 
  pinMode(pin_led, OUTPUT);
  digitalWrite(pin_led, HIGH);
}

void LORA_SETTINGS(){
  lora.begin(115200, SERIAL_8N1, rxLoRa, txLoRA); 
  lora.println("AT+ADDRESS?");                    
  Serial.println(lora.readString());              
  delay(20);
  lora.println("AT+BAND?");                    
  Serial.println(lora.readString());              
  delay(20);
  lora.println("AT+NETWORKID?");                    
  Serial.println(lora.readString());  
}

void WIFI_SETTINGS(){
  WiFi.mode(WIFI_STA);
  WiFiManager wm; 
  // Gatway.resetSettings();
  bool res;
  // res = Gatway.autoConnect("Gatway_ESP32","biomasimo"); // password protected ap
  wm.setConfigPortalTimeout(40); // auto close config portal after n seconds
  res = wm.autoConnect("ESP32","123456789"); // 
  if (!res){
    Serial.println("Failed to connect");
    // ESP.restart();
  }
  else{
    // if you get here you have connected to the WiFi
    Serial.println("connected...yeey :)");
    //Serial.println(WiFi.localIP());  // Mostrar o IP (wifi manager ja faz isso)
  }
  //------------------------------------
  // MQTT
  client.setServer(mqttServer, mqttPort); // define servidor e porta
  //client.setCallback(callback);         // define callback para receber msg de confirmação
}

void separate_lat_and_lon(){
  strncpy(latlon[0], extractedStrings[1] + 1, 9);     //+1 para pular o caracter, 9 para recolher os 9 bytes de informação
  strncpy(latlon[1], extractedStrings[1] + 1 + 9, 9); //+1 para pular o caracter, 9 para recolher os 9 bytes de informação
}

void send_confirmation(){
  toggleSerial(true); // Liga a comunicação serial novamente
  Serial.println("Serial ligada");
  char conf[30]; // vetor para empacote mensagem de confirmacao
  sprintf(conf, "AT+SEND=%c,2,OK", end_to_send);
  Serial.println(conf); // mostra mensagem a ser enviada
  lora.println(conf);   // manda a mensagem de confirmacao de recebimento
  Serial.println("mandando mensagem de confirmação..");
  Serial.println(lora.readString()); // lê a resposta do módulo
  led_to_send();
  delay(100);
  led_to_send();
  Serial.println("========[FIM DO PROCESSO -> AGUARDANDO NOVA MENSAGEM]==========");
}

void led_to_send(){
  digitalWrite(led_to_receive, HIGH); // Liga Led
  delay(100);                     // tempo de led ligado
  digitalWrite(led_to_receive, LOW);  // Desliga led
}

void zerar_extractedStrings(){ 
  int i, j;
  // zerar extractedStrings
  for (i = 0; i < 13; ++i){
    for (j = 0; j < 21; ++j){
      extractedStrings[i][j] = '\0';
    }
  }
  // zerar utctime
  for (i = 0; i < 26; i++){
    utctime[i] = '\0';
  }
  // zerar latlon
  for (i = 0; i < 3; i++){
    for (j = 0; j < 11; j++){
      latlon[i][j] = '\0';
    }
  }
  // zerando demais variáveis
  RSSI_LoRA = 0;
  rssi = 0;
  doc.clear(); // Isso apagará todo o conteúdo do documento
}

void toggleSerial(bool enable){
  if (enable){
    lora.begin(115200, SERIAL_8N1, rxLoRa, txLoRA);
  }
  else{
    lora.end();
  }
}

void reconnect(){
  int cont_to_reset = 0;
  while (!client.connected()){
    client.setServer(mqttServer, mqttPort);
    Serial.println("\nConectando ao broker MQTT...");

    //if (client.connect("ESP32Client")){
    if (client.connect("ESP32Client", mqttUser, mqttPassword)){
      Serial.println("Conectado");
      break;
    }
    else{
      Serial.print("Falha na conexão - Estado: ");
      Serial.print(client.state());
      Serial.println(" Tentando novamente");
      cont_to_reset++;
    }
    if (cont_to_reset >= 2){
      Serial.println("Não foi possivel reconectar ao Broker, reiniciando sistema");
      Serial.println("=======================================");
      ESP.restart();
    }
    vTaskDelay(50);
  }
}