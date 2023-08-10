#include <HardwareSerial.h>
#define rxGPS 25
#define txGPS 26

HardwareSerial lora(1);
String incomingString;

void setup()
{
  Serial.begin(115200); // connect serial
  lora.begin(115200, SERIAL_8N1, rxGPS, txGPS); // connect gps sensor
}

void loop()
{
  if (lora.available()) {

    incomingString = lora.readString();
    Serial.println(incomingString);
   
    char dataArray[70]; 
    incomingString.toCharArray(dataArray,70);
    char* data = strtok(dataArray, ",");
    data = strtok(NULL, ",");
    data = strtok(NULL, ",");
    
    //Serial.println(incomingString);

    Serial.println(data);

    if (strcmp(data,"123456789") == 0){
    //digitalWrite(2,status);
    Serial.println("deu bom");
    //status = !status;
    }
    //if (strcmp(data,"LO") == 0) {
    //}
    }
  }