// biblitoecas e variáveis para WIfi/MQTT
//#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <WiFiManager.h> // https://github.com/tzapu/WiFiManager -> para adicionar colocar login e senha

//-------------------------------------------
// Configurações da rede Wi-Fi (não necessário depois da library Wifimeneger)
//const char *ssid = "NOS-2E40";
//const char *password = "2TJA5RZ9";

 //const char* ssid = "iPhone de Mateus";
 //const char* password = "12345678";

 //const char* ssid = "agents";
 //const char* password = "QgC9O8VucAByqvVu5Rruv1zdpqM66cd23KG4ElV7vZiJND580bzYvaHqz5k07G2";

WiFiClient espClient;
int32_t rssi; // variavel para recever sinal RSSI wifi
// Configurações do broker MQTT
const char *mqttServer = "broker.mqtt-dashboard.com";
bool flag_mqtt = false; //flag para garantir envio do pacote.
const int mqttPort = 1883;
const char *mqttUser = "USUARIO_DO_BROKER";
const char *mqttPassword = "SENHA_DO_BROKER";
PubSubClient client(espClient);
char *RSSI_LoRA;

String jsonData = ""; //para receber json
DynamicJsonDocument doc(256); // Tamanho do buffer JSON
//---------------------------------------------------------
// Lora - Uart
#include <HardwareSerial.h>

#define rxLoRa 16
#define txLoRA 17

HardwareSerial lora(1); // objeto Lora
String incomingString;  // string que vai receber as informações

char end_to_send = '1'; // endereço do outro lora

char markers[12];// = "ABCDEFGHIJK";  // J indica o fim da string
//char makers_with_T[] = "ABCDEFGHIJK"; //K indica o fim da string

char *data; // para armazenar as informações que chegam
char extractedStrings[12][20]; // 9 caracteres de A a I e tamanho suficiente para armazenar os valores
char utctime[20]; //vetor que vai receber time
char last_data[] = "K"; //Caracter para conferir se o pacote é da memoria (UTC Time Format has a Z)  
char find_gatway[] = "O"; //mensagem que device envia para encontrar gatway
void zerar_extractedStrings(); //para zerar a matriz
char type_data = 0; //tipo de dado que está chegando [0] - atual; [1] - dado guardado
//---------------------------------------------------------
#include <Arduino.h>
// outos pinos do sistema
#define led_to_rec 21
// variaveis para função millis (mostrar "." enquanto nao recebe sinal);
unsigned int break_line = 60000; // 60 segundos (tempo de reinício de função) (milis)
unsigned int time_to_show_point = 1000; //"." é mostrado a cada tempo (milis)
unsigned long time_break_line; //variavel de controle
unsigned long time_show_msg; //variavel de controle
void setup()
{
  Serial.begin(115200);                           // connect serial
  lora.begin(115200, SERIAL_8N1, rxLoRa, txLoRA); // connect lora  modulo
  lora.println("AT+ADDRESS?"); // para conferir o endereco do modulo
  Serial.println(lora.readString()); // para conferir o endereco do modulo
  //------------------------------------
  // definições WIFI/MQTT
  //setup_wifi();
  WiFiManager Gatway; //objeto do tipo wifimeneger 
  bool res;
  res = Gatway.autoConnect("Gatway_ESP32","biomasimo"); // password protected ap
  if(!res) {
    Serial.println("Failed to connect");
    // ESP.restart();
  } 
  else {
    //if you get here you have connected to the WiFi    
    Serial.println("connected...yeey :)");
  }
    
  client.setServer(mqttServer, mqttPort);
  //------------------------------------
  // outros pinos
  pinMode(led_to_rec,OUTPUT);
  digitalWrite(led_to_rec, LOW);//Desliga led
  //------------------------------------
  //millis
  time_break_line = millis();
  time_show_msg = millis();
}

void loop(){
  start: // para usar goto se necessário
  //----------------------------------------------
  //mostra "." para indicar que está aguardando pacote LORA
  
  if (millis()-time_break_line>=break_line) {
    // Reinicie o contador millis()
    Serial.print("\n"); //quera linha
    time_break_line = millis();
  }
  if(millis()-time_show_msg >= time_to_show_point){
    Serial.print(".");
    time_show_msg=millis();
  }
  if (lora.available()){
    Serial.println("\n========[PACOTE CHEGOU]=========="); //debug serial
    incomingString = lora.readString();
    Serial.println(incomingString);
    //----------------------------------------------
    toggleSerial(false); // Desliga a comunicação serial para não haver interrupções indesejadas
    //----------------------------------------------

    char dataArray[100]; //vetor que vai receber string
    incomingString.toCharArray(dataArray, 100); //método para atribui string a vetor
    data = strtok(dataArray, ",");
    data = strtok(NULL, ",");
    data = strtok(NULL, ",");
    RSSI_LoRA = &*data; // para pegar o RSSI do sinal lora
    RSSI_LoRA = strtok(NULL, ",");
    // Serial.println(incomingString);
    Serial.println(data);
  
    //----------------------------------------------
    // Dividir data em pacotes
    type_data = 0; //tipo de dado pre estabelecido
    int i, j = 0, startPos = -1, count = 0;
    //conferir se há T no vetor

    for (i = 0;data[i] != '\0'; i++){
      if (strchr(find_gatway,data[i])){ //se device estver procurando o gatwat
        Serial.println("[Device procurando gatway]"); //debug
        delay(200);
        send_confirmation(); //envia confirmação
        goto start; //termina loop
      }
      if(strchr(last_data,data[i])){ // exsitir "Z" no pacote o dado é passado
        
        sprintf(markers, "ABCDEFGHIJK"); //atribui K como ultimo caracter (depois do horario)
        type_data = 1; //flag para indicar que o pacote é passado
        break;
      }
      else{
        
        type_data = 0; //flag para pacote em tempo real
        sprintf(markers, "ABCDEFGHIJ"); // caso contrario o pacote é em tempo real
      }
    }
    //----------------------------------------------
    if(type_data == 0){Serial.println("[Pacote atual]");} else //debug
    if(type_data == 1){Serial.println("[Pacote da memoria]");} //debug
    //----------------------------------------------
    //pré processamento - lógica para quebrar o pacote em partes 
    for (i = 0; data[i] != '\0'; i++){ 
      if (strchr(markers, data[i])) //analisa se o caractere 
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
    //Organizar formato de hora: 2023-10-05T11:30:44Z
    convert_time_to_UTC_format();
    //for (int i = 0; i < count-1; i++){ // -1 por que J não conta
    //printf("dado %c: %s\n", extractedStrings[i + 1][0], extractedStrings[i + 1] + 1);
    //}
    //------------------------------------
    // imprimir nível da conexão com Wifi
    rssi = WiFi.RSSI();
    Serial.print("Nível de sinal Wi-Fi: ");
    Serial.print(rssi);
    Serial.println(" dBm");
    //------------------------------------------------------
    doc["latitude"] = atof(extractedStrings[1] + 1);    // -- A
    doc["longitude"] = atof(extractedStrings[2] + 1);   // -- B
    doc["vel"] = atof(extractedStrings[3] + 1);         // -- C
    doc["temperatura"] = atof(extractedStrings[4] + 1); // -- D
    doc["umidade"] = atof(extractedStrings[5] + 1);     // -- E
    doc["X"] = atof(extractedStrings[6] + 1);           // -- F
    doc["Y"] = atof(extractedStrings[7] + 1);           // -- G
    doc["Z"] = atof(extractedStrings[8] + 1);           // -- H
    doc["Bat_Perc"] = atof(extractedStrings[9] + 1);    // -- I
    if(type_data == 1) doc["time"] = utctime; // -- J
    doc["RSSI_LoRa"] = atof(RSSI_LoRA);
    doc["RSSI_WIFI"] = rssi;
    // Serializar o objeto JSON em uma string
    if (incomingString.indexOf('\r') != -1) {
    Serial.println("Caracteres \\r encontrados na incomingString.");
    }
    jsonData = "";
    serializeJson(doc, jsonData);
    Serial.println(jsonData);
    //---------------------------
    // confere conexão
    if (!client.connected()){
      reconnect();
    }
    client.loop();
    //-------------------------------------------
    // Publicar no tópico especificado
    if (client.publish("IPB/TESTE/TRACKER/01", jsonData.c_str())){ // encaminha json montado!
      Serial.println("Message published successfully");
      delay(200);
      flag_mqtt = true; //se foi publicado a mensagem de confirmação será enviada
    }
    else{
      Serial.println("Failed to publish message");
      //adicionar aqui o salvamento da mensagem
      delay(200);
      flag_mqtt = false; //se não foi publicado a mensagem de confirmação não será enviada
    }
    //-------------------------------------------
    void zerar_extractedStrings(); //para zerar a matriz de dados
    Serial.println("string zerada");
    //-------------------------------------------
    if (flag_mqtt){
      send_confirmation(); //se o pacote foi publicado a mensagem de confirmação é enviada para o device
    }
    else{
    toggleSerial(true); // Liga a comunicação serial novamente
    }
    //-------------------------------------------
  }
}

void convert_time_to_UTC_format(){
    // Copie a parte da data
    strncpy(utctime, extractedStrings[10]+1, 4); // Copia "2023"
    utctime[4] = '-'; // Adiciona '-'
    strncpy(utctime + 5, extractedStrings[10]+1 + 4, 2); // Copia "10"
    utctime[7] = '-'; // Adiciona '-'

    // Copie a parte da hora
    strncpy(utctime + 8, extractedStrings[10]+1 + 6, 2); // Copia "05"
    utctime[10] = 'T'; // Adiciona 'T'
    strncpy(utctime + 11, extractedStrings[10]+1 + 8, 2); // Copia "11"
    utctime[13] = ':'; // Adiciona ':'
    strncpy(utctime + 14, extractedStrings[10]+1 + 10, 2); // Copia "30"
    utctime[16] = ':'; // Adiciona ':'
    strncpy(utctime + 17, extractedStrings[10]+1 + 12, 2); // Copia "44"
    utctime[19] = 'Z'; // Null-terminate a string
    utctime[20] = '\0'; // Null-terminate a string
    // Imprima a string resultante
    printf("Horário com os caracteres adicionados: %s\n", utctime);
}

void send_confirmation(){
  toggleSerial(true); // Liga a comunicação serial novamente
  Serial.println("Serial ligada");
  char conf[30]; //vetor para empacote mensagem de confirmacao
  sprintf(conf, "AT+SEND=%c,2,OK",end_to_send);
  Serial.println(conf); //mostra mensagem a ser enviada
  lora.println(conf); //manda a mensagem de confirmacao de recebimento
  Serial.println("mandando mensagem de confirmação..");
  Serial.println(lora.readString()); //lê a resposta do módulo
  led_to_send();
  Serial.println("[FIM DO PROCESSO] -> AGUARDAR NOVA MENSAGEM");
}

void led_to_send(){
  digitalWrite(led_to_rec, HIGH); //Liga Led
  delay(200); //tempo de led ligado
  digitalWrite(led_to_rec, LOW);//Desliga led
}

void zerar_extractedStrings(){ //função para zerar string que armazena os dados
    for (int i = 0; i < 12; ++i) {
        for (int j = 0; j < 21; ++j) {
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


/*void setup_wifi()
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
*/

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
      Serial.println(" Tentando novamente em 2 segundos");
      delay(2000);
    }
  }
}
