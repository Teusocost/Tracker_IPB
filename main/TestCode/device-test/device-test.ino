//---------------------------------------------------------
// biblitoecas e variáveis para o GPS = UART
#include <TinyGPS++.h>
#include <HardwareSerial.h>
#define rxGPS 16
#define txGPS 17

// variávies para GPS
double lat = 0, lon = 0;
int sat = 0, vel = 0, year = 0, month = 0, day = 0, hour = 0, minute = 0, second = 0;
int delay_read_gps = 1000, counter_gps_cicle =0, n_cicles_gps = 90; //delay, contador, tempo que o sistema vai ler (delay * n ciclos)
HardwareSerial gpsSerial(2);
TinyGPSPlus gps;
unsigned long now; //variavel de controle de tempo
//---------------------------------------------------------
// biblitoecas e definições para o Rylr 998 (LoRa) = UART
#define rxLORA 25
#define txLORA 26
HardwareSerial lora(1);

#define LED_BUILTIN_MQTT_SEND  2 //pisca led quando envia LoRa
char end_to_send = '2';   // endereço do lora que vai receber esse pacote

unsigned long time_geral; //variavel de controle - administrar tempo geral de tentativa
int tent = 1;     //variavel de controle - n de tentativas
float time_reenv; //variavel de controle - adminstrar tempo de reenvio
bool flag_to_delete_last_data = false;
unsigned int time_to_resend = 5000; // tempo em ms para nova tentativa de envio LoRa
unsigned int time_finish_resend = 6000; //Tempo em ms de tentativas
String lastValue;

int requiredBufferSize = 0; // quantidade de bytes que serão enviados (variavel)
char mensagem[120]; //Vetor para mensagem completa
char data[80]; //Vetor para apenas variáveis
char keep[100]; //Vetor para guardar pacotes com data/hour
String incomingString = "NULL";  // string que vai receber as informações
char *searchTerm = "OK"; //mensagem que chega para confirmação
char *conf; // para armazenar as informações que chegam

bool serialEnabled = true; // Variável de controle para a comunicação serial

int min_quality_signal_resend = -50; //qualidade minima do sinal lora para enviar dado da memoria


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
float x = 0,y=0,z=0;
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
#define TIME_TO_SLEEP 5

//---------------------------------------------------------
//funções instanciadas antes que o sistema passe a funcionar
#define status_sensor_lora 32
#define status_sensors 18 //ADXL345 e DHT21
#define status_battery 4
void led_to_send();
void toggleSerial_lora(bool enable);
void toggleSerial_gps(bool enable);
void keep_data();
void configuration_to_confirmation();
 esp_reset_reason_t reason;
//---------------------------------------------------------
void setup()
{
  //------------------------------------
  // definições placa
  Serial.begin(115200);
  // pinos de leds/transistores/leituras de subsistemas
  pinMode(LED_BUILTIN_MQTT_SEND, OUTPUT); //indicar envio
  pinMode(status_sensor_lora, OUTPUT); //status antena lora
  pinMode(status_sensors, OUTPUT); //status sensores
  pinMode(status_battery, OUTPUT); //status bateria
  digitalWrite(status_sensors,HIGH); //liga todos os sensores (DHT21, ADXL345) (permancem sempre ligados)
  //------------------------------------
  // Verifique o motivo da reinicialização - feito para definir o estado do sistema, ou seja, para encaminhar ou não um pacote HELLO
  reason = esp_reset_reason();
  //------------------------------------
  // gps definições
  toggleSerial_gps(true); //sistema ja liga gps
  delay(25);
  if(gpsSerial.available()){
    Serial.println("Sensor GPS não encontrado, verifique o esquema eletrico");
    security_function();
    }
  Serial.println("ligando o gps em modo hot");
  gpsSerial.print("$PMTK101*32<CR><LF>\r\n"); //acorda o gps em modo hot
  lat = 0; lon = 0;

  //------------------------------------
  // sht21 definições
  Wire.begin(SDA_PIN, SCL_PIN); // Inicializa a comunicação I2C
  //------------------------------------
  // Acelerômetro
  if (!accel.begin()){
    Serial.println("No ADXL345 sensor detected.");
    security_function();
  }

  //------------------------------------
  // LoRa
  toggleSerial_lora(true); //comunicacao GPS ligada
  digitalWrite(status_sensor_lora,HIGH);
  delay(100);
  lora.println("AT+ADDRESS?"); // para conferir o endereco do modulo
  Serial.println(lora.readString()); // para conferir o endereco do modulo
  digitalWrite(status_sensor_lora,LOW);
}

void loop(){
  esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR); //saí do modo sleep mode quando time * factor
  if(reason != ESP_RST_DEEPSLEEP){
    digitalWrite(status_sensor_lora,HIGH); //liga LoRa
    Serial.println("[SISTEMA REINICIADO] -> ENVIAR PACOTE HELLO");
    send_hello(); //função para mandar um hello e encontrar o gateway;
    goto wait_confirmation;
  }
  Serial.println("=======ESP ACORDADO========="); //debug serial.print
  //Serial.println(lora.readString()); // para conferir o endereco do modulo
  // Capturando dados GPS
  Serial.println("Processando/aguardando dados GPS");

  now = millis(); // iniciando função para contagem
  while (gpsSerial.available()){ // Entra no laço se comunicação está ok
    if (gps.encode(gpsSerial.read())){ // decodifiação de dados recebidos
      gps.encode(gpsSerial.read()); //interpreta dados brutos
      read_all_data_gps(); //lê todos os dados
      printallvalues();
      Serial.println("----------------------");
      if(lat != 0 && lon != 0){
        Serial.println("Coordenadas encontradas");
        break; // loop para aguardar latitude e longitude
      }
      if(counter_gps_cicle >= n_cicles_gps ){
        Serial.println("\nCoordenadas não encontradas"); // informa essa condição
        digitalWrite(status_sensors,LOW); //desliga todos os sensores (DHT21, L80 ADXL345) (NÃO SERÃO UTILIZADOS)
        gps_standby();
        goto without_lat_lon; //programa pula envio ou não de pacote antigos 
        break;
      }
      counter_gps_cicle++;
      delay(delay_read_gps);
    }
  }
  gps_standby(); //coloca o gps em standby
  //------------------------------------
  // status da bateria (funççao externa)

  digitalWrite(status_battery,HIGH); //liga sistema leitura baterias
  batterystatus(Voltage, Percentage);
  digitalWrite(status_battery,LOW); //desliga sistema leitura baterias
  //------------------------------------
  // leitura de temperatura e humidade SHT21
  readSHT21Data(temperature, humidity); // Chama a função para ler os dados do sensor SHT21       
  sensors_event_t event;
  accel.getEvent(&event);
  //------------------------------------
  x = event.acceleration.x; //alecerometro em x
  y = event.acceleration.y; //alecerometro em y
  z = event.acceleration.z; //alecerometro em z
  digitalWrite(status_sensors,LOW); //desliga todos os sensores (DHT21, L80 ADXL345)
  //-----------------------------------
  //organizar e enviar LoRa - tentativa atua
  if(lat != 0 && lon != 0){ //caso o sistema pule a conferencia realizada no gps ele n passa dessa parte
    Serial.println("=======Enviar informacoes atuais========="); //debug serial.print
    digitalWrite(status_sensor_lora,HIGH); //liga LoRa
    sprintf(data, "A%.6f%.6fB%iC%.2fD%.2fE%.2fF%.2fG%3.2fH%.0dI",lat, lon, vel, temperature, humidity, x, y, z, Percentage); //atribui e organiza as informações em data
    //o caractere J indica o fim da mensagem
    requiredBufferSize = snprintf(NULL, 0, "%s",data); //calcula tamanho string
    sprintf(mensagem, "AT+SEND=%c,%i,%s",end_to_send,requiredBufferSize,data); // junta as informações em "mensagem"
    reen_data(); //funcao para enviar dados
  }
  //-----------------------------------
  without_lat_lon:// se não houver lat e long sistema já vem para cá
  lastValue = spiffsUtils.readLastValue("/dados.txt");
  if(lat == 0 && lon == 0 && lastValue != NULL){ // se não houver lat e lon atuais o programa apenas confere se há pacotes antigos para enviar e tenta encontrar conexão com gatway
    Serial.println("=======[Ex. memoria] -> Procurar GATWAY=========");
    digitalWrite(status_sensor_lora,HIGH); //liga LoRa
    send_hello(); //função para mandar um hello e encontrar o gateway;
  }
  else if(lat == 0 && lon == 0 && lastValue == NULL){
    Serial.println("=======[N Ex. memoria - lat e lon N existem]=========");
    goto without_coord; //não a nada a ser feito, sistema dorme.
  }
  //----------------------------------------
  //protocolo de confirmação de envio
  wait_confirmation:
  Serial.println("==========Aguardar confirmacao========="); //debug serial.print
  configuration_to_confirmation();
  while(-1){ // laco para receber confirmação
    //--------------------------------
    //Confere se confirmação chegou
    toggleSerial_lora(true); // liga serial
    //delay(10);
    incomingString = lora.readString(); //lê a resposta do módulo
    toggleSerial_lora(false); //desliga serial
    //------------------------------
    //reenvia mensagem
    if(millis() >= time_reenv){ //em n segundos, se não chegou confirmacao
      Serial.printf("Reenviando pacote, tentativa %d\n",tent++);
      time_reenv = millis()+time_to_resend; //atualizacao de time_reenv
      reen_data(); //reenvia pacote
    }
    if(millis()-time_geral >= time_finish_resend){ //fim do laço de tentativas
      Serial.println("fim de tentativas");
      if(lat != 0 && lon != 0 && flag_to_delete_last_data == false){
        keep_data(); //função para guardar dados em SPIFFS
        Show_SPIFFS();
        Serial.println("pacote de dados guardado");
        break; //fecha laço While
      }
      else 
      if(flag_to_delete_last_data == true){
        Serial.println("pacote de dados retorna para memoria");
        Show_SPIFFS();
        break;
      }
      else{
        Serial.println("pacote nao guardado (lat e lon off)");
        Show_SPIFFS();
        break;
      }
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
      if(strcmp(conf,"OK")==0){ //Se a confirmação chegou for a mesma que a esperada
        Serial.println("confirmação chegou!"); //mostra confirmacao
        delete_ultimate_data(); //funcao para apagr ultima sinal armazenado, se for o caso
        conf = strtok(NULL, ","); //quebra para conferir qualidade de sinal
        if(quality_signal_lora(atof(conf))){ //se a qualidade do sinal estiver boa
          Serial.println("sinal ta bom");
          lastValue = spiffsUtils.readLastValue("/dados.txt"); //devolve o ultimo valor gravado
          if(lastValue == NULL) {
            Serial.println("não há nada para enviar");
          }
          else{
            Serial.println("=======Enviar dados guardados========="); //debug serial.print
            Serial.println("Último valor gravado:");
            Serial.println(lastValue); //imprime o ultimo valor gravado
            delay(1000); //delay para debug no gatway
            flag_to_delete_last_data = true; //flag para indicar que dado pode ser apagado depois de confirmacao de envio
            char lastvalue_char[80]; //vetor que vai receber o ultimo valor guardado (string -> char)
            lastValue.toCharArray(lastvalue_char, 80); //método transforma string em char
            requiredBufferSize = snprintf(NULL, 0, "%s",lastvalue_char); //calcula tamanho string
            sprintf(mensagem, "AT+SEND=%c,%i,%s",end_to_send,requiredBufferSize,lastvalue_char); // junta as informações em "mensagem"
            reen_data(); //funcao para enviar dados
            configuration_to_confirmation(); //atualiza variaiveis de controle de tempo para aguardar confirmacao
            incomingString = "NULL"; //apaga ultimo dado recebido em serial 
            goto wait_confirmation; //retorna para receber confirmação de envio
          }
        }
        flag_to_delete_last_data = false;
        break;//fecha confirmação
      }
      else{
        goto wait_confirmation; //retorna para receber confirmação de envio
      }
    }
  }
  //-----------------------------------
  without_coord: //sistema pula para ca se não apresentar coordenadas
  digitalWrite(status_sensor_lora,LOW); //desliga LoRa
  //força o ESP32 entrar em modo SLEEP
  Serial.println(("Sistema entrando em Deep Sleep"));
  Serial.println("Desligando todos os sensores");
  Serial.println("=======Fim do processo========="); //debug serial.print
  esp_deep_sleep_start();
} // fim loop


void send_hello(){ //função para mandar um hello e econtrar o gatway
  sprintf(data, "HELLO"); //envia "hello" para conferir se gateway esta por perto
    requiredBufferSize = snprintf(NULL, 0, "%s",data); //calcula tamanho string
    sprintf(mensagem, "AT+SEND=%c,%i,%s",end_to_send,requiredBufferSize,data);
    reen_data(); //funcao para enviar dados
}

void gps_standby(){
  Serial.println("colocando o gps em standby");
  gpsSerial.print("$PMTK161,0*28<CR><LF>\r\n"); //coloca o gps em standby
}

void configuration_to_confirmation(){
  Serial.println("Aguardando confirmação");
  time_geral = millis(); //administrar tempo geral de tentativa
  time_reenv = millis()+time_to_resend; //adminstrar tempo de reenvio
  tent = 1; //n de tentativas
}

void delete_ultimate_data (){
  if (flag_to_delete_last_data){
    spiffsUtils.removeLastValue("/dados.txt");
  }

}

int quality_signal_lora(int value){
  bool result = (value >= min_quality_signal_resend)? result = true: result = false; // se a qualidade do sinal estiver = ou acima do valor esperado
  return result; //retorna o status
}

void led_to_send (){
  digitalWrite(LED_BUILTIN_MQTT_SEND, HIGH); //Liga Led
  delay(200); //tempo de led ligado
  digitalWrite(LED_BUILTIN_MQTT_SEND, LOW);//Desliga led
}

void reen_data(){ //funcao para reenviar dados
  toggleSerial_lora(true); //Liga serial
  delay(10);
  Serial.println("enviando pacote LoRa");
  lora.println(mensagem); //manda a mensagem montada para o módulo 
  Serial.println(mensagem);
  Serial.println(lora.readString()); //lê a resposta do módulo
  toggleSerial_lora(false); //DEsliga Serial
  //-----------------------------------
  //led para indicar envio
  led_to_send();
}

void toggleSerial_lora(bool enable){ //funcao ligar/desligar comunicao com LORA
  if (enable){
    lora.begin(115200, SERIAL_8N1, rxLORA, txLORA); 
  }
  else{
    lora.end();
  }
}

void toggleSerial_gps(bool enable){ //funcao ligar/desligar comunicao com LORA
  if (enable){
    gpsSerial.begin(9600, SERIAL_8N1, rxGPS, txGPS); // connect gps sensor 
  }
  else{
    gpsSerial.end();

  }
}

void keep_data(){
  Serial.println("dado que serão guardados");
  sprintf(keep, "%s%i%02i%02i%02i%02i%02iK\n",data,year,month,day,hour,minute,second); //K é o final da mensage,
  Serial.println(keep);
  spiffsUtils.appendToFile("/dados.txt", keep); //grava um novo valor em SPIFF
}

void Show_SPIFFS(){
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

void printallvalues(){
  Serial.print("LAT: ");
  Serial.println(lat, 6);
  Serial.print("LONG: ");
  Serial.println(lon, 6);
  Serial.print("SPEED: ");
  Serial.println(vel);

  Serial.print("Date: ");
  Serial.print(day); Serial.print("/");
  Serial.print(month); Serial.print("/");
  Serial.println(year);

  Serial.print("Hour: ");
  Serial.print(hour); Serial.print(":");
  Serial.print(minute); Serial.print(":");
  Serial.println(second);
}

void security_function(){ //função de seguranca caso algum módulo não for encontrado no sistema
  Serial.println("Sistema com mal funcionamento");
  Serial.println("Desligando/adormecendo todos os módulos");
  digitalWrite(status_sensors,LOW);
  digitalWrite(status_battery,LOW);
  digitalWrite(status_sensor_lora,LOW);
  gps_standby(); 
  Serial.println("colocando o sistema em standby");
  esp_deep_sleep_start();
}