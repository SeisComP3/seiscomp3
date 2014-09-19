/*
 * gint.h:
 *
 * Copyright (c) 2003 Guralp Systems Limited
 * Author James McKenzie, contact <software@guralp.com>
 * All rights reserved.
 *
 */

/*
 * $Id: gint.h,v 1.4 2004/06/29 15:56:22 root Exp $
 */

/*
 * $Log: gint.h,v $
 * Revision 1.4  2004/06/29 15:56:22  root
 * *** empty log message ***
 *
 * Revision 1.3  2004/06/29 14:18:11  root
 * *** empty log message ***
 *
 * Revision 1.2  2003/06/20 12:17:59  root
 * *** empty log message ***
 *
 * Revision 1.1  2003/05/16 10:40:24  root
 * *** empty log message ***
 *
 * Revision 1.2  2003/05/14 16:31:49  root
 * #
 *
 * Revision 1.1  2003/05/13 15:10:07  root
 * #
 *
 */

#ifndef __GINT_H__
#define __GINT_H__

/* Regardless of what you were taught in CS classes */
/* some processors/compilers do not do the right thing (TM)*/
/* when their integers overflow - this code however makes */
/* sure nothing overflows ever*/
/* */
/* It should probably check if the local machine is using */
/* network byte order but since suns are slow anyway its */
/* not going to help much */

/* define all of these as static inline here and then */
/* redo them all in the .c file, that way we can use */
/* the inline ones and the compiler does need to do */
/* anything clever */

static inline int
G2iuint8 (uint8_t * buf)
{
  return (int) *buf;
}

static inline int
G2iuint16 (uint8_t * buf)
{
  return (buf[0] << 8) | buf[1];
}

static inline int
G2iuint24 (uint8_t * buf)
{
  return (buf[0] << 16) | (buf[1] << 8) | buf[2];
}


static inline int
G2iluint24 (uint8_t * buf)
{
  return (buf[2] << 16) | (buf[1] << 8) | buf[0];
}

static inline uint32_t
G2iuint32 (uint8_t * buf)
{
  return (buf[0] << 24) | (buf[1] << 16) | (buf[2] << 8) | buf[3];
}

static inline uint32_t
G2iluint32 (uint8_t * buf)
{
  return (buf[3] << 24) | (buf[2] << 16) | (buf[1] << 8) | buf[0];
}

static inline int
G2iint8 (uint8_t * buf)
{
  if ((*buf) & 0x80)
    {
      return ((int) *buf) - 0x100;
    }
  return G2iuint8 (buf);
}

static inline int
G2iint16 (uint8_t * buf)
{
  if (buf[0] & 0x80)
    {
      return ((buf[0] << 8) | buf[1]) - 0x10000;
    }
  return G2iuint16 (buf);
}


static inline int
G2iint24 (uint8_t * buf)
{

  if (buf[0] & 0x80)
    {
      return ((buf[0] << 16) | (buf[1] << 8) | buf[2]) - 0x1000000;
    }
  return G2iuint24 (buf);
}


static inline int
G2ilint24 (uint8_t * buf)
{

  if (buf[2] & 0x80)
    {
      return ((buf[2] << 16) | (buf[1] << 8) | buf[0]) - 0x1000000;
    }
  return G2iluint24 (buf);
}

static inline int
G2iint32 (uint8_t * buf)
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


static inline int
G2ilint32 (uint8_t * buf)
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

extern int G2uint8 (uint8_t * buf);
extern int G2uint16 (uint8_t * buf);
extern int G2luint24 (uint8_t * buf);
extern int G2uint24 (uint8_t * buf);
extern uint32_t G2uint32 (uint8_t * buf);
extern uint32_t G2luint32 (uint8_t * buf);
extern int G2int8 (uint8_t * buf);
extern int G2int16 (uint8_t * buf);
extern int G2int24 (uint8_t * buf);
extern int G2lint24 (uint8_t * buf);
extern int G2int32 (uint8_t * buf);
extern int G2lint32 (uint8_t * buf);
extern void G2base36toa(uint32_t i, char *a);
extern uint32_t G2atobase36(char *a);

#endif /* __GINT_H__ */
