/*
 * reset.c
 *
 *  Created on: 2020. 12. 9.
 *      Author: baram
 */


#include "reset.h"
#include "rtc.h"


static uint32_t reset_count = 0;



bool resetInit(void)
{
  bool ret = true;


  // 만약 Reset 핀이 눌렸다면
  //
  if (RCC->CSR == (1<<26))
  {
    rtcBackupRegWrite(1, rtcBackupRegRead(1) + 1);
    delay(500);
    reset_count = rtcBackupRegRead(1);
  }

  rtcBackupRegWrite(1, 0);


  if (reset_count != 2)
  {
    void (**jump_func)() = (void (**)())(0x8005000 + 4);

    if ((uint32_t)(*jump_func) != 0xFFFFFFFF)
    {
      HAL_RCC_DeInit();
      HAL_DeInit();

      for (int i=0; i<8; i++)
      {
        NVIC->ICER[i] = 0xFFFFFFFF;
        __DSB();
        __ISB();
      }
      SysTick->CTRL = 0;

      (*jump_func)();
    }
  }


  return ret;
}

uint32_t resetGetCount(void)
{
  return reset_count;
}
