/*
 * hw.c
 *
 *  Created on: Dec 6, 2020
 *      Author: baram
 */


#include "hw.h"





void hwInit(void)
{
  bspInit();
  rtcInit();

  cliInit();
  resetInit();
  ledInit();
  usbInit();
  uartInit();
  flashInit();
}
