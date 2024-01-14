#include <Wire.h>
#include <Adafruit_Sensor.h> 
#include <Adafruit_ADXL345_U.h>

Adafruit_ADXL345_Unified accel = Adafruit_ADXL345_Unified();
float x,y,z;
void setup(void) 
{
   Serial.begin(115200);  
   if(!accel.begin())
   {
      Serial.println("No ADXL345 sensor detected.");
      while(1);
   }
}
void loop(void) 
{
   sensors_event_t event;
   accel.getEvent(&event);
   x = event.acceleration.x;
   y = event.acceleration.y;
   z = event.acceleration.z;
   
  Serial.print(x);
  Serial.print(" ");
  Serial.print(y);
  Serial.print(" ");
  Serial.println(z);

   delay(100);
}