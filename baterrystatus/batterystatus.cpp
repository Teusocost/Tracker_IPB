#include <Arduino.h>
#include "batterystatus.h"

#define analogInPin 33 // porta que vai ler o nível da bateria

int sensorValue;
float calibration = 0.44; // Check Battery voltage using multimeter & add/subtract the value


float mapfloat(float x, float in_min, float in_max, float out_min, float out_max)
{
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

void batterystatus(float &Voltage, int &Percentage)
{
    sensorValue = analogRead(analogInPin);
    Voltage = (((sensorValue * 3.3) / 4096) * 2 + calibration); // multiply by two as voltage divider network is 100K & 100K Resistor

    Percentage = mapfloat(Voltage, 2.65, 4.2, 0, 100); // 2.8V as Battery Cut off Voltage & 4.2V as Maximum Voltage

    if (Percentage >= 100)
    {
        Percentage = 100;
    }
    if (Percentage <= 0)
    {
        Percentage = 1;
    }
  Serial.print("\t Output Voltage = ");
  Serial.print(Voltage);
  Serial.print("\t Battery Percentage = ");
  Serial.println(Percentage);
    
}
