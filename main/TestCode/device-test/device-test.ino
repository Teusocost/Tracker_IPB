//---------------------------------------------------------
// biblitoecas e variáveis para o GPS = UART
#include <TinyGPS++.h>
#include <HardwareSerial.h>
#define rxGPS 16
#define txGPS 17

// variávies para GPS
double lat = 0, lon = 0;
int sat = 0, vel = 0, year = 0, month = 0, day = 0, hour = 0, minute = 0, second = 0;
HardwareSerial gpsSerial(2);
TinyGPSPlus gps;
float time_wait_gps = 120000; // tempo em ms que espera o GPS receber coordenadas
float time_control = 0; //para controlar o tempo
float time_comp = 5000; // diferença entre segundo ao qual a mensagem "coord n encontradas" será mostrado
//---------------------------------------------------------
// biblitoecas e definições para o Rylr 998 (LoRa) = UART
#define rxLORA 25
#define txLORA 26
HardwareSerial lora(1);

char LED_BUILTIN_MQTT_SEND = 2; //pisca led quando envia LoRa
char end_to_send = '2';   // endereço do lora que vai receber esse pacote

unsigned int time_to_resend = 3000; // tempo em ms para nova tentativa de envio LoRa
unsigned int time_finish_resend = 30000; //Tempo em ms de tentativas

char mensagem[120]; //Vetor para mensagem completa
char data[80]; //Vetor para apenas variáveis
char keep[100]; //Vetor para guardar pacotes com data/hour
String incomingString = "NULL";  // string que vai receber as informações
char *searchTerm = "OK"; //mensagem que chega para confirmação
char *conf; // para armazenar as informações que chegam

bool serialEnabled = true; // Variável de controle para a comunicação serial
//---------------------------------------------------------
// Armazenamento SPIFFs
#include "SPIFFS_Utils.h"
SPIFFS_Utils spiffsUtils; 
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
#define TIME_TO_SLEEP 4

//---------------------------------------------------------
//funções instanciadas antes que o sistema passe a funcionar
void led_to_send();
void toggleSerial_lora(bool enable);
void keep_data();
//---------------------------------------------------------
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
    Serial.println("Resentando");
    ESP.restart(); //Função para resetar ESP
  }
  //------------------------------------
  // LoRa
  lora.begin(115200, SERIAL_8N1, rxLORA, txLORA); // connect gps sensor
  lora.println("AT+ADDRESS?"); // para conferir o endereco do modulo
  Serial.println(lora.readString()); // para conferir o endereco do modulo
}

void loop(){
  toggleSerial_lora(false);  //Comunicacao com LoRa Desligado
  esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR); //saí do modo sleep mode quando time * factor
  Serial.println("=======ESP INICIADO========="); //debug serial.print
  //Serial.println(lora.readString()); // para conferir o endereco do modulo
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
  unsigned long now = millis(); // iniciando função para contagem
  while (gpsSerial.available()){ // Entra no laço se comunicação está ok
    if (gps.encode(gpsSerial.read())){ // decodifiação de dados recebidos
      gps.encode(gpsSerial.read()); // processar dados brutos
      while(lat == 0 && lon == 0 ){ // loop para aguardar latitude e longitude
        gpsSerial.available();
        gps.encode(gpsSerial.read()); //confere se dados brutos chegaram e processa
        read_all_data_gps(); //lê todos os dados
        if(millis() - time_control >= time_comp){ // condição para imprimir status (aguardando)
          time_control = millis();
          Serial.println("Aguardaddo coordenadas");
          if ((millis() - now) >= time_wait_gps) { // Se não encontrar lat e lon os dados são enviados sem esses
            Serial.println("Coordenadas não encontradas"); // informa essa condição
            break;
          }
        }
      }
      read_all_data_gps(); //Se não foi necessário entrar no laço ele armazena os dados de qualquer maneira!
      break;
      //-----------------------------------
    }
  }
  //-----------------------------------
  //-----------------------------------
  //organizar e enviar LoRa - tentativa atual

  sprintf(data, "A%.6fB%.6fC%iD%.2fE%.2fF%.2fG%.2fH%3.2fI%.0dJ",lat, lon, vel, temperature, humidity, event.acceleration.x, event.acceleration.y, event.acceleration.z, Percentage); //atribui e organiza as informações em data
  //o caractere J indica o fim da mensagem
  int requiredBufferSize = snprintf(NULL, 0, "%s",data); //calcula tamanho string
  sprintf(mensagem, "AT+SEND=%c,%i,%s",end_to_send,requiredBufferSize,data); // junta as informações em "mensagem"
  toggleSerial_lora(true); //liga a comunicação com LoRa
  lora.println(mensagem); //manda a mensagem montada para o módulo
  Serial.println(mensagem); //imprime no monitor a mensagem montada 
  Serial.println(lora.readString()); //lê a resposta do módulo

  //----------------------------------------
  //protocolo de confirmação de envio
  Serial.println("Aguardando confirmação (30 segundos)");
  float time = millis(); //administrar tempo geral de tentativa
  float time_reenv = millis()+time_to_resend; //adminstrar tempo de reenvio
  int tent = 1; //n de tentativas
  while(-1){ // laco para receber confirmação
    //--------------------------------
    //Confere se confirmação chegou
    toggleSerial_lora(true); // liga serial
    delay(10);
    incomingString = lora.readString(); //lê a resposta do módulo
    toggleSerial_lora(false); //desliga serial
    //------------------------------
    //reenvia mensagem
    if(millis() >= time_reenv){ //em n segundos, se não chegou confirmacao
      Serial.printf("Reenviando pacote, tentativa %d\n",tent++);
      time_reenv = millis()+time_to_resend; 
      reen_data();
    }
    if(millis()-time >= time_finish_resend){ //fim do laço de tentativas
      Serial.println("fim de tentativas");
      keep_data(); //função para guardar dados em SPIFFS
      Serial.println("Informação guardada");
      break; //fecha laço While
    }
    if(incomingString != NULL){ // se chegou algum dado
      Serial.println(incomingString); // mostra dado
      char dataArray[50]; // vetor para trabalhar com informação
      incomingString.toCharArray(dataArray, 50); //transforma string em char
      //Serial.println(dataArray);
      conf = strtok(dataArray, ","); // quebra ,
      conf = strtok(NULL, ","); //quebra ,
      conf = strtok(NULL, ","); //quebra ,
      Serial.println(conf); //Mostra a confirmação
      char msg_to_conf[] = "OK"; //Mensagem esperada
      if(strcmp(conf,"OK")==0){ //Se mensagem que chegou for a mesma que a esperada
      Serial.println("confirmação chegou!"); //mostra confirmacao
      break; //fecha laço While
      }
    }
  }
  //-----------------------------------
  //força o ESP32 entrar em modo SLEEP
  Serial.println(("sistema entrando em Deep Sleep"));
  esp_deep_sleep_start();
  //-----------------------------------
  //led para indicar envio
  void led_to_send();
  //-----------------------------------
  //força o ESP32 entrar em modo SLEEP
  esp_deep_sleep_start(); 
} // fim loop

void led_to_send (){
  digitalWrite(LED_BUILTIN_MQTT_SEND, HIGH); //Liga Led
  delay(200); //tempo de led ligado
  digitalWrite(LED_BUILTIN_MQTT_SEND, LOW);//Desliga led
}

void reen_data(){ //funcao para reenviar dados
  toggleSerial_lora(true); //Liga serial
  delay(10);
  lora.println(mensagem); //manda a mensagem montada para o módulo 
  Serial.println(lora.readString()); //lê a resposta do módulo
  toggleSerial_lora(false); //DEsliga Serial
}

void toggleSerial_lora(bool enable){ //funcao ligar/desligar comunicao com LORA
  if (enable){
    lora.begin(115200, SERIAL_8N1, rxLORA, txLORA); 
  }
  else{
    lora.end();
  }
}

void keep_data(){
  Serial.println("dado que serão guardados");
  sprintf(keep, "%s%.0i-%.0i-%.0iT%.0i:%.0i:%.0i\n",data,year,month,day,hour,minute,second);
  Serial.println(keep);
  spiffsUtils.appendToFile("/dados.txt", keep); //grava um novo valor em SPIFF
  Serial.println("dados guardados!");
  Serial.println("Todos os dados:");
  spiffsUtils.listFiles();
}

void read_all_data_gps(){
  lat = gps.location.lat(); // latitude
  lon = gps.location.lng(); // longitude
  sat = gps.satellites.value(); //número de satélites
  vel = gps.speed.mps(); //velocidade
  year = gps.date.year(); //ano
  month = gps.date.month(); //mes
  day = gps.date.day(); //dia
  hour = gps.time.hour(); //hora
  minute = gps.time.minute(); //minuto
  second = gps.time.second(); //segundo
}