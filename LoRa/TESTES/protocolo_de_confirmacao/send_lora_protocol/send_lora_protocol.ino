#include <HardwareSerial.h>
#define rxLoRa 16
#define txLoRA 17

HardwareSerial lora(2); //n das portas Uart do esp
String incomingString;  // string que vai receber as informações
char *searchTerm = "OK"; //mensagem que chega para confirmação
char *conf; // para armazenar as informações que chegam
char end_to_send = '2';   // endereço do lora que vai recever esse pacote
bool serialEnabled = true; // Variável de controle para a comunicação serial


//---------------------------------------------------------
// Variáveis para entrar em deep sleep mode
// fator de conversão de microsegundos para segundos
#define uS_TO_S_FACTOR 1000000
// tempo que o ESP32 ficará em modo sleep (em segundos)
#define TIME_TO_SLEEP 10


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

void setup(){
  Serial.begin(115200);
  lora.begin(115200);
  lora.setTimeout(1000);
  delay(1500);
  //lora.println("AT+RESET");
  //Serial.println(lora.readString());
  //delay(1000);
  //================OBS=================
  //para salvae as configurações no módulo, digite ",M" depois da definição, por
  // exemplo -> AT+BAND=868000000,M 

  // Por outro lado se quiser upar essas informações via software, pode ser usado
  // o seguinte trecho em setup():

  //---------------------------------------------------
  //lora.println("AT+BAND=868000000"); // AT+SEND=1,2,LO
  //Serial.println(lora.readString());
  //delay(1000);
  //lora.println("AT+ADDRESS=1"); // AT+SEND=1,2,LO
  //Serial.println(lora.readString());
  //delay(1000);
  //lora.println("AT+NETWORKID=18"); // AT+SEND=1,2,LO
  //Serial.println(lora.readString());
  //delay(1000);
  //---------------------------------------------------
}

void loop(){   
  esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR); //saí do modo sleep mode quando time * factor
  toggleSerial(false);  //começa tudo desligado
  double lat = -41.123456;
  double lon = 6.123456;
  int vel = 2;
  float temp = 42.22;
  float hum = 53.44;
  float x = -2.02;
  float y = 4.04;
  float z = 8.41;
  float bat = 80;

  char mensagem[120];
  char data[70];
  sprintf(data, "A%.6fB%.6fC%iD%.2fE%.2fF%.2fG%.2fH%3.2fI%.0fJ",lat, lon, vel, temp, hum, x, y, z, bat); //atribui e organiza as informações em data
  //o caractere J indica o fim da mensagem
  int requiredBufferSize = snprintf(NULL, 0, "%s",data); //calcula tamanho string
  sprintf(mensagem, "AT+SEND=%c,%i,%s",end_to_send,requiredBufferSize,data); // junta as informações em "mensagem"
  toggleSerial(true); //liga a comunicação com LoRa
  lora.println(mensagem); //manda a mensagem montada para o módulo
  Serial.println(mensagem); //imprime no monitor a mensagem montada 
  Serial.println(lora.readString()); //lê a resposta do módulo
  while(-1){
    toggleSerial(true);
    Serial.println("Aguardando confirmação");
    incomingString = lora.readString(); //lê a resposta do módulo
    toggleSerial(false);
    Serial.println(incomingString);
    char dataArray[50]; 
    incomingString.toCharArray(dataArray, 50);
    //Serial.println(dataArray);
    conf = strtok(dataArray, ",");
    conf = strtok(NULL, ",");
    conf = strtok(NULL, ",");
    Serial.println(conf);
    char msg_to_conf[] = "OK";
    if(strcmp(conf,"OK")==0){
    Serial.println("confirmação chegou!");
    break;
    }
  }
    //desliga a comunicação com LoRa
  conf = "";
  //toggleSerial(false);
  //-----------------------------------
  //força o ESP32 entrar em modo SLEEP
  esp_deep_sleep_start(); 
}

