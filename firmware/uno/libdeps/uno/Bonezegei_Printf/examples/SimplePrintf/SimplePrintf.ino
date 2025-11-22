/*
  This Library is written for board without Printf
  Author: Bonezegei (Jofel Batutay)
  Date: March 2024 
  Simple Printf
*/

#include <Bonezegei_Printf.h>

//param Stream
Bonezegei_Printf debug(&Serial);

void setup() {
  Serial.begin(9600);
  delay(1000);
  debug.printf("\nHello World\n");
  debug.printf("String: %s \n", "this is the string");
  debug.printf("Integer: %d %u\n", 1234, 5678);
  debug.printf("Character: %c\n", 'C');
}

void loop() {
  debug.printf("millis: %u \n", millis() );
  delay(100);
}
