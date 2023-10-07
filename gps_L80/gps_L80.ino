#include <TinyGPS++.h>
#include <HardwareSerial.h>
#define rxGPS 16
#define txGPS 17
 
//long lat, lon;
HardwareSerial gpsSerial(2);


TinyGPSPlus gps;
double lat = 0, lon = 0;
int sat = 0, vel = 0, year = 0, month = 0, day = 0, hour = 0, minute = 0, second = 0;
char keep[100];
int requiredBufferSize;
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
      read_all_data_gps();
      Serial.println("---------------------------");
      delay(4000);
    }
  }
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

  Serial.print("tamanho de data: ");
  Serial.println(snprintf(NULL, 0, "%i",year));
    Serial.print("tamanho de month: ");
  Serial.println(snprintf(NULL, 0, "%i",month));
    Serial.print("tamanho de day: ");
  Serial.println(snprintf(NULL, 0, "%i",day));
    Serial.print("tamanho de hour: ");
  Serial.println(snprintf(NULL, 0, "%i",hour));
    Serial.print("tamanho de minute: ");
  Serial.println(snprintf(NULL, 0, "%i",minute));
    Serial.print("tamanho de second: ");
  Serial.println(snprintf(NULL, 0, "%i",second));

  //sprintf(keep, "%s%i-%02i-%02iT%02i:%02i:%02iZK\n",year,month,day,hour,minute,second);
  //Serial.println(keep);
  //requiredBufferSize = snprintf(NULL, 0, "%s",keep); //calcula tamanho string
  //Serial.println(requiredBufferSize);
}