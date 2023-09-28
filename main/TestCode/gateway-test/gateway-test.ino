// biblitoecas e variáveis para WIfi/MQTT
#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

// Configurações da rede Wi-Fi
//const char *ssid = "NOS-2E40";
//const char *password = "2TJA5RZ9";
WiFiClient espClient;
 //const char* ssid = "iPhone de Mateus";
 //const char* password = "12345678";

 const char* ssid = "agents";
 const char* password = "QgC9O8VucAByqvVu5Rruv1zdpqM66cd23KG4ElV7vZiJND580bzYvaHqz5k07G2";

// Configurações do broker MQTT
const char *mqttServer = "broker.mqtt-dashboard.com";
bool flag_mqtt = false; //flag para garantir envio do pacote.
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

char end_to_send = '1'; // endereço do outro lora

char markers[12];// = "ABCDEFGHIJK";  // J indica o fim da string
//char makers_with_T[] = "ABCDEFGHIJK"; //K indica o fim da string

char *data; // para armazenar as informações que chegam
char extractedStrings[12][20]; // 9 caracteres de A a I e tamanho suficiente para armazenar os valores
void zerar_extractedStrings(); //para zerar a matriz
char type_data = 0; //tipo de dado que está chegando [0] - atual; [1] - dado guardado
//---------------------------------------------------------
// outos pinos do sistema
#define led_to_rec 21

void setup()
{
  Serial.begin(115200);                           // connect serial
  lora.begin(115200, SERIAL_8N1, rxLoRa, txLoRA); // connect lora  modulo
  lora.println("AT+ADDRESS?"); // para conferir o endereco do modulo
  Serial.println(lora.readString()); // para conferir o endereco do modulo
  //------------------------------------
  // definições WIFI/MQTT
  setup_wifi();
  client.setServer(mqttServer, mqttPort);
  //------------------------------------
  // outros pinos
  pinMode(led_to_rec,OUTPUT);
  digitalWrite(led_to_rec, LOW);//Desliga led
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

    char dataArray[100]; //vetor que vai receber string
    incomingString.toCharArray(dataArray, 100); //método para atribui string a vetor
    data = strtok(dataArray, ",");
    data = strtok(NULL, ",");
    data = strtok(NULL, ",");
    // Serial.println(incomingString);
    Serial.println(data);
  
    //----------------------------------------------
    // Dividir data em pacotes
    type_data = 0; //tipo de dado pre estabelecido
    int i, j = 0, startPos = -1, count = 0;
    //conferir se há T no vetor
    char last_data[] = "Z"; //Caracter para conferir se o dado é anterior  

    for (i = 0;data[i] != '\0'; i++){
      if(strchr(last_data,data[i])){ // exsitir "Z" no pacote o dado é passado
       sprintf(markers, "ABCDEFGHIJZK"); //atribui K como ultimo caracter (depois do horario)
       type_data = 1; //flag para indicar que o pacote é passado
       break;
      }
      else{
        type_data = 0; //flag para pacote em tempo real
        sprintf(markers, "ABCDEFGHIJ"); // caso contrario o pacote é em tempo real
      }
    }
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
    for (int i = 0; i < count-1; i++){ // -1 por que J não conta
    printf("dado %c: %s\n", extractedStrings[i + 1][0], extractedStrings[i + 1] + 1);
    }
    //------------------------------------------------------
    DynamicJsonDocument doc(256); // Tamanho do buffer JSON
    
    doc["latitude"] = atof(extractedStrings[1] + 1);    // -- A
    doc["longitude"] = atof(extractedStrings[2] + 1);   // -- B
    doc["vel"] = atof(extractedStrings[3] + 1);         // -- C
    doc["temperatura"] = atof(extractedStrings[4] + 1); // -- D
    doc["umidade"] = atof(extractedStrings[5] + 1);     // -- E
    doc["X"] = atof(extractedStrings[6] + 1);           // -- F
    doc["Y"] = atof(extractedStrings[7] + 1);           // -- G
    doc["Z"] = atof(extractedStrings[8] + 1);           // -- H
    doc["Bat_Perc"] = atof(extractedStrings[9] + 1);    // -- I
    if(type_data == 1) doc["time"] = (extractedStrings[10] + 1); // -- J
    // Serializar o objeto JSON em uma string
    if (incomingString.indexOf('\r') != -1) {
    Serial.println("Caracteres \\r encontrados na incomingString.");
    }
    String jsonData = "";
    serializeJson(doc, jsonData);
    Serial.println(jsonData);
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
    if (flag_mqtt){ //se o pacote foi publicado a mensagem de confirmação é enviada para o device
    toggleSerial(true); // Liga a comunicação serial novamente
    Serial.println("Serial ligada");
    char conf[30]; //vetor para empacote mensagem de confirmacao
    sprintf(conf, "AT+SEND=%c,2,OK",end_to_send);
    lora.println(conf); //manda a mensagem de confirmacao de recebimento
    Serial.println("mandando mensagem de confirmação..");
    Serial.println(lora.readString()); //lê a resposta do módulo
    led_to_receive();
    }
    else{
    toggleSerial(true); // Liga a comunicação serial novamente
    }
    //-------------------------------------------
  }
}

void led_to_receive(){
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
      Serial.println(" Tentando novamente em 1 segundos");
      delay(1000);
    }
  }
}
