#include "batterystatus.h"

float Voltage; 
int Percentage;

void setup() {
  // Initialize serial and wait for port to open:
  Serial.begin(115200);
  // This delay gives the chance to wait for a Serial Monitor without blocking if none is found
}

void loop() {

  batterystatus(Voltage,Percentage);

  delay(500);
}
