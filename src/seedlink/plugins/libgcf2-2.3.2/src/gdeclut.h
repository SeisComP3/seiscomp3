/*
 * gdeclut.h:
 *
 * Copyright (c) 2004 Guralp Systems Limited
 * Author James McKenzie, contact <software@guralp.com>
 * All rights reserved.
 *
 */

/*
 * $Id: gdeclut.h,v 1.3 2006/09/14 14:33:39 root Exp $
 */

/*
 * $Log: gdeclut.h,v $
 * Revision 1.3  2006/09/14 14:33:39  root
 * *** empty log message ***
 *
 * Revision 1.2  2004/11/18 12:44:43  root
 * *** empty log message ***
 *
 */

#ifndef __GDECLUT_H__
#define __GDECLUT_H__


struct G2DecLutEnt {
uint8_t ndecs;
uint8_t decs[8];
};

typedef struct {
struct G2DecLutEnt *lut;
int n;
} G2DecLut;
#endif /* __GDECLUT_H__ */
