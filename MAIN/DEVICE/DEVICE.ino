/* ============================================================================
 * 
 *   Code to Node
 *   This code went developer to last work in IPB - BRAGANÇA 
 * 
 *   Autor: Mateus Costa de Araujo 
 *   Data:  February, 2024
 *
 *   OBS: Write this code in 20mhz in arduino framework
============================================================================ */

#include <TinyGPS++.h>                  // GNSS - UART
#include <HardwareSerial.h>
#include "SPIFFS_Utils.h"
#include <Wire.h>                       //SHT21 and accelerometer I2C
#include "sht21.h"
#include "batterystatus.h"
#include <Adafruit_Sensor.h>            // accelerometer
#include <Adafruit_ADXL345_U.h>         // accelerometer
#include <EEPROM.h>
#include <math.h>
//==========================================================================
// Defines
#define rxGPS 16
#define txGPS 17
#define rxLORA 25
#define txLORA 26
#define LED_BUILTIN_MQTT_SEND 2 
#define SDA_PIN 21 
#define SCL_PIN 22 
#define uS_TO_S_FACTOR 1000000 // to converte micro in secund
#define TIME_TO_SLEEP 10        //second to Deep Sleep                              (EDITABLE)
#define status_sensor_lora 32         
#define status_sensors 18             
#define status_battery 4
#define EEPROM_ADDRESS_TO_FILTER_GPS 0
//==========================================================================
// GPS global variables
double lat = 0.0, 
       lon = 0.0;
       
int sat    = 0, 
    vel    = 0, 
    year   = 0, 
    month  = 0, 
    day    = 0, 
    hour   = 0, 
    minute = 0, 
    second = 0;

unsigned long time_gps_wait = 300 * 1000,                 // time waiting signal GNSS                 (EDITABLE)
    delay_read_gps = 1000,                                // time across two GNSS read
    time_to_available_gps = 3 * 1000;                     // time waiting the GNSS response           (EDITABLE)
unsigned short filter_to_first_acquisition_GPS_data = 15, // filter the first acquisition gps data    (EDITABLE)
               commum_filter_acquisition_GPS_data = 3;    // filter the acquisition gps data          (EDITABLE)
unsigned long now = 0;                                    // Control time
unsigned long now_finish = 0;                             // Control time
char count_blink_led = 0;                                 // Flag on when device turn on in first
bool EEPROM_flag_gps_filter = false;                      // Flag to filter GPS data
int decimal = 6;                                          // decimal place to analyze in gps          (EDITABLE)
HardwareSerial gpsSerial(2);
TinyGPSPlus gps;
//---------------------------------------------------------
//LoRa variables
HardwareSerial lora(1);
char end_to_send = '2';                  // Address to send package
unsigned long time_geral = 0;            // Send attempt flag
unsigned short tent = 1;                            // Attempt number (send LoRa package)
unsigned int time_to_resend = 10 * 1000; // Time (ms) to resend lora package
float time_reenv = 0.0;
bool flag_to_delete_last_data = false;
unsigned int time_finish_resend = time_to_resend * 2; // attempts number
String lastValue;
int requiredBufferSize = 0;     // Package size
char mensagem[120],             // Final package before send messege
     data[80],                   // Corrent messege package
     keep[100];                  // Save messe in SPIFFS
String incomingString = "NULL"; // Primary information received
char *searchTerm = "OK";        // Confirmation messege resend to node
char *conf;                     // Manipulation package variable
bool serialEnabled = true;
int min_quality_signal_resend = -80; //                                           (EDITABLE)
//---------------------------------------------------------
//SPIFFS variables
SPIFFS_Utils spiffsUtils;
//---------------------------------------------------------
// SHT21 variables
float temperature = 0.0, 
      humidity = 0.0;
//---------------------------------------------------------
// accelerometer variables
sensors_event_t event;
Adafruit_ADXL345_Unified accel = Adafruit_ADXL345_Unified();
float x = 0.0, 
      y = 0.0, 
      z = 0.0;
//---------------------------------------------------------
// Baterry variables
float Voltage = 0.0;
int   Percentage = 0.0;
//---------------------------------------------------------
//system variables
esp_reset_reason_t reason;            // Reason that reset microcontroller
bool flag_firts_on = 0;
//==========================================================================
//Function
void EEPROM_FLAGS();
void PINS();
void GNSS_ON(); 
void SHT21_ADXL345_ON();
void Device_switch_turn_on();
void First_GNSS_test();
void GET_GNSS_DATA();
bool FILTER_GPS_DATA(short type);
int comparation(double previous, double current);
void EEPROM_SET_FLAG_TO_FILTER_GNSS_TRUE();
void EEPROM_SET_FLAG_TO_FILTER_GNSS_FALSE();
void GET_BATTERY_STATUS();
void GET_ACCELEROMETER_DATA();
void SEND_LORA_CURRENT_PACKAGE();
void SEND_LORA_MEMORY_PACKAGE();
void SLEEP_DEVICE_WITHOUT_PACKAGE();
void print_vallues();
void send_hello();
void read_batery_system();
void gps_standby();
void gps_standby();
void configuration_to_confirmation();
void delete_ultimate_data();
int quality_signal_lora(int value);
void led_to_send();                   
void reen_data();
void toggleSerial_lora(bool enable);  
void toggleSerial_gps(bool enable);   
void keep_data();                     
void Show_SPIFFS();
void read_all_data_gps();
void printallvalues();
void security_function();
//==========================================================================
// SETUP
void setup(){
  //--------------------------
  //System definitions
  setCpuFrequencyMhz(20);
  Serial.begin(115200);
  EEPROM_FLAGS(); 
  PINS();
  reason = esp_reset_reason(); //reason to restart microcontroller
  //--------------------------
  // Modules definitions
  GNSS_ON(); // GPS ON
  SHT21_ADXL345_ON();
  //------------------------------------
  if (reason != ESP_RST_DEEPSLEEP) Device_switch_turn_on();
} 
//==========================================================================
// The device actions
void loop(){
  esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR); // Wake up
  Serial.println("=======ESP ACORDADO========="); 
  Serial.println("Processando/aguardando dados GPS");
  //------------------------------------
  // collecting all data
  First_GNSS_test();
  GET_GNSS_DATA();
  GET_BATTERY_STATUS();
  readSHT21Data(temperature, humidity);
  GET_ACCELEROMETER_DATA();
  digitalWrite(status_sensors, LOW); // Turn off DHT21 and ADXL345
  print_vallues();                   
  //------------------------------------
  if (lat != 0 && lon != 0) SEND_LORA_CURRENT_PACKAGE();
  else 
    lastValue = spiffsUtils.readLastValue("/dados.txt");
    if (lat == 0 && lon == 0 && lastValue != NULL) SEND_LORA_MEMORY_PACKAGE();
  else 
    if (lat == 0 && lon == 0 && lastValue == NULL){
      SLEEP_DEVICE_WITHOUT_PACKAGE(); 
      goto finish_process;
    }
  //without_lat_lon: // se não houver lat e long sistema já vem para cá
  //------------------------------------
  // Confirmation protocol
  WAIT_GATEWAY_CONFIRMATION();
  digitalWrite(status_sensor_lora, LOW); //turn off LoRa
  //------------------------------------
  // Finish process
  finish_process:                          
  Serial.println("=======Fim do processo========="); 
  esp_deep_sleep_start();
} // End process
//==========================================================================
// FUNCTIONS
void EEPROM_FLAGS(){
  EEPROM.begin(1);
  EEPROM_flag_gps_filter = EEPROM.read(EEPROM_ADDRESS_TO_FILTER_GPS);
  Serial.print("Flag EEPROM GNSS: ");
  Serial.println(EEPROM_flag_gps_filter);
}

void PINS(){
  pinMode(LED_BUILTIN_MQTT_SEND, OUTPUT); 
  pinMode(status_sensor_lora, OUTPUT);    
  pinMode(status_sensors, OUTPUT);        
  pinMode(status_battery, OUTPUT);        
  digitalWrite(status_sensors, HIGH);     // (DHT21, ADXL345)
  digitalWrite(status_sensor_lora, LOW);  
}

void GNSS_ON(){
  toggleSerial_gps(true); // GPS ON
  delay(25);
  if (gpsSerial.available()){
    Serial.println("Sensor GPS não encontrado, verifique o esquema eletrico");
    security_function();
  }
  Serial.println("ligando o gps em modo hot");
  Serial.println("Carregando buffer GNSS");
  gpsSerial.print("$PMTK101*32<CR><LF>\r\n"); // hot mode
  delay(500); //debug
  gpsSerial.print("$PMTK101*32<CR><LF>\r\n"); // hot mode
  delay(1000); //debug
}

void SHT21_ADXL345_ON(){
  Wire.begin(SDA_PIN, SCL_PIN); // I2C - SHT21
  if (!accel.begin()){
      Serial.println("No ADXL345 sensor detected.");
      security_function();
  }
}

void Device_switch_turn_on(){
  flag_firts_on = 1;
  toggleSerial_lora(true);
  digitalWrite(status_sensor_lora, HIGH); // LoRa On
  delay(100);
  lora.println("AT+ADDRESS?");
  Serial.println(lora.readString());
  delay(20);
  lora.println("AT+BAND?");
  Serial.println(lora.readString());
  delay(20);
  lora.println("AT+NETWORKID?");
  Serial.println(lora.readString());
  digitalWrite(status_sensor_lora, HIGH);
  Serial.println("[SISTEMA REINICIADO] -> ENVIAR PACOTE HELLO");
  send_hello();
  led_to_send();
  digitalWrite(status_sensor_lora, LOW); //  LoRa OFF
  Serial.println("Carregando buffer GNSS");
  delay(6000); // buffer gnss
}

void First_GNSS_test(){
  now = millis(); 
  now_finish = now;
  while(!gpsSerial.available()){ 
    if(millis()>= now){
      Serial.println("aguardando acionamento do GNSS");
      gpsSerial.print("$PMTK101*32<CR><LF>\r\n"); //  Hot mode
      now = millis()+1000;
    }
    if(millis() >= now_finish+time_to_available_gps){ 
      Serial.println("Algo errado com o GNSS");
      Serial.println("ESP DORIMNDO");
      security_function();
    }
  }
}

void GET_GNSS_DATA(){ 

  while (millis() < (now + time_gps_wait)){ 
    if (gps.encode(gpsSerial.read())){ // Is there data?
      Serial.print("Millis: ");
      Serial.println(millis());
      Serial.print("Time alvo: ");
      Serial.println(now + time_gps_wait);
      Serial.print("STATUS: ");
      Serial.println(gpsSerial.available()?"TRUE":"FALSE"); 
      gps.encode(gpsSerial.read()); // Read raw data 
      read_all_data_gps();          
      printallvalues();
      Serial.println("----------------------");

      if (lat != 0 && lon != 0 && gps.location.isValid()){
        Serial.println("Coordenadas encontradas");

        if (flag_firts_on || EEPROM_flag_gps_filter){ // filter gps data
          if(FILTER_GPS_DATA(1))
            break;
        }
        else{
          if(FILTER_GPS_DATA(0))
            break;
        }
      }
      delay(delay_read_gps);
    }
  }
  if (millis() >= (now + time_gps_wait) && lat == 0 && lon == 0){
    Serial.println("Coordenadas não encontradas"); 
    if(!EEPROM_flag_gps_filter) EEPROM_SET_FLAG_TO_FILTER_GNSS_TRUE();

    //digitalWrite(status_sensors, LOW);             // desliga todos os sensores (DHT21, L80 ADXL345) (NÃO SERÃO UTILIZADOS)
    //gps_standby();
    //goto without_lat_lon; // programa pula envio ou não de pacote antigos
  }
  gps_standby(); // coloca o gps em standby
}

bool FILTER_GPS_DATA(short type){
  static short i = 0;
  static double lat_temp = 0,
                lon_temp = 0;
  if(!comparation(lat_temp,lat) || !comparation(lon_temp,lon)){
    lat_temp = lat;
    lon_temp = lon;
    i++;
    Serial.print("Apurando localização, n: ");
    Serial.println(i);

    switch (type){
      case 0:
        if (i >= commum_filter_acquisition_GPS_data)
          return 1;
        else
          return 0;

      case 1:
        if (i >= filter_to_first_acquisition_GPS_data){
          EEPROM_SET_FLAG_TO_FILTER_GNSS_FALSE();
          return 1;
        }
        else
          return 0;
      default:
        return 0;
    }
  }
  else
   return 0;
}

int comparation(double previous, double current){
    static double tol = 1.0 / pow(10, decimal);
    return fabs(previous - current) < tol;
}

void EEPROM_SET_FLAG_TO_FILTER_GNSS_TRUE(){
    EEPROM.write(EEPROM_ADDRESS_TO_FILTER_GPS, true);
    EEPROM.commit();
    Serial.println("Valor atualizado na EEPROM: TRUE");
}

void EEPROM_SET_FLAG_TO_FILTER_GNSS_FALSE(){
    EEPROM.write(EEPROM_ADDRESS_TO_FILTER_GPS, false);
    EEPROM.commit();
    Serial.println("Valor atualizado na EEPROM: FALSE");
}

void GET_BATTERY_STATUS(){
  digitalWrite(status_battery, HIGH); 
  batterystatus(Voltage, Percentage);
  digitalWrite(status_battery, LOW); 
}

void GET_ACCELEROMETER_DATA(){
  accel.getEvent(&event);
  x = event.acceleration.x;          
  y = event.acceleration.y;          
  z = event.acceleration.z; 
}

void SEND_LORA_CURRENT_PACKAGE(){
    Serial.println("=======Enviar informacoes atuais=========");                                                              
    digitalWrite(status_sensor_lora, HIGH);                                                                                   
    sprintf(data, "A%.6f%.6fB%iC%.0fD%.0fE%.2fF%.2fG%.2fH%.0dI", lat, lon, vel, temperature, humidity, x, y, z, Percentage); // package montage
    // o caractere J indica o fim da mensagem
    requiredBufferSize = snprintf(NULL, 0, "%s", data);                           // String size
    sprintf(mensagem, "AT+SEND=%c,%i,%s", end_to_send, requiredBufferSize, data); // Complete Package
    reen_data();                                                                  // Send
}

void SEND_LORA_MEMORY_PACKAGE(){
  Serial.println("=======[Ex. memoria] -> Procurar GATWAY=========");
  digitalWrite(status_sensor_lora, HIGH); 
  send_hello();                           
}

void SLEEP_DEVICE_WITHOUT_PACKAGE(){
  Serial.println("=======[N Ex. memoria - lat e lon N existem]=========");
}

void WAIT_GATEWAY_CONFIRMATION(){
  wait_confirmation:
  Serial.println("==========Aguardar confirmacao========="); 
  configuration_to_confirmation();

  while (-1){ 
    //--------------------------------
    toggleSerial_lora(true); 
    incomingString = lora.readString(); // Read LoRa module response 
    toggleSerial_lora(false);           
    //------------------------------
    if (millis() >= time_reenv){ 
      Serial.printf("Reenviando pacote, tentativa %d\n", tent++);
      time_reenv = millis() + time_to_resend; 
      reen_data();                            // Resend
    }
    if (millis() >= time_geral+time_to_resend-(tent*1000)){     // End of attempts (subtraction prevents resubmission)
      Serial.println("fim de tentativas");
      if (lat != 0 && lon != 0 && humidity!= 0 && x > -12 && x < 12 && y > -12 && y < 12 && z > -12 && z < 12 && flag_to_delete_last_data== false ){
        keep_data(); 
        // Show_SPIFFS();
        Serial.println("pacote de dados guardado");
        break; 
      }
      else if (flag_to_delete_last_data == true){
        Serial.println("pacote de dados retorna para memoria");
        // Show_SPIFFS();
        break;
      }
      else{
        Serial.println("pacote nao guardado (lat e lon off)");
        // Show_SPIFFS();
        break;
      }
    }
    ///////////////////////////////////////////////////////////////////////////////////////////////////////
    // MSD RECEIVED

    if (incomingString != NULL){ // se chegou algum dado

      Serial.println(incomingString);            
      char dataArray[50];                        
      incomingString.toCharArray(dataArray, 50); // String to char
      // Serial.println(dataArray);
      conf = strtok(dataArray, ","); // cut ,
      conf = strtok(NULL, ",");      
      conf = strtok(NULL, ",");      
      Serial.println(conf);          
      char msg_to_conf[] = "OK";     // Expected message

      if (strcmp(conf, "OK") == 0){            
        Serial.println("confirmação chegou!"); 
        setCpuFrequencyMhz(240);               // Overclock
        delay(10);                             
        delete_ultimate_data();                
        conf = strtok(NULL, ",");              // cut ,
        if (quality_signal_lora(atof(conf))){  // check signal
          lastValue = spiffsUtils.readLastValue("/dados.txt"); 
          setCpuFrequencyMhz(20);              // Overclock
          delay(10);                           
          Serial.println("sinal ta bom");
          if (lastValue == NULL){
            Serial.println("não há nada para enviar");
          }
          else{
            Serial.println("=======Enviar dados guardados========="); 
            Serial.println("Último valor gravado:");
            Serial.println(lastValue);                                                              // imprime o ultimo valor gravado
            delay(1000);                                                                            // delay para debug no gatway
            flag_to_delete_last_data = true;                                                        // flag para indicar que dado pode ser apagado depois de confirmacao de envio
            char lastvalue_char[80];                                                                // vetor que vai receber o ultimo valor guardado (string -> char)
            lastValue.toCharArray(lastvalue_char, 80);                                              // método transforma string em char
            requiredBufferSize = snprintf(NULL, 0, "%s", lastvalue_char);                           // calcula tamanho string
            sprintf(mensagem, "AT+SEND=%c,%i,%s", end_to_send, requiredBufferSize, lastvalue_char); // junta as informações em "mensagem"
            reen_data();                                                                            // funcao para enviar dados
            configuration_to_confirmation();                                                        // atualiza variaiveis de controle de tempo para aguardar confirmacao
            incomingString = "NULL";                                                                // apaga ultimo dado recebido em serial
            goto wait_confirmation;                                                                 // retorna para receber confirmação de envio
          }
        }
        setCpuFrequencyMhz(20);              // desacelera a CPU do mesmo jeito
        delay(10);                           // debug
        flag_to_delete_last_data = false;
        break; // fecha confirmação
      }
      else{      
        goto wait_confirmation; // retorna para receber confirmação de envio
      }
    } // fecha tratamento de mensagem recebida
  }   // fecha bloco de confirmação
}

void print_vallues(){
  Serial.print("Voltagem: ");
  Serial.println(Voltage);
  Serial.print("Percentage: ");
  Serial.println(Percentage);
  Serial.print("xyz: ");
  Serial.print(x);
  Serial.print(" ");
  Serial.print(y);
  Serial.print(" ");
  Serial.println(z);
  Serial.print("temperature: ");
  Serial.println(temperature);
  Serial.print("humidity: ");
  Serial.println(humidity);
}

void send_hello(){                                    
  read_batery_system();
  sprintf(data, "%dO",Percentage);                     // envia pacote com nível a bateria para conferir se gateway esta por perto
  requiredBufferSize = snprintf(NULL, 0, "%s", data); // calcula tamanho string
  sprintf(mensagem, "AT+SEND=%c,%i,%s", end_to_send, requiredBufferSize, data);
  reen_data();                                        // funcao para enviar dados
}

void read_batery_system(){
  digitalWrite(status_battery, HIGH); // liga sistema leitura baterias
  delay(1); //debug do transsitor
  batterystatus(Voltage, Percentage);
  digitalWrite(status_battery, LOW); // desliga sistema leitura baterias
}

void gps_standby(){
  Serial.println("colocando o gps em standby");
  gpsSerial.print("$PMTK161,0*28<CR><LF>\r\n"); // coloca o gps em standby
}

void configuration_to_confirmation(){
  Serial.println("Aguardando confirmação");
  time_geral = millis() + time_finish_resend;                  // administrar tempo para todas os envios
  time_reenv = millis() + time_to_resend; // adminstrar tempo de reenvio
  tent = 1;                               // n de tentativas
}

void delete_ultimate_data(){
  if (flag_to_delete_last_data){
    spiffsUtils.removeLastValue("/dados.txt");
  }
}

int quality_signal_lora(int value){
  bool result = (value >= min_quality_signal_resend) ? result = true : result = false; // se a qualidade do sinal estiver = ou acima do valor esperado
  return result;                                                                       // retorna o status
}

void led_to_send(){
  digitalWrite(LED_BUILTIN_MQTT_SEND, HIGH); // Liga Led
  delay(200);                                // tempo de led ligado
  digitalWrite(LED_BUILTIN_MQTT_SEND, LOW);  // Desliga led
}

void reen_data(){          
  toggleSerial_lora(true); // Liga serial
  delay(10);
  Serial.println("enviando pacote LoRa");
  lora.println(mensagem); // manda a mensagem montada para o módulo
  Serial.println(mensagem);
  Serial.println(lora.readString()); // lê a resposta do módulo
  toggleSerial_lora(false);          // DEsliga Serial
  led_to_send();
}

void toggleSerial_lora(bool enable){ 
  if (enable){
    lora.begin(115200, SERIAL_8N1, rxLORA, txLORA);
  }
  else{
    lora.end();
  }
}

void toggleSerial_gps(bool enable){ 
  if (enable){
    gpsSerial.begin(9600, SERIAL_8N1, rxGPS, txGPS); // connect gps sensor
  }
  else{
    gpsSerial.end();
  }
}

void keep_data(){
  Serial.println("dado que serão guardados");
  sprintf(keep, "%s%02d%02d%02d%02d%K\n", data,month, day, hour, minute); // K é o final da mensage,
  Serial.println(keep);
  spiffsUtils.appendToFile("/dados.txt", keep); // grava um novo valor em SPIFF
}

void Show_SPIFFS(){
  Serial.println("Todos os dados:");
  spiffsUtils.listFiles();
}

void read_all_data_gps(){
  lat = gps.location.lat();     // latitude
  lon = gps.location.lng();     // longitude
  sat = gps.satellites.value(); // número de satélites
  vel = gps.speed.mps();        // velocidade
  year = gps.date.year();       // ano
  month = gps.date.month();     // mes
  day = gps.date.day();         // dia
  hour = gps.time.hour();       // hora
  minute = gps.time.minute();   // minuto
  second = gps.time.second();   // segundo
}

void printallvalues(){
  Serial.print("LAT: ");
  Serial.println(lat, 6);
  Serial.print("LONG: ");
  Serial.println(lon, 6);
  Serial.print("SPEED: ");
  Serial.println(vel);
  Serial.print("SAT: ");
  Serial.println(sat);

  Serial.print("Date: ");
  Serial.printf("%02d",day);
  Serial.print("/");
  Serial.printf("%02d",month);
  Serial.print("/");
  Serial.println(year);

  Serial.print("Hour: ");
  Serial.printf("%02d",hour);
  Serial.print(":");
  Serial.printf("%02d",minute);
  Serial.print(":");
  Serial.printf("%02d\n",second);
  
  //led_to_send();    //pisca o led três vezes ao ler valores gnss
  
  if (count_blink_led <= 3 && flag_firts_on){ // para indicar que o gnss está funcionando normalmente
      led_to_send();    //pisca o led três vezes ao ler valores gnss
      count_blink_led++;
  }
}

void security_function() { 
  Serial.println("Sistema com mal funcionamento");
  Serial.println("Desligando/adormecendo todos os módulos");
  digitalWrite(status_sensors, LOW);
  digitalWrite(status_battery, LOW);
  digitalWrite(status_sensor_lora, LOW);
  gps_standby();
  esp_deep_sleep_start();
}
