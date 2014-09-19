/*
 * gint.c:
 *
 * Copyright (c) 2003 Guralp Systems Limited
 * Author James McKenzie, contact <software@guralp.com>
 * All rights reserved.
 *
 */

static char rcsid[] = "$Id: gint.c,v 1.3 2004/06/29 15:56:22 root Exp $";

/*
 * $Log: gint.c,v $
 * Revision 1.3  2004/06/29 15:56:22  root
 * *** empty log message ***
 *
 * Revision 1.2  2004/06/29 14:18:11  root
 * *** empty log message ***
 *
 * Revision 1.1  2003/05/16 10:40:24  root
 * *** empty log message ***
 *
 * Revision 1.2  2003/05/14 16:31:44  root
 * #
 *
 * Revision 1.1  2003/05/13 15:09:59  root
 * #
 *
 */

/* Regardless of what you were taught in CS classes */
/* some processors/compilers do not do the right thing (TM)*/
/* when their integers overflow - this code however makes */
/* sure nothing overflows ever*/
/* */
/* It should probably check if the local machine is using */
/* network byte order but since suns are slow anyway its */
/* not going to help much */

/* here we define as without static or inline for */
/* people who want to link against this library */

#include "includes.h"

#include "gint.h"

int
G2uint8 (uint8_t * buf)
{
  return (int) *buf;
}

int
G2uint16 (uint8_t * buf)
{
  return (buf[0] << 8) | buf[1];
}

int
G2uint24 (uint8_t * buf)
{
  return (buf[0] << 16) | (buf[1] << 8) | buf[2];
}

int
G2luint24 (uint8_t * buf)
{
  return (buf[2] << 16) | (buf[1] << 8) | buf[0];
}

uint32_t
G2uint32 (uint8_t * buf)
{
  return (buf[0] << 24) | (buf[1] << 16) | (buf[2] << 8) | buf[3];
}

uint32_t
G2luint32 (uint8_t * buf)
{
  return (buf[3] << 24) | (buf[2] << 16) | (buf[1] << 8) | buf[0];
}

int
G2int8 (uint8_t * buf)
{
  if ((*buf) & 0x80)
    {
      return ((int) *buf) - 0x100;
    }
  return G2iuint8 (buf);
}

int
G2int16 (uint8_t * buf)
{
  if (buf[0] & 0x80)
    {
      return ((buf[0] << 8) | buf[1]) - 0x10000;
    }
  return G2iuint16 (buf);
}


int
G2int24 (uint8_t * buf)
{

  if (buf[0] & 0x80)
    {
      return ((buf[0] << 16) | (buf[1] << 8) | buf[2]) - 0x1000000;
    }
  return G2iuint24 (buf);
}

int
G2lint24 (uint8_t * buf)
{

  if (buf[2] & 0x80)
    {
      return ((buf[2] << 16) | (buf[1] << 8) | buf[0]) - 0x1000000;
    }
  return G2iluint24 (buf);
}

int
G2int32 (uint8_t * buf)
{
  int ret;
  if (buf[0] & 0x80)
    {
/*Bad voodoo below*/
      ret = ((buf[1] << 16) | (buf[2] << 8) | buf[3]) - 0x1000000;
      ret -= ((0xff - buf[0]) << 24);

      return ret;
    }
  return G2iuint32 (buf);
}

int
G2lint32 (uint8_t * buf)
{
  int ret;
  if (buf[3] & 0x80)
    {
/*Bad voodoo below*/
      ret = ((buf[2] << 16) | (buf[1] << 8) | buf[0]) - 0x1000000;
      ret -= ((0xff - buf[3]) << 24);

      return ret;
    }
  return G2iluint32 (buf);
}

static int
char_to_base36 (char c)
{
  if ((c >= '0') && (c <= '9'))
    return (int) c - (int) '0';
  if ((c >= 'a') && (c <= 'z'))
    return (int) c - (((int) 'a') - 10);
  if ((c >= 'A') && (c <= 'Z'))
    return (int) c - (((int) 'A') - 10);
  return 0;
}

static char
base36_to_char (int i)
{
  if (i < 10)
    return '0' + i;
  else
    return ('A' - 10) + i;
}

void
G2base36toa (uint32_t i, char *a)
{
  char c[7];
  int n = 6;

  c[6] = 0;

  while ((n--) && i)
    {
      c[n] = base36_to_char (i % 36);
      i = i / 36;
    }

  memcpy (a, c + n + 1, 6 - n);
}


uint32_t
G2atobase36 (char *a)
{
  uint32_t r = 0;

  while (*a)
    {
      r *= 36;
      r += char_to_base36 (*(a++));
    }

  return r;
}
