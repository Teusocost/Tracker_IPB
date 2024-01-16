#include <Wire.h>
#include <Adafruit_Sensor.h> 
#include <Adafruit_ADXL345_U.h>

Adafruit_ADXL345_Unified accel = Adafruit_ADXL345_Unified();
float x,y,z;

int select_type(int X,int Y,int Z);

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
  Serial.print(z);
  Serial.print(" ");
  Serial.println(select_type(x,y,z));

   delay(100);
}
//function to define type

int select_type(int X,int Y,int Z){
  int type = 0;
  //top
if (X >= -3 && X < 3 &&
    Y >= -2.5 && Y < 4.5 &&
    Z >= 7 && Z < 11
) type = 1;
else
    if (X >= 4 && X < 12 &&
        Y >= -2 && Y < 6 &&
        Z >= -4 && Z < 7
    ) type = 2;
    else
        if (X >= 9 && X < 12 &&
            Y >= -3 && Y < 3 &&
            Z >= -3 && Z < 1
        ) type = 3;

        //left
        else
            if (X >= -1 && X < 2 &&
                Y >= 3 && Y < 8 &&
                Z >= 4 && Z < 9
            ) type = 4;
            else
                if (X >= -1 && X < 2 &&
                    Y >= 8 && Y < 12 &&
                    Z >= -6 && Z < 5
                ) type = 5;
                else
                    //behind
                    if (X >= -9 && X < -3 &&
                        Y >= -3 && Y < 3 &&
                        Z >= 4 && Z < 7
                    ) type = 6;
                    else
                        if (X >= -12 && X < 6 &&
                            Y >= -1 && Y < 3 &&
                            Z >= -2 && Z < 2
                        ) type = 7;
                        else
                            //Riht
                            if (X >= -1 && X < 3 &&
                                Y >= -6 && Y < 1 &&
                                Z >= 6 && Z < 12
                            ) type = 8;
                            else
                                if (X >= -2 && X < 2 &&
                                    Y >= -12 && Y < -6 &&
                                    Z >= 0 && Z < 6
                                ) type = 9;

return type;
}