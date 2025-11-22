/*
  This Library is written for board without Printf
  Author: Bonezegei (Jofel Batutay)
  Date: March 2024 
*/
#ifndef _BONEZEGEI_PRINTF_H_
#define _BONEZEGEI_PRINTF_H_

#include <Arduino.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define PRINTF_MAX 128

class Bonezegei_Printf {
public:
  Bonezegei_Printf();
  Bonezegei_Printf(Stream *st);

  void printf(const char *c, ...);
  void dummy(const char *c, ...);

  Stream *s;
  char text[PRINTF_MAX];  //define size to avoid crashing
};

#endif
