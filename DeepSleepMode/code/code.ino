#include <U8x8lib.h> //biblioteca para controle do display oled
//Baixe a biblioteca em https://github.com/olikraus/u8glib

//construtor da instancia do controlador do display
//SDA = 21 e SCL = 22
U8X8_SSD1306_128X64_NONAME_SW_I2C display(SCL, SDA, U8X8_PIN_NONE);

//RTC_DATA_ATTR aloca a variável na memoria RTC
RTC_DATA_ATTR int bootCount = 0;

//sensibilidade para aceitação do toque
#define Threshold 40

//fator de conversão de microsegundos para segundos
#define uS_TO_S_FACTOR 1000000
//tempo que o ESP32 ficará em modo sleep (em segundos)
#define TIME_TO_SLEEP 3

void setup() {
     Serial.begin(115200);
     delay(1000); 
     //incrementa o numero de vezes que o BOOT ocorreu
     ++bootCount;
     configureDisplay();
     //chama a função para imprimir o motivo do BOOT
     print_wakeup_reason();
     //se o numero de boot for PAR configuramos o ESP32 para despertar através do botão (EXT0)
     if(bootCount % 2 == 0) {
   esp_sleep_enable_ext0_wakeup(GPIO_NUM_39,1); //1 = High, 0 = Low
     }
     //se for multiplo de 3 configuramos o ESP32 para despertar depois de um tempo definido
     else if(bootCount % 3 == 0) {
   esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);
     }
     //caso contrario configuramos os pinos de touch capacitivo para despertar o ESP32
     else {
          //Setup interrupt on Touch Pad 5 (GPIO12)
          touchAttachInterrupt(T5, callback, Threshold);
          //Configure Touchpad as wakeup source
          esp_sleep_enable_touchpad_wakeup();
     }
     Serial.println("entrando em modo sleep"); 
     esp_deep_sleep_start(); //força o ESP32 entrar em modo SLEEP
}

//nada a se fazer no loop
void loop() {
}

//callback das interrupções
void callback(){
//caso queira fazer algo ao ocorrer a interrupção
}

void configureDisplay()
{
     //inicializa o display e configura alguns parametros
     display.begin();
     display.setPowerSave(0); //modo powerSave (0-Off ? 1-On)
     display.setFont(u8x8_font_torussansbold8_u); //fonte utilizada

     //imprime no display os numero de vezes que aconteceu o BOOT
     display.drawString(0,0, "BOOT NUM:");
     display.drawString(0,2,String(bootCount).c_str());
     display.drawString(0,4, "MOTIVO:");
}

//função para imprimir a causa do ESP32 despertar
void print_wakeup_reason( ){
     esp_sleep_wakeup_cause_t wakeup_reason;
     String reason = "";

     wakeup_reason = esp_sleep_get_wakeup_cause(); //recupera a causa do despertar

     switch(wakeup_reason)
     {
          case 1 :reason = "EXT0 RTC_IO BTN"; break;
          case 2 :reason = "EXT1 RTC_CNTL";   break;
          case 3 :reason = "TIMER";           break;
          case 4 :reason = "TOUCHPAD";        break;
          case 5 :reason = "ULP PROGRAM";     break;
          default :reason = "NO DS CAUSE";    break;
     }
     Serial.println(reason);
     display.clearLine(6); //apaga a linha 6 do display
     display.drawString(0,6, reason.c_str()); //imprime a causa do despertar no display
     //se despertou por TOUCHPAD, então vamos verificar em qual dos pinos ocorreu
     if(wakeup_reason == 4) {
 print_wakeup_touchpad(); //verifica o pino e imprime no display
     } 
}

//função para imprimir o pino que foi tocado
void print_wakeup_touchpad() {
     touch_pad_t touchPin;
     touchPin = esp_sleep_get_touchpad_wakeup_status(); //recupera o GPIO que despertou o ESP32
     String GPIO = "";

     switch(touchPin) 
     {
   case 0 : GPIO = "4";  break;
         case 1 : GPIO = "0";  break;
         case 2 : GPIO = "2";  break;
         case 3 : GPIO = "15"; break;
         case 4 : GPIO = "13"; break;
         case 5 : GPIO = "12"; break;
         case 6 : GPIO = "14"; break;
         case 7 : GPIO = "27"; break;
         case 8 : GPIO = "33"; break;
         case 9 : GPIO = "32"; break;
         default : Serial.println("Wakeup not by touchpad"); break;
     }
     Serial.println("GPIO: "+GPIO);
     display.clearLine(7);//apaga a linha 7 do display
     display.drawString(0,7, "GPIO: ");
     display.drawString(6,7, GPIO.c_str()); //imprime o GPIO 
}
