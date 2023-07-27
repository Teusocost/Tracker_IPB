#include <TinyGPS++.h>
#include <HardwareSerial.h>
#define rxGPS 16
#define txGPS 17
 
long lat, lon;
HardwareSerial gpsSerial(1);


TinyGPSPlus gps;
 
void setup()
{
  Serial.begin(115200); // connect serial
  gpsSerial.begin(9600, SERIAL_8N1, rxGPS, txGPS); // connect gps sensor
}
 
void loop()
{
  while (gpsSerial.available())     // check for gps data
  {
    if (gps.encode(gpsSerial.read()))   // encode gps data
    {
      
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
  

      Serial.println("---------------------------");
      delay(4000);
    }
  }
}