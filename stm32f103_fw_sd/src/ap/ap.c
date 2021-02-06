/*
 * ap.c
 *
 *  Created on: Dec 6, 2020
 *      Author: baram
 */


#include "ap.h"





void apInit(void)
{
  uartOpen(_DEF_UART1, 57600);  // USB
  uartOpen(_DEF_UART2, 57600);  // UART

  cliOpen(_DEF_UART1, 57600);
}

void apMain(void)
{
  uint32_t pre_time;


  pre_time = millis();
  while(1)
  {
    if (millis()-pre_time >= 500)
    {
      pre_time = millis();
      ledToggle(_DEF_LED1);
    }
    cliMain();
  }
}

