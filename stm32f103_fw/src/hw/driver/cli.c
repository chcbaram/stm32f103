/*
 * cli.c
 *
 *  Created on: 2020. 12. 19.
 *      Author: baram
 */



#include "cli.h"
#include "uart.h"



#define CLI_KEY_BACK              0x08
#define CLI_KEY_DEL               0x7F
#define CLI_KEY_ENTER             0x0D
#define CLI_KEY_ESC               0x1B
#define CLI_KEY_LEFT              0x44
#define CLI_KEY_RIGHT             0x43
#define CLI_KEY_UP                0x41
#define CLI_KEY_DOWN              0x42
#define CLI_KEY_HOME              0x31
#define CLI_KEY_END               0x34

#define CLI_PROMPT_STR            "cli# "


enum
{
  CLI_RX_IDLE,
  CLI_RX_SP1,
  CLI_RX_SP2,
  CLI_RX_SP3,
  CLI_RX_SP4,
};



typedef struct
{
  uint8_t buf[CLI_LINE_BUF_MAX];
  uint8_t buf_len;
  uint8_t cursor;
  uint8_t count;
} cli_line_t;


typedef struct
{
  uint8_t  ch;
  uint32_t baud;
  bool     is_open;
  bool     is_log;
  uint8_t  log_ch;
  uint32_t log_baud;
  uint8_t  state;

  bool        hist_line_new;
  int8_t      hist_line_i;
  uint8_t     hist_line_last;
  uint8_t     hist_line_count;

  cli_line_t  line_buf[CLI_LINE_HIS_MAX];
  cli_line_t  line;
} cli_t;



cli_t   cli_node;



static bool cliUpdate(cli_t *p_cli, uint8_t rx_data);
static void cliLineClean(cli_t *p_cli);
static void cliLineAdd(cli_t *p_cli);
static void cliLineChange(cli_t *p_cli, int8_t key_up);
static void cliShowPrompt(cli_t *p_cli);




bool cliInit(void)
{
  cli_node.is_open = false;
  cli_node.is_log  = false;
  cli_node.state   = CLI_RX_IDLE;

  cli_node.hist_line_i     = 0;
  cli_node.hist_line_last  = 0;
  cli_node.hist_line_count = 0;
  cli_node.hist_line_new   = false;

  cliLineClean(&cli_node);

  return true;
}

bool cliOpen(uint8_t ch, uint32_t baud)
{
  cli_node.ch = ch;
  cli_node.baud = baud;

  cli_node.is_open = uartOpen(ch, baud);

  return cli_node.is_open;
}

bool cliOpenLog(uint8_t ch, uint32_t baud)
{
  bool ret;

  cli_node.log_ch = ch;
  cli_node.log_baud = baud;

  ret = uartOpen(ch, baud);

  if (ret == true)
  {
    cli_node.is_log = true;
  }
  return ret;
}

bool cliLogClose(void)
{
  cli_node.is_log = false;
  return true;
}

void cliShowLog(cli_t *p_cli)
{
  if (cli_node.is_log == true)
  {
    uartPrintf(p_cli->log_ch, "Cursor  : %d\n", p_cli->line.cursor);
    uartPrintf(p_cli->log_ch, "Count   : %d\n", p_cli->line.count);
    uartPrintf(p_cli->log_ch, "buf_len : %d\n", p_cli->line.buf_len);
    uartPrintf(p_cli->log_ch, "buf     : %s\n", p_cli->line.buf);
    uartPrintf(p_cli->log_ch, "line_i  : %d\n", p_cli->hist_line_i);
    uartPrintf(p_cli->log_ch, "line_lt : %d\n", p_cli->hist_line_last);
    uartPrintf(p_cli->log_ch, "line_c  : %d\n", p_cli->hist_line_count);

    for (int i=0; i<p_cli->hist_line_count; i++)
    {
      uartPrintf(p_cli->log_ch, "buf %d   : %s\n", i, p_cli->line_buf[i].buf);
    }
    uartPrintf(p_cli->log_ch, "\n");
  }
}

void cliShowPrompt(cli_t *p_cli)
{
  uartPrintf(p_cli->ch, "\n\r");
  uartPrintf(p_cli->ch, CLI_PROMPT_STR);
}

bool cliMain(void)
{
  if (cli_node.is_open != true)
  {
    return false;
  }

  if (uartAvailable(cli_node.ch) > 0)
  {
    cliUpdate(&cli_node, uartRead(cli_node.ch));
  }

  return true;
}

bool cliUpdate(cli_t *p_cli, uint8_t rx_data)
{
  bool ret = false;
  uint8_t tx_buf[8];
  cli_line_t *line;

  line = &p_cli->line;


  if (p_cli->state == CLI_RX_IDLE)
  {
    switch(rx_data)
    {
      // 엔터
      //
      case CLI_KEY_ENTER:
        if (line->count > 0)
        {
          cliLineAdd(p_cli);
        }

        line->count = 0;
        line->cursor = 0;
        line->buf[0] = 0;
        cliShowPrompt(p_cli);
        break;


      case CLI_KEY_ESC:
        p_cli->state = CLI_RX_SP1;
        break;


      // DEL
      //
      case CLI_KEY_DEL:
        if (line->cursor < line->count)
        {
          uint8_t mov_len;

          mov_len = line->count - line->cursor;
          for (int i=1; i<mov_len; i++)
          {
            line->buf[line->cursor + i - 1] = line->buf[line->cursor + i];
          }

          line->count--;
          line->buf[line->count] = 0;

          uartPrintf(p_cli->ch, "\x1B[1P");
        }
        break;


      // 백스페이스
      //
      case CLI_KEY_BACK:
        if (line->count > 0 && line->cursor > 0)
        {
          if (line->cursor == line->count)
          {
            line->count--;
            line->buf[line->count] = 0;
          }

          if (line->cursor < line->count)
          {
            uint8_t mov_len;

            mov_len = line->count - line->cursor;

            for (int i=0; i<mov_len; i++)
            {
              line->buf[line->cursor + i - 1] = line->buf[line->cursor + i];
            }

            line->count--;
            line->buf[line->count] = 0;
          }
        }

        if (line->cursor > 0)
        {
          line->cursor--;
          uartPrintf(p_cli->ch, "\b \b\x1B[1P");
        }
        break;


      default:
        if ((line->count + 1) < line->buf_len)
        {
          if (line->cursor == line->count)
          {
            uartWrite(p_cli->ch, &rx_data, 1);

            line->buf[line->cursor] = rx_data;
            line->count++;
            line->cursor++;
            line->buf[line->count] = 0;
          }
          if (line->cursor < line->count)
          {
            uint8_t mov_len;

            mov_len = line->count - line->cursor;
            for (int i=0; i<mov_len; i++)
            {
              line->buf[line->count - i] = line->buf[line->count - i - 1];
            }
            line->buf[line->cursor] = rx_data;
            line->count++;
            line->cursor++;
            line->buf[line->count] = 0;

            uartPrintf(p_cli->ch, "\x1B[4h%c\x1B[4l", rx_data);
          }
        }
        break;
    }
  }

  switch(p_cli->state)
  {
    case CLI_RX_SP1:
      p_cli->state = CLI_RX_SP2;
      break;

    case CLI_RX_SP2:
      p_cli->state = CLI_RX_SP3;
      break;

    case CLI_RX_SP3:
      p_cli->state = CLI_RX_IDLE;

      if (rx_data == CLI_KEY_LEFT)
      {
        if (line->cursor > 0)
        {
          line->cursor--;
          tx_buf[0] = 0x1B;
          tx_buf[1] = 0x5B;
          tx_buf[2] = rx_data;
          uartWrite(p_cli->ch, tx_buf, 3);
        }
      }

      if (rx_data == CLI_KEY_RIGHT)
      {
        if (line->cursor < line->buf_len)
        {
          line->cursor++;
        }
        tx_buf[0] = 0x1B;
        tx_buf[1] = 0x5B;
        tx_buf[2] = rx_data;
        uartWrite(p_cli->ch, tx_buf, 3);
      }

      if (rx_data == CLI_KEY_UP)
      {
        cliLineChange(p_cli, true);
        uartPrintf(p_cli->ch, (char *)p_cli->line.buf);
      }

      if (rx_data == CLI_KEY_DOWN)
      {
        cliLineChange(p_cli, false);
        uartPrintf(p_cli->ch, (char *)p_cli->line.buf);
      }

      if (rx_data == CLI_KEY_HOME)
      {
        uartPrintf(p_cli->ch, "\x1B[%dD", line->cursor);
        line->cursor = 0;

        p_cli->state = CLI_RX_SP4;
      }

      if (rx_data == CLI_KEY_END)
      {
        uint16_t mov_len;

        if (line->cursor < line->count)
        {
          mov_len = line->count - line->cursor;
          uartPrintf(p_cli->ch, "\x1B[%dC", mov_len);
        }
        if (line->cursor > line->count)
        {
          mov_len = line->cursor - line->count;
          uartPrintf(p_cli->ch, "\x1B[%dD", mov_len);
        }
        line->cursor = line->count;
        p_cli->state = CLI_RX_SP4;
      }
      break;

    case CLI_RX_SP4:
      p_cli->state = CLI_RX_IDLE;
      break;
  }



  cliShowLog(p_cli);

  return ret;
}

void cliLineClean(cli_t *p_cli)
{
  p_cli->line.count   = 0;
  p_cli->line.cursor  = 0;
  p_cli->line.buf_len = CLI_LINE_BUF_MAX - 1;
  p_cli->line.buf[0]  = 0;
}

void cliLineAdd(cli_t *p_cli)
{

  p_cli->line_buf[p_cli->hist_line_last] = p_cli->line;

  if (p_cli->hist_line_count < CLI_LINE_HIS_MAX)
  {
    p_cli->hist_line_count++;
  }

  p_cli->hist_line_i    = p_cli->hist_line_last;
  p_cli->hist_line_last = (p_cli->hist_line_last + 1) % CLI_LINE_HIS_MAX;
  p_cli->hist_line_new  = true;
}

void cliLineChange(cli_t *p_cli, int8_t key_up)
{
  uint8_t change_i;


  if (p_cli->hist_line_count == 0)
  {
    return;
  }


  if (p_cli->line.cursor > 0)
  {
    uartPrintf(p_cli->ch, "\x1B[%dD", p_cli->line.cursor);
  }
  if (p_cli->line.count > 0)
  {
    uartPrintf(p_cli->ch, "\x1B[%dP", p_cli->line.count);
  }


  if (key_up == true)
  {
    if (p_cli->hist_line_new == true)
    {
      p_cli->hist_line_i = p_cli->hist_line_last;
    }
    p_cli->hist_line_i = (p_cli->hist_line_i + p_cli->hist_line_count - 1) % p_cli->hist_line_count;
    change_i = p_cli->hist_line_i;
  }
  else
  {
    p_cli->hist_line_i = (p_cli->hist_line_i + 1) % p_cli->hist_line_count;
    change_i = p_cli->hist_line_i;
  }

  p_cli->line = p_cli->line_buf[change_i];
  p_cli->line.cursor = p_cli->line.count;

  p_cli->hist_line_new = false;
}


