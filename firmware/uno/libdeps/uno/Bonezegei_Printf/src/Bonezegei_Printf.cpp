/*
  This Library is written for board without Printf
  Author: Bonezegei (Jofel Batutay)
  Date: March 2024 
*/

#include "Bonezegei_Printf.h"

Bonezegei_Printf::Bonezegei_Printf() {
}

Bonezegei_Printf::Bonezegei_Printf(Stream *st) {
  s = st;
  for (int a = 0; a < PRINTF_MAX; a++) {
    text[a] = 0;
  }
  dummy(" ");
}


void Bonezegei_Printf::printf(const char *c, ...) {
  va_list ap;

  if (c != NULL) {
    va_start(ap, c);
    vsprintf(text, c, ap);
    va_end(ap);
    s->write(text);
  }
}

void Bonezegei_Printf::dummy(const char *c, ...) {
  va_list ap;

  if (c != NULL) {
    va_start(ap, c);
    vsprintf(text, c, ap);
    va_end(ap);
  }
}
