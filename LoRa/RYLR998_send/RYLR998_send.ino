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
    //lora.println("AT+SEND=" + lora_RX_address + ",2,LO"); // AT+SEND=1,2,LO
    lora.println("AT+SEND=1,9,123456789");
    Serial.println(lora.readString());
    //Serial.println("enviado LO");
    delay(2000);
    //lora.println("AT+SEND=" + lora_RX_address + ",2,HI"); // AT+SEND=1,2,HI
    //Serial.println(lora.readString());
    //Serial.println("enviado HI");
    //delay(2000);
}