/*
 * ap.c
 *
 *  Created on: Dec 6, 2020
 *      Author: baram
 */


#include "ap.h"



cmd_t cmd;



void apInit(void)
{
  uartOpen(_DEF_UART1, 57600);  // USB
  uartOpen(_DEF_UART2, 57600);  // UART

  cliOpen(_DEF_UART2, 57600);

  cmdInit(&cmd);
  cmdOpen(&cmd, _DEF_UART1, 57600);
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
      //ledToggle(_DEF_LED1);
    }
    cliMain();

    if (cmdReceivePacket(&cmd) == true)
    {
      cliPrintf("cmd    0x%X\n", cmd.rx_packet.cmd);
      cliPrintf("dir    0x%X\n", cmd.rx_packet.dir);
      cliPrintf("error  0x%X\n", cmd.rx_packet.error);
      cliPrintf("length 0x%X\n", cmd.rx_packet.length);

      for (int i=0; i<cmd.rx_packet.length; i++)
      {
        cliPrintf("data   %02d : 0x%X\n", i, cmd.rx_packet.data[i]);
      }
      cliPrintf("\n");

      if (cmd.rx_packet.cmd == 0x10)
      {
        if (cmd.rx_packet.data[0] == 1)
        {
          ledOn(_DEF_LED1);
        }
        else
        {
          ledOff(_DEF_LED1);
        }
      }
    }
  }
}


