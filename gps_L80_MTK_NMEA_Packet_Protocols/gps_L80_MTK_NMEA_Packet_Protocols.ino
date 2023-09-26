#include <TinyGPS++.h>
#include <HardwareSerial.h>
#define rxGPS 16
#define txGPS 17
 
long lat, lon;
HardwareSerial gpsSerial(2);

int cont=1;
TinyGPSPlus gps;
 
void setup()
{
  Serial.begin(115200); // connect serial
  gpsSerial.begin(9600, SERIAL_8N1, rxGPS, txGPS); // connect gps sensor
  Serial.println("acordando gps em modo HOT");
  gpsSerial.print("$PMTK101*32<CR><LF>\r\n"); //acorda o gps em modo hot
}
 
void loop()
{
  while (gpsSerial.available())     // check for gps data
  {
    if (gps.encode(gpsSerial.read())){   // encode gps data
      //read_all_values();// -> valores ja oferecidos pelo outro code
      Serial.println(gpsSerial.read());
      read_all_values();
      Serial.println("---------------------------");
      cont++;
      delay(5000);
      if (cont >= 5){
        Serial.println("colocando o modulo em standby (30 seg)");
        gpsSerial.print("$PMTK161,0*28<CR><LF>\r\n");
        delay(30000);
        Serial.println("acordando gps em modo HOT");
        gpsSerial.print("$PMTK101*32<CR><LF>\r\n"); //acorda o gps em modo hot
        cont = 1;      
      }
    }
  }
}

void read_all_values(){
        Serial.print(gps.encode(gpsSerial.read()));
      Serial.print("SATS: ");
      Serial.println(gps.satellites.value());
      Serial.print("LAT: ");
      Serial.println(gps.location.lat(), 6);
      Serial.print("LONG: ");
      Serial.println(gps.location.lng(), 6);
      Serial.print("ALT: ");
      Serial.println(gps.altitude.meters());
      Serial.print("SPEED: ");
      Serial.println(gps.speed.mps());
 
      Serial.print("Date: ");
      Serial.print(gps.date.day()); Serial.print("/");
      Serial.print(gps.date.month()); Serial.print("/");
      Serial.println(gps.date.year());
 
      Serial.print("Hour: ");
      Serial.print(gps.time.hour()); Serial.print(":");
      Serial.print(gps.time.minute()); Serial.print(":");
      Serial.println(gps.time.second());
  
}