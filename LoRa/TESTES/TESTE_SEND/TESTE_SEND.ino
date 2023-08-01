#include <SoftwareSerial.h>

SoftwareSerial lora(16, 17); //RX TX

String lora_RX_address = "2";   //enter Lora RX address
void setup()
{
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

void loop()
{        
    double lat = -41.123456;
    double lon = 6.123456;
    int sat = 7;
    int vel = 2;
    float temp = 42.22;
    float hum = 53.44;
    float x = 10;
    float y = 9;
    float z = 8;
    float Bat_Volt = 4.1;
    float bat = 80;
  
    char mensagem[70];
    sprintf(mensagem, "AT+SEND=2,47,%.6f/%.6f/%i/%i/%.2f/%.2f/%.0f/%.0f/%.0f/%.0f/%.0f",lat, lon, sat, vel, temp, hum, x, y, z, Bat_Volt, bat );

    lora.println(mensagem);
    //delay(200);
    Serial.println(lora.readString());
    Serial.println(mensagem);
    //lora.println("AT+SEND=" + lora_RX_address + ",2,LO"); // AT+SEND=1,2,LO
    //lora.println("AT+SEND=2," + strlen(lat) + "," + lat + "\r\n");
    
    //Serial.println("enviado LO");
    delay(500);
    //lora.println("AT+SEND=" + lora_RX_address + ",2,HI"); // AT+SEND=1,2,HI
    //Serial.println(lora.readString());
    //Serial.println("enviado HI");
    //delay(2000);
}