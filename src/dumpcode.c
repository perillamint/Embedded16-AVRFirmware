#include <stdio.h>
#include <avr/pgmspace.h>

int isprintable(unsigned char c)
{
  if(0x20 <= c && 0x7E >=c)
    return 1;
  return 0;
}

void printchar(unsigned char c)
{
  if(isprintable(c))
    printf_P(PSTR("%c"),c);
  else
    printf_P(PSTR("."));
}

void dumpcode(unsigned char *buff, int len)
{
  int i;
  printf_P(PSTR("----------BEGIN DUMP----------\n"));
   
  for(i=0;i<len;i++)
    {
      if(i%16==0)
        printf_P(PSTR("0x%08X  "),(int)&buff[i]);
      printf_P(PSTR("%02X "),buff[i]);
      if(i%16-15==0)
		{
          int j;
          printf_P(PSTR("  "));
          for(j=i-15;j<=i;j++)
			printchar(buff[j]);
          printf_P(PSTR("\n"));
		}
    }
  if(i%16!=0)
    {
      int j;
      int spaces=(len-i+16-i%16)*3+2;
      for(j=0;j<spaces;j++)
        printf_P(PSTR(" "));
      for(j=i-i%16;j<len;j++)
        printchar(buff[j]);
    }
  printf_P(PSTR("\n"));
  printf_P(PSTR("---------END DUMP----------\n"));
} 
