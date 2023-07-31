#include <SoftwareSerial.h>

String incomingString;

SoftwareSerial lora(16,17);
  bool status = 1;
void setup()
{
  pinMode(2,OUTPUT);
  Serial.begin(115200);
  lora.begin(115200);
  lora.setTimeout(500);
  lora.println("AT+RESET");
  Serial.println(lora.readString());
  delay(1000);
  lora.println("AT+BAND=868000000"); // AT+SEND=1,2,LO
  Serial.println(lora.readString());
  delay(1000);
  lora.println("AT+ADDRESS=2"); // AT+SEND=1,2,LO
  Serial.println(lora.readString());
  delay(1000);
  lora.println("AT+NETWORKID=18"); // AT+SEND=1,2,LO
  Serial.println(lora.readString());
  delay(1000);

}

void loop()
{
  if (lora.available()) {

    incomingString = lora.readString();
    Serial.println(incomingString);

    char dataArray[20]; 
    incomingString.toCharArray(dataArray,20);
    char* data = strtok(dataArray, ",");
    data = strtok(NULL, ",");
    data = strtok(NULL, ",");
    Serial.println(incomingString);
    Serial.println(data);
    if (strcmp(data,"123456789") == 0){
    digitalWrite(2,status);
    Serial.println("deu bom");
    status = !status;
    }
    //if (strcmp(data,"LO") == 0) {
    //}
    }
  }