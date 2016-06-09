#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <atom.h>
#include <atomtimer.h>

#include "spidrv.hpp"
#include "debugshell.hpp"

#include "dumpcode.h"

static char buf[MAX_CMD_BUF_SIZE];
static char cmdbuf[3][MAX_CMD_BUF_SIZE];

static SPIdrv spidrv;

DebugShell::DebugShell()
{
  this -> cursor = 0;
}

DebugShell::~DebugShell()
{
  //
}

int DebugShell::read_cmd()
{
  char tmp;
  this -> cursor = 0;

  for(;;)
    {
      tmp = getchar();

      //Echo back.
      if(0x0D == tmp)
        {
          putchar('\n');
        }
      else if(0x08 == tmp)
        {
          if(this -> cursor > 0)
            {
              putchar(tmp);
            }
        }
      else
        {
          putchar(tmp);
        }

      if(0x08 == tmp) //Backspace
        {
          if(this -> cursor > 0)
            {
              this -> cursor--;
              putchar(' ');
              putchar(0x08);
            }

          continue;
        }
          

      //Isn't it overflowed and tmp has CR?
      if(MAX_CMD_BUF_SIZE > this -> cursor && 0x0D == tmp)
        {
          buf[this -> cursor] = 0x00;
          return 0;
        }
      else if(0x0D == tmp) //command ovf.
        {
          return -1;
        }

      //Isn't it overflowed?
      if(MAX_CMD_BUF_SIZE - 1 > this -> cursor)
        {
          buf[this -> cursor] = tmp;
        }
      else //Overflowed. end it with 0x00.
        {
          buf[this -> cursor] = 0x00;
        }

      this -> cursor++;
    }

  return 0;
}

int DebugShell::eval_cmd()
{
  int ret;

  ret = sscanf(buf, "%s", cmdbuf[0]);
  if(ret < 0)
    {
      return -1;
    }

  if(strncmp(cmdbuf[0], "help", MAX_CMD_BUF_SIZE) == 0)
    {
      cmd_help();
    }
  else if(strncmp(cmdbuf[0], "spidump", MAX_CMD_BUF_SIZE) == 0)
    {
      cmd_spidump();
    }
  else if(strncmp(cmdbuf[0], "dumpmem", MAX_CMD_BUF_SIZE) == 0)
    {
      cmd_dumpmem();
    }
  else if(strncmp(cmdbuf[0], "setmem", MAX_CMD_BUF_SIZE) == 0)
    {
      cmd_setmem();
    }
  else
    {
      printf_P(PSTR("Command not found: %s\n"), buf);
    }

  return 0;
}

int DebugShell::cmd_help()
{
  printf_P(PSTR("iPot debug shell\n\n"));
  printf_P(PSTR("help - print this help message.\n"));
  printf_P(PSTR("spidump - dump SPI shm using dumpcode()\n"));
  printf_P(PSTR("dumpmem [hex addr] [dec size] - dump values at given address.\n"));
  printf_P(PSTR("setmem  [hex addr] [hex value] - set memory at given address with given value.\n"));
  printf_P(PSTR("\n"));

  printf_P(PSTR("WARN!! WARN!! WARN!!\n"));
  printf_P(PSTR("dumpmem and setmem can cause undesired behavior.\n"));
  printf_P(PSTR("Use it with care.\n\n"));

  return 0;
}

int DebugShell::cmd_spidump()
{
  printf_P(PSTR("SPI shm dump:\n"));
  dumpcode(spidrv.get_spi_mem(), MAX_SPIMEM_SIZE);

  return 0;
}

int DebugShell::cmd_dumpmem()
{
  int ret;
  void *addr;
  int size;

  ret = sscanf(buf, "%s %s %s", cmdbuf[0], cmdbuf[1], cmdbuf[2]);
  if(ret < 0)
    {
      printf_P("Error while parsing command.\n");
      printf_P("Use help command to get help.\n");
      return -1;
    }

  if(strncmp(cmdbuf[1], "0x", 2) != 0)
    {
      printf_P(PSTR("Address must starts with 0x.\n"));
      return -1;
    }

  ret = sscanf(cmdbuf[1] + 2, "%x", (unsigned int*)&addr);
  if(ret < 0)
    {
      printf_P(PSTR("Invalid address format.\n"));
      return -1;
    }

  ret = sscanf(cmdbuf[2], "%d", &size);
  if(ret < 0)
    {
      printf_P(PSTR("Invalid size format.\n"));
      return -1;
    }

  dumpcode(addr, size);
  return 0;
}

int DebugShell::cmd_setmem()
{
  int ret;
  void *addr;
  int size;

  union omnidata_u {
    uint32_t uint32;
    uint16_t uint16;
    uint8_t uint8;
  } omnidata;

  ret = sscanf(buf, "%s %s %s", cmdbuf[0], cmdbuf[1], cmdbuf[2]);
  if(ret < 0)
    {
      printf_P("Error while parsing command.\n");
      printf_P("Use help command to get help.\n");
      return -1;
    }

  if(strncmp(cmdbuf[1], "0x", 2) != 0)
    {
      printf_P(PSTR("Address must starts with 0x.\n"));
      return -1;
    }

  ret = sscanf(cmdbuf[1] + 2, "%x", (unsigned int*)&addr);
  if(ret < 0)
    {
      printf_P(PSTR("Invalid address format.\n"));
      return -1;
    }

  if(strncmp(cmdbuf[2], "0x", 2) != 0)
    {
      printf_P(PSTR("Data value must starts with 0x.\n"));
      return -1;
    }

  ret = sscanf(cmdbuf[2] + 2, "%lx", &omnidata.uint32);
  if(ret < 0)
    {
      printf_P(PSTR("Invalid address format.\n"));
      return -1;
    }

  size = strlen(cmdbuf[2] + 2);

  if(size <= 2)
    {
      *((uint8_t*)addr) = omnidata.uint8;
    }
  else if(size <= 4)
    {
      *((uint16_t*)addr) = omnidata.uint16;
    }
  else if(size <= 8)
    {
      *((uint32_t*)addr) = omnidata.uint32;
    }
  else
    {
      printf_P(PSTR("Invalid value length.\n"));
      return -1;
    }

  return 0;
}

int DebugShell::init()
{
  printf_P(PSTR("iPot debug shell.\n"));
  printf_P(PSTR("Use \"help\" command for its usage.\n"));
  return 0;
}

int DebugShell::do_repl()
{
  printf_P(PSTR("iPot # "));
  read_cmd();
  eval_cmd();
  return 0;
}
