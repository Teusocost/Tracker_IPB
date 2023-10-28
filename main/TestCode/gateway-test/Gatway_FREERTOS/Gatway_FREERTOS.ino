///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// biblitoecas e variáveis para WIfi/MQTT
// #include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <WiFiManager.h> // https://github.com/tzapu/WiFiManager -> para adicionar colocar login e senha

//-------------------------------------------
// Configurações da rede Wi-Fi (não necessário depois da library Wifimeneger)
// const char *ssid = "NOS-2E40";
// const char *password = "2TJA5RZ9";

// const char* ssid = "iPhone de Mateus";
// const char* password = "12345678";

// const char* ssid = "agents";
// const char* password = "QgC9O8VucAByqvVu5Rruv1zdpqM66cd23KG4ElV7vZiJND580bzYvaHqz5k07G2";

WiFiClient espClient;
int32_t rssi; // variavel para recever sinal RSSI wifi
// Configurações do broker MQTT
const char *mqttServer = "broker.mqtt-dashboard.com";
bool flag_mqtt = false; // flag para garantir envio do pacote.
const int mqttPort = 1883;
//const char *mqttUser = "USUARIO_DO_BROKER";
//const char *mqttPassword = "SENHA_DO_BROKER";
const char *topic = "IPB/TESTE/TRACKER/01";
const char *topic2 = "IPB/TESTE/GATWAY/01";
const char *msg_to_status_gatway = "1"; //variavel enviada para informar status da gatway
PubSubClient client(espClient);
int time_to_resend_msg_status_gatway = 30000; //tempo em que o esp reenvia ao BD seu status
char *RSSI_LoRA;

String jsonData = "";         // para receber json
DynamicJsonDocument doc(256); // Tamanho do buffer JSON
//---------------------------------------------------------
// Lora - Uart
#include <HardwareSerial.h>

#define rxLoRa 16
#define txLoRA 17

HardwareSerial lora(1); // objeto Lora
String incomingString;  // string que vai receber as informações

char end_to_send = '1'; // endereço do outro lora

char markers[12]; // = "ABCDEFGHIJK";  // J indica o fim da string
// char makers_with_T[] = "ABCDEFGHIJK"; //K indica o fim da string

char *data;                    // para armazenar as informações que chegam
char extractedStrings[12][20]; // 9 caracteres de A a I e tamanho suficiente para armazenar os valores
char utctime[25];              // vetor que vai receber time
char latlon[2][10];            // veotor para receber latitude e longitude
char last_data[] = "K";        // Caracter para conferir se o pacote é da memoria (UTC Time Format has a Z)
char find_gatway[] = "O";      // mensagem que device envia para encontrar gatway
void zerar_extractedStrings(); // para zerar a matriz
char type_data = 0;            // tipo de dado que está chegando [0] - atual; [1] - dado guardado
//---------------------------------------------------------
#include <Arduino.h>
// outos pinos do sistema
#define led_to_rec 12
// variaveis para função millis (mostrar "." enquanto nao recebe sinal);
unsigned int break_line = 60000;        // 60 segundos (tempo de reinício de função) (milis)
unsigned int time_to_show_point = 1000; //"." é mostrado a cada tempo (milis)
unsigned long time_break_line;          // variavel de controle
unsigned long time_show_msg;            // variavel de controle

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// ja estava aqui
const uint8_t pin_led = 25;
// variaveis que indicam o núcleo
static uint8_t taskCoreZero = 0;
static uint8_t taskCoreOne = 1;
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void setup(){
  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  //adquirido do outro .ino
  Serial.begin(115200);                           // connect serial
  lora.begin(115200, SERIAL_8N1, rxLoRa, txLoRA); // connect lora  modulo
  lora.println("AT+ADDRESS?");                    // para conferir o endereco do modulo
  Serial.println(lora.readString());              // para conferir o endereco do modulo
  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  //ja estavam aqui
  pinMode(pin_led, OUTPUT);
  digitalWrite(pin_led, HIGH);
  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  //------------------------------------
  // definições WIFI
  // setup_wifi();
  //------------------------------------
  WiFi.mode(WIFI_STA);
  WiFiManager Gatway; // objeto do tipo wifimeneger
  // Gatway.resetSettings();
  bool res;
  // res = Gatway.autoConnect("Gatway_ESP32","biomasimo"); // password protected ap
  res = Gatway.autoConnect("Gatway_ESP32","123456789"); // sem senha
  if (!res){
    Serial.println("Failed to connect");
    // ESP.restart();
  }
  else{
    // if you get here you have connected to the WiFi
    Serial.println("connected...yeey :)");
    Serial.println(WiFi.localIP());
  }
  //------------------------------------
  // MQTT
  client.setServer(mqttServer, mqttPort); // define servidor e porta
  //------------------------------------
  // outros pinos
  pinMode(led_to_rec, OUTPUT);
  digitalWrite(led_to_rec, LOW); // Desliga led
  //------------------------------------
  // millis para processos em loop
  time_break_line = millis();
  time_show_msg = millis();
  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  // cria uma tarefa que será executada na função coreTaskZero, com prioridade 1 e execução no núcleo 0
  // coreTaskZero: piscar LED e contar quantas vezes

  // cria uma tarefa que será executada na função coreTaskOne, com prioridade 2 e execução no núcleo 1
  // coreTaskOne: atualizar as informações do display

    xTaskCreatePinnedToCore(
        publishMQTTtest,   /* função que implementa a tarefa */
        "coreTaskOne", /* nome da tarefa */
        5000,          /* número de palavras a serem alocadas para uso com a pilha da tarefa */
        NULL,          /* parâmetro de entrada para a tarefa (pode ser NULL) */
        2,             /* prioridade da tarefa (0 a N) */
        NULL,          /* referência para a tarefa (pode ser NULL) */
        taskCoreZero);  /* Núcleo que executará a tarefa */

    delay(500); // tempo para a tarefa iniciar

    xTaskCreatePinnedToCore(
        debug_led,     /* função que implementa a tarefa */
        "coreTaskTwo", /* nome da tarefa */
        5000,          /* número de palavras a serem alocadas para uso com a pilha da tarefa */
        NULL,          /* parâmetro de entrada para a tarefa (pode ser NULL) */
        1,             /* prioridade da tarefa (0 a N) */
        NULL,          /* referência para a tarefa (pode ser NULL) */
        taskCoreZero);  /* Núcleo que executará a tarefa */
 //
    delay(500); // tempo para a tarefa iniciar
 //
 //   // cria uma tarefa que será executada na função coreTaskTwo, com prioridade 2 e execução no núcleo 0
 //   // coreTaskTwo: vigiar o botão para detectar quando pressioná-lo
    xTaskCreatePinnedToCore(
        processing,   /* função que implementa a tarefa */
        "coreTaskThree", /* nome da tarefa */
        10000,          /* número de palavras a serem alocadas para uso com a pilha da tarefa */
        NULL,          /* parâmetro de entrada para a tarefa (pode ser NULL) */
        3,             /* prioridade da tarefa (0 a N) */
        NULL,          /* referência para a tarefa (pode ser NULL) */
        taskCoreOne); /* Núcleo que executará a tarefa */
    delay(500);        // tempo para a tarefa iniciar
  
}

void loop(){}

// essa função ficará mudando o estado do led a cada 1 segundo
// e a cada piscada (ciclo acender e apagar) incrementará nossa variável blinked


// essa função será responsável apenas por atualizar as informações no
// display a cada 100ms
/*
void LocalIP(void *pvParameters){

  String taskMessage = "Task running on core ";
  taskMessage = taskMessage + xPortGetCoreID();

  int i = 1000; //time para reprintar '.'
  while (true){
    espClient = server.available();
    digitalWrite(pin_led, LOW);
    if (espClient){
      // digitalWrite(pin_led, HIGH);
      while(1){
        digitalWrite(pin_led, LOW);
        espClient.println("Cliente conectado");
        espClient.println("HTTP/1.1 200 OK");
        espClient.println("Content-Type: text/html");
        espClient.println();
        espClient.println("<html><body>");
        espClient.println("<h1>GATWAY ESP32</h1>");

        if (Serialout != "\0"){
          espClient.println("<br>");
          espClient.println(Serialout);
          espClient.println("<br>");
          Serialout = "\0";
        }
        espClient.println("</body></html>");
        espClient.stop();
        Serial.println("Cliente desconectado");
        if (!espClient){
          break;
        }
      }
    }
  }
}
*/
void publishMQTTtest (void *pcParameters){
  while(true){
      if (!client.connected()){
        reconnect();
      }
      client.loop();
    client.loop();
    client.publish(topic2, msg_to_status_gatway);
    Serial.println("Test");
    vTaskDelay(time_to_resend_msg_status_gatway);
  }
}
void debug_led (void *pvParameters){

  String taskMessage = "Task running on core ";
  taskMessage = taskMessage + xPortGetCoreID();

  int control = 0;
  while (true){
    if(WiFi.status() != WL_CONNECTED){
      digitalWrite(pin_led, HIGH);
      
    }
    else{
      digitalWrite(pin_led, LOW);
    }
    vTaskDelay(1000);
  }
}

// essa função será responsável por ler o estado do botão
// e atualizar a variavel de controle.
void processing(void *pvParameters){
  String taskMessage = "Task running on core ";
  taskMessage = taskMessage + xPortGetCoreID();
  while (true){
    //----------------------------------------------
    // mostra "." para indicar que está aguardando pacote LORA
    if (millis() - time_break_line >= break_line){
      // Reinicie o contador millis()
      Serial.print("\n"); // quera linha
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
      // Serial.println(incomingString);
      Serial.println(data);

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
        Serial.println("[Device procurando gatway]");
      }
      else // debug
        if (type_data == 2){
          Serial.println("[Pacote da memoria]");
        }
        else // debug
          if (type_data == 3){
            Serial.println("[Pacote atual]");
          } // debug
      //----------------------------------------------
      // pré processamento - lógica para quebrar o pacote em partes se pacote requiser isso
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
      for (int i = 0; i < count - 1; i++){ // -1 por que J não conta
        printf("dado %c: %s\n", extractedStrings[i + 1][0], extractedStrings[i + 1] + 1);
      }
      //------------------------------------------------------
      if (type_data == 2 || type_data == 3){
        doc["latitude"] = atof(latlon[0]);
        doc["longitude"] = atof(latlon[1]);
        doc["vel"] = atof(extractedStrings[2] + 1);
        doc["temperatura"] = atof(extractedStrings[3] + 1);
        doc["umidade"] = atof(extractedStrings[4] + 1);
        doc["X"] = atof(extractedStrings[5] + 1);
        doc["Y"] = atof(extractedStrings[6] + 1);
        doc["Z"] = atof(extractedStrings[7] + 1);
        doc["Bat_Perc"] = atof(extractedStrings[8] + 1);
      }
      if (type_data == 2){
        doc["time"] = atof(extractedStrings[9] + 1);
      }
      if (type_data == 1 || type_data == 2 || type_data == 3){
        doc["RSSI_LoRa"] = atof(RSSI_LoRA);
        doc["RSSI_WIFI"] = rssi;
      }
      /*
      // Serializar o objeto JSON em uma string
      if (incomingString.indexOf('\r') != -1) {
      Serial.println("Caracteres \\r encontrados na incomingString.");
      }
      */
      jsonData = "";
      serializeJson(doc, jsonData);
      Serial.println(jsonData);
      //---------------------------
      // confere conexão
      if (!client.connected())
      {
        reconnect();
      }
      client.loop();
      //-------------------------------------------
      // Publicar no tópico especificado
      if (client.publish(topic, jsonData.c_str(),2)){ // encaminha json montado! QoS = 2
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
  Serial.println("[FIM DO PROCESSO] -> AGUARDAR NOVA MENSAGEM");
}

void led_to_send(){
  digitalWrite(led_to_rec, HIGH); // Liga Led
  delay(100);                     // tempo de led ligado
  digitalWrite(led_to_rec, LOW);  // Desliga led
}

void zerar_extractedStrings(){ // função para zerar string que armazena os dados
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

bool serialEnabled = true; // Variável de controle para a comunicação serial

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
    Serial.print("Conectando ao broker MQTT...");

    // if (client.connect("ESP32Client", mqttUser, mqttPassword))
    if (client.connect("ESP32Client")){
      Serial.println("Conectado");
      // client.subscribe("TOPICO_SUBSCRIBER");
    }
    else{
      Serial.print("Falha na conexão - Estado: ");
      Serial.print(client.state());
      Serial.println(" Tentando novamente em 5 segundos");
      delay(5000);
      cont_to_reset++;
    }
    if (cont_to_reset >= 2){
      Serial.println("Não foi possivel reconectar ao Broker, reiniciando sistema");
      ESP.restart();
    }
  }
}