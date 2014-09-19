
/* The following defines are dependant on site specific Earthworm configuration,
 * These value are from a Earthworm v6.2 installation.
 */

#ifndef EWDEFS_H
#define EWDEFS_H


/**** From the earthworm_global.d of an operational system ****/

#define TYPE_HEARTBEAT 3
#define TYPE_TRACEBUF2 19
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
#define INBUFFERSIZE 100



/**** From transport.h ****/

typedef struct {             /******** description of message *********/
  unsigned char    type;     /* message is of this type               */
  unsigned char    mod;      /* was created by this module id         */
  unsigned char    instid;   /* at this installation                  */
} MSG_LOGO;


#endif
