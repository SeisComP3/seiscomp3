/*
 * The Spread Toolkit.
 *     
 * The contents of this file are subject to the Spread Open-Source
 * License, Version 1.0 (the ``License''); you may not use
 * this file except in compliance with the License.  You may obtain a
 * copy of the License at:
 *
 * http://www.spread.org/license/
 *
 * or in the file ``license.txt'' found in this distribution.
 *
 * Software distributed under the License is distributed on an AS IS basis, 
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License 
 * for the specific language governing rights and limitations under the 
 * License.
 *
 * The Creators of Spread are:
 *  Yair Amir, Michal Miskin-Amir, Jonathan Stanton, John Schultz.
 *
 *  Copyright (C) 1993-2013 Spread Concepts LLC <info@spreadconcepts.com>
 *
 *  All Rights Reserved.
 *
 * Major Contributor(s):
 * ---------------
 *    Ryan Caudy           rcaudy@gmail.com - contributions to process groups.
 *    Claudiu Danilov      claudiu@acm.org - scalable wide area support.
 *    Cristina Nita-Rotaru crisn@cs.purdue.edu - group communication security.
 *    Theo Schlossnagle    jesus@omniti.com - Perl, autoconf, old skiplist.
 *    Dan Schoenblum       dansch@cnds.jhu.edu - Java interface.
 *
 */


#ifndef INC_SCATTER
#define INC_SCATTER

#include "spu_system.h"
#define	MAX_SCATTER_ELEMENTS	10000

/* scat_element is EXACTLY as defined in iovec */

typedef struct dummy_scat_element{
	char	*buf;
	size_t	len;
} scat_element;

typedef struct dummy_scatter {
	size_t		num_elements;
	scat_element	elements[ARCH_SCATTER_SIZE];
} sys_scatter;

typedef struct dummy_big_scatter {
	size_t		num_elements;
	scat_element	elements[MAX_SCATTER_ELEMENTS];
} scatter;

#endif  /* INC_SCATTER */
