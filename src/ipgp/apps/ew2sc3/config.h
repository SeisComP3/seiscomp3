/************************************************************************
 *                                                                      *
 * Copyright (C) 2012 OVSM/IPGP                                         *
 *                                                                      *
 * This program is free software: you can redistribute it and/or modify *
 * it under the terms of the GNU General Public License as published by *
 * the Free Software Foundation, either version 3 of the License, or    *
 * (at your option) any later version.                                  *
 *                                                                      *
 * This program is distributed in the hope that it will be useful,      *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of       *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the        *
 * GNU General Public License for more details.                         *
 *                                                                      *
 * This program is part of 'Projet TSUAREG - INTERREG IV Caraïbes'.     *
 * It has been co-financed by the European Union and le Ministère de    *
 * l'Ecologie, du Développement Durable, des Transports et du Logement. *
 *                                                                      *
 ************************************************************************/



#define TYPE_HEARTBEAT 3
#define TYPE_TRACEBUF  20

/**** From imp_exp_gen.h ****/
#define MAX_ALIVE_STR  256

#define STX 2     /* Start Transmission: used to frame beginning of message */
#define ETX 3     /* End Transmission: used to frame end of message */
#define ESC 27    /* Escape: used to 'cloak' unfortunate binary bit patterns which look like sacred characters */

/* Define States for Socket Message Receival */
#define SEARCHING_FOR_MESSAGE_START 0
#define EXPECTING_MESSAGE_START 1
#define ASSEMBLING_MESSAGE 2

/* Define Buffer Size for Socket Receiving Buffer */
#define INBUFFERSIZE 4096

/**** From trace_buf.h ****/

#define TRACE_STA_LEN   7
#define TRACE_CHAN_LEN  9
#define TRACE_NET_LEN   9
#define TRACE_LOC_LEN   3

