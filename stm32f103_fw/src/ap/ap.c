/*
 * ap.c
 *
 *  Created on: Dec 6, 2020
 *      Author: baram
 */


#include "ap.h"



cmd_t cmd;


void cliModem(cli_args_t *args);


void apInit(void)
{
  uartOpen(_DEF_UART1, 57600);  // USB
  uartOpen(_DEF_UART2, 57600);  // UART

  cliOpen(_DEF_UART1, 57600);

  cliAdd("modem", cliModem);
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

void cliModem(cli_args_t *args)
{
  bool ret = false;
  bool keep_loop;
  ymodem_t ymodem;


  if (args->argc == 2 && args->isStr(0, "down"))
  {
    uint32_t addr_offset;
    uint32_t addr;

    addr_offset = args->getData(1);

    keep_loop = true;

    ymodemOpen(&ymodem, _DEF_UART1);

    while(keep_loop)
    {
      if (ymodemReceive(&ymodem) == true)
      {
        switch(ymodem.type)
        {
          case YMODEM_TYPE_START:
            flashErase(addr_offset, ymodem.file_length);
            break;

          case YMODEM_TYPE_DATA:
            addr = addr_offset + ymodem.file_addr;
            flashWrite(addr, ymodem.file_buf, ymodem.file_buf_length);
            break;

          case YMODEM_TYPE_END:
            keep_loop = false;
            break;

          case YMODEM_TYPE_CANCEL:
            keep_loop = false;
            break;

          case YMODEM_TYPE_ERROR:
            keep_loop = false;
            break;
        }
      }
    }

    if (ymodem.type == YMODEM_TYPE_END)
    {
      cliPrintf("Down OK\n");
    }
    else
    {
      cliPrintf("Down Fail\n");
    }
    ret = true;
  }

  if (ret != true)
  {
    cliPrintf("modem down [addr] \n");
  }
}



