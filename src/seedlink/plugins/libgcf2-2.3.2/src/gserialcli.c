/*
 * gserialcli.c:
 *
 * Copyright (c) 2004 Guralp Systems Limited
 * Author James McKenzie, contact <software@guralp.com>
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

static char rcsid[] =
  "$Id: gserialcli.c,v 1.19 2005/07/29 16:13:03 root Exp $";

/*
 * $Log: gserialcli.c,v $
 * Revision 1.19  2005/07/29 16:13:03  root
 * *** empty log message ***
 *
 * Revision 1.18  2005/07/29 16:09:39  root
 * *** empty log message ***
 *
 * Revision 1.17  2005/03/01 10:41:42  root
 * *** empty log message ***
 *
 * Revision 1.16  2004/11/09 09:35:36  root
 * *** empty log message ***
 *
 * Revision 1.15  2004/10/29 10:28:51  root
 * *** empty log message ***
 *
 * Revision 1.14  2004/05/19 15:54:10  root
 * *** empty log message ***
 *
 * Revision 1.13  2004/05/18 17:19:21  root
 * *** empty log message ***
 *
 * Revision 1.12  2004/04/29 14:20:43  root
 * *** empty log message ***
 *
 * Revision 1.11  2004/04/23 00:47:18  root
 * *** empty log message ***
 *
 * Revision 1.10  2004/04/23 00:21:24  root
 * *** empty log message ***
 *
 * Revision 1.9  2004/04/21 09:34:18  root
 * *** empty log message ***
 *
 * Revision 1.8  2004/04/15 22:35:45  root
 * *** empty log message ***
 *
 * Revision 1.7  2004/04/15 11:08:18  root
 * *** empty log message ***
 *
 * Revision 1.6  2004/04/14 15:39:52  root
 * *** empty log message ***
 *
 * Revision 1.4  2004/03/18 19:14:43  root
 * *** empty log message ***
 *
 * Revision 1.3  2004/03/18 19:13:36  root
 * *** empty log message ***
 *
 * Revision 1.2  2004/02/13 16:33:14  root
 * *** empty log message ***
 *
 * Revision 1.1  2004/02/13 15:42:03  root
 * *** empty log message ***
 *
 */

#include "includes.h"
#include "gserialcli.h"

#define INTR_TIMEOUT 5
#define SELECT_TIMEOUT 2
#define NEXT(a) (((a)+1) & (G2SERIALCLI_LLL-1))
#define PREV(a) (((a)+(G2SERIALCLI_LLL-1)) & (G2SERIALCLI_LLL-1))


static void
block (G2Serial * s, G2SerBlock * p)
{
  G2SerialCLI *c = (G2SerialCLI *) s->client_data;

  G2SerNackIntr (s, p, p->seq);
}


const static uint8_t cmd_rec[] = { 'd', 'n', 'a', 'm', 'm', 'o', 'C' };

static void
extract_banner (G2SerialCLI * c)
{
  int r = PREV (c->llwptr);
  int sm = 0;
  int len = 0;
  int b, e;

  while (r != c->llwptr)
    {

      if (c->lastline[r] != cmd_rec[sm])
        sm = 0;
      if (c->lastline[r] == cmd_rec[sm])
        sm++;
      if (sm == sizeof (cmd_rec))
        break;
      r = PREV (r);
    }

  if (r == c->llwptr)
    return;

  b = r;
  while (b != c->llwptr)
    {
      if (c->lastline[b] == '\n')
        break;
      len++;
      b = PREV (b);
    }

  b = NEXT (b);


  e = r;
  while (e != c->llwptr)
    {
      if (c->lastline[e] == '\n')
        break;
      len++;
      e = NEXT (e);
    }
  if (e != c->llwptr)
    e = NEXT (e);

  while (e != c->llwptr)
    {
      if (c->lastline[e] == '\n')
        break;
      len++;
      e = NEXT (e);
    }
  if (e != c->llwptr)
    e = NEXT (e);

  c->banner = malloc (len + 2); //JMM off-by one since test above
  r = 0;

  while (b != e)
    {
      c->banner[r++] = c->lastline[b];
      b = NEXT (b);
    }
  c->banner[r] = 0;

}


static void
extract_response (G2SerialCLI * c)
{
  int r = c->llrptr;
  uint8_t *ptr = c->response = malloc (G2SERIALCLI_LLL + 1);

  while (r != c->llwptr)
    {
      *(ptr++) = c->lastline[r];
      r = NEXT (r);
    }
  *(ptr++) = 0;
}

const static uint8_t fssm_rec[] =
  { 'f', 'r', 'e', 'e', ' ', 's', 'p', 'a', 'c', 'e', '\r', '\n' };
const static uint8_t bfsm_rec[] =
  { 'b', 'l', 'o', 'c', 'k', 's', ' ', 'f', 'r', 'e', 'e', '\r', '\n' };
const static uint8_t udsm_rec[] =
  { 'u', 'n', 'd', 'e', 'f', 'i', 'n', 'e', 'd', '\r', '\n' };
const static uint8_t oksm_rec[] = { ' ', 'o', 'k', '\r', '\n' };


void
G2SerialCLIParse (G2SerialCLI * c, uint8_t * buf, int len)
{

  while (len--)
    {
      uint8_t ch = *(buf++);

      /*Old hardware gets \r and \n the wrong ways aroung */
      switch (ch)
        {
        case '\r':
          if (c->crlfsm == G2SERIALCLI_CRLF_LF)
            {
              ch = '\n';
              c->crlfsm = G2SERIALCLI_CRLF_NONE;
            }
          else 
            {
              c->crlfsm = G2SERIALCLI_CRLF_CR;
            }
          break;
        case '\n':
          if (c->crlfsm== G2SERIALCLI_CRLF_CR ) {
              c->crlfsm = G2SERIALCLI_CRLF_NONE;
          } else {
            c->crlfsm = G2SERIALCLI_CRLF_LF;
            ch = '\r';
          }
          break;
        default:
          c->crlfsm = G2SERIALCLI_CRLF_NONE;
        }

#if 0
     if (ch=='\r') printf("\\r");
     if (ch=='\n') printf("\\n\n");
     if ((ch>=32) && (ch<127)) printf("%c",ch);
     fflush(stdout);
#endif


      /* Store the character in the ring buffer */
      c->lastline[c->llwptr] = ch;
      c->llwptr = NEXT (c->llwptr);
      if (c->llrptr == c->llwptr)
        c->llrptr = NEXT (c->llwptr);

      /*state machine looking for ok */
      if ((ch == ' ') && (c->oksm != G2SERIALCLI_OKSM_US))
        c->oksm = G2SERIALCLI_OKSM_LOST;
      switch (c->oksm)
        {
        case G2SERIALCLI_OKSM_LOST:
          if (ch == ' ')
            c->oksm++;
          break;
        case G2SERIALCLI_OKSM_SP:
          if (ch == 'o')
            c->oksm++;
          else
            c->oksm = G2SERIALCLI_OKSM_LOST;
          break;
        case G2SERIALCLI_OKSM_O:
          /*FIXME: DM24MK3 doesn't have _ whatever - sigh. */
          if (ch == 'k')
            /* c->oksm++; */
            c->oksm += 2;
          else
            c->oksm = G2SERIALCLI_OKSM_LOST;
          break;
        case G2SERIALCLI_OKSM_K:
          if (ch == '_')
            c->oksm++;
          else
            c->oksm = G2SERIALCLI_OKSM_LOST;
          break;
        case G2SERIALCLI_OKSM_US:
          if (ch == '\r')
            c->oksm++;
          break;
        case G2SERIALCLI_OKSM_CR:
          if (ch == '\n')
            {

              c->oks++;
              c->incmdmode = 1;
              c->oksm = G2SERIALCLI_OKSM_LOST;

              if (!c->banner)
                extract_banner (c);

              if (!c->response)
                extract_response (c);


              break;
            }

        }


      /*state machine looking for free space or blocks free*/

      if (ch != bfsm_rec[c->bfsm])
        c->bfsm = 0;
      if (ch == bfsm_rec[c->bfsm])
        c->bfsm++;
      if (c->bfsm == sizeof (bfsm_rec))
        {
#ifdef DEBUG
          printf ("<CR>");
#endif
          G2SerWrite (c->s, "\r", 1);
          c->bfsm = G2SERIALCLI_BFSM_LOST;
        }

      if (ch != fssm_rec[c->fssm])
        c->fssm = 0;
      if (ch == fssm_rec[c->fssm])
        c->fssm++;
      if (c->fssm == sizeof (fssm_rec))
        {
#ifdef DEBUG
          printf ("<CR>");
#endif
          G2SerWrite (c->s, "\r", 1);
          c->fssm = G2SERIALCLI_FSSM_LOST;
        }

      /*state machine looking for undefined */
      if (ch != udsm_rec[c->udsm])
        c->udsm = 0;
      if (ch == udsm_rec[c->udsm])
        c->udsm++;
      if (c->udsm == sizeof (udsm_rec))
        {
#ifdef DEBUG
          printf ("<CR>");
#endif
          G2SerWrite (c->s, "\r", 1);
          c->udsm = G2SERIALCLI_UDSM_LOST;
          c->undefs++;
        }

      /*Run the state machine looking for 0x1b 0x11 */

      if (ch == 0x1b)
        c->obesm = G2SERIALCLI_OBESM_LOST;

      switch (c->obesm)
        {
        case G2SERIALCLI_OBESM_LOST:
          if (ch == 0x1b)
            c->obesm++;
          break;
        case G2SERIALCLI_OBESM_OB:
          if (ch == 0x11)
            {
              /*Do stuff */

              c->incmdmode = 0;

              c->obesm = G2SERIALCLI_OBESM_LOST;
              c->oksm = G2SERIALCLI_OKSM_LOST;
              c->syncsm = G2SERIALCLI_SY_LOST;
            }
          else
            {
              c->obesm = G2SERIALCLI_OBESM_LOST;
            }
          break;
        default:
          c->obesm = G2SERIALCLI_OBESM_LOST;
        }


#ifdef DEBUG
      printf ("%d %d %d %d:", c->oksm, c->fssm, c->obesm, c->incmdmode);

      // printf("%d %d:",c->oksm,c->udsm);
      if (((ch > 31) && (ch < 127)) || (ch == '\n'))
        putchar (ch);
      else
        putchar ('.');
      printf (" %x\n", ch);
#endif

    }


}

static void
all_data (G2Serial * s, uint8_t * buf, int len)
{
  G2SerialCLI *c = (G2SerialCLI *) s->client_data;

  if ((c->callback) && (c->incmdmode))
    (c->callback) (c, buf, len);

  G2SerialCLIParse (c, buf, len);
}

G2SerialCLI *
G2CreateSerCLI (G2Serial * s)
{
  G2SerialCLI *c = (G2SerialCLI *) malloc (sizeof (G2SerialCLI));


  s->client_data = c;
  c->s = s;
  c->p = G2CreateSerParser (s, block, NULL, NULL, NULL, all_data);

  c->llrptr = 0;
  c->llwptr = 0;

  c->client_data = NULL;
  c->callback = NULL;

  c->oksm = G2SERIALCLI_OKSM_LOST;
  c->obesm = G2SERIALCLI_OBESM_LOST;
  c->fssm = G2SERIALCLI_FSSM_LOST;
  c->udsm = G2SERIALCLI_UDSM_LOST;
  c->syncsm = G2SERIALCLI_SY_LOST;
  c->cmdsm = G2SERIALCLI_CMD_NONE;
  c->bfsm = G2SERIALCLI_BFSM_LOST;
  c->oks = 0;
  c->undefs = 0;

  gettimeofday (&c->last_intr, NULL);




  if (!c->p)
    {
      free (c);
      return NULL;
    }

  c->incmdmode = 0;

  c->banner = NULL;
  c->response = NULL;
  c->command = NULL;

#ifdef DEBUG
  printf ("<INTR>");
  printf ("<CR>");
#endif
  G2SerIntr (c->s);
  G2SerWrite (c->s, "\r", 1);

  return c;
}

static void
quit_cmd_mode (G2SerialCLI * c)
{
  uint8_t die[2] = { 0x1b, 0x11 };
  /*Empty the serial buffer? */

  /*
     * Step 1, get the end device to issue 
     * 0x1b, 0x11 and return to block mode 
     * this should cause the next device return to 
     * command mode, we send some more go s to 
     * drop further up the chain. 
     *
   */


  G2SerWriteStrSlow (c->s, "\r27 emit 17 emit go\r");
  G2Sleep (500000);
  G2SerWriteStrSlow (c->s, "go\rgo\rgo\r");
  G2Sleep (1000);

  /*
   * This deals with the embarassing case where the SAM 
   * misses the DM's 0x1b 0x11 and sits relaying packets
   * thinking the DM is in terminal mode.
   */

  G2SerWrite (c->s, die, 2);

  c->oksm = G2SERIALCLI_OKSM_LOST;
  c->obesm = G2SERIALCLI_OBESM_LOST;
}


void
G2DestroySerCLI (G2SerialCLI * c)
{
  if (c->incmdmode)
    quit_cmd_mode (c);

  if (c->banner)
    free (c->banner);
  if (c->response)
    free (c->response);

  free (c);
}

void
G2HushDestroySerCLI (G2SerialCLI * c)
{

  if (c->banner)
    free (c->banner);
  if (c->response)
    free (c->response);

  free (c);
}




void
G2SerCLIDispatch (G2SerialCLI * c)
{
  struct timeval now, diff;

  if (G2SerData (c->s)) {
    G2SerDispatch (c->p);
  }

  if (c->incmdmode)
    {
      gettimeofday (&c->last_intr, NULL);
    }
  else
    {
      gettimeofday (&now, NULL);
      timersub (&now, &c->last_intr, &diff);


      if (diff.tv_sec >= INTR_TIMEOUT)
        {
#ifdef DEBUG
          printf ("<INTR>");
          printf ("<CR>");
#endif
          G2SerIntr (c->s);
          G2SerWrite (c->s, "\r", 1);
          c->last_intr = now;
        }
    }


  switch (c->syncsm)
    {
    case G2SERIALCLI_SY_LOST:
      if (c->incmdmode)
        {
          G2SerDrain (c->s, 2);
          c->oks = 0;
          G2SerWrite (c->s, "\r", 1);
          c->syncsm++;
        }
      break;
    case G2SERIALCLI_SY_DRAINED:
      if (c->oks)
        c->syncsm++;
      break;
    case G2SERIALCLI_SY_SYNCED:
      if (!c->incmdmode)
        c->syncsm = G2SERIALCLI_SY_LOST;
      break;
    }

  switch (c->cmdsm)
    {
    case G2SERIALCLI_CMD_NONE:
    case G2SERIALCLI_CMD_DONE:
      break;
    case G2SERIALCLI_CMD_PENDING:
      if (c->syncsm == G2SERIALCLI_SY_SYNCED)
        {
          c->oks = 0;
          c->undefs = 0;
          G2SerWriteStrSlow (c->s, c->command);
          if (c->response)
            free (c->response);
          c->response = NULL;
          c->oksm = G2SERIALCLI_OKSM_LOST;
          c->cmdsm++;
          c->llrptr = c->llwptr;
        }
      break;

    case G2SERIALCLI_CMD_SENT:
      if (c->oks)
        {
          c->syncsm = G2SERIALCLI_SY_LOST;
          G2SerWrite (c->s, "\r", 1);
          c->cmdsm++;
        }
      break;
    case G2SERIALCLI_CMD_SYNC:
      if (c->syncsm == G2SERIALCLI_SY_SYNCED)
        c->cmdsm++;
      break;
    }


}

int
G2SerCLIBlock (G2SerialCLI * c, int timeout)
{
  char *cmd = c->command;
  struct timeval now, then, diff;
  int fd = c->s->fd;
  fd_set rfds;

  gettimeofday (&then, NULL);

  FD_ZERO (&rfds);

  while (((c->cmdsm != G2SERIALCLI_CMD_DONE)
          && (c->cmdsm != G2SERIALCLI_CMD_NONE))
         || (c->syncsm != G2SERIALCLI_SY_SYNCED))
    {
      gettimeofday (&now, NULL);

      timersub (&now, &then, &diff);
      if (diff.tv_sec > timeout) {
        return 0;
      }


      FD_SET (fd, &rfds);

      diff.tv_sec = timeout - diff.tv_sec;
      diff.tv_usec = 0;

      if (diff.tv_sec == 0)
        diff.tv_sec = 1;
      if (diff.tv_sec > SELECT_TIMEOUT)
        diff.tv_sec = SELECT_TIMEOUT;

      select (fd + 1, &rfds, NULL, NULL, &diff);
      G2SerCLIDispatch (c);

    }

  return 1;
}

int
G2SerCLIInCmdMode (G2SerialCLI * c)
{
  return (c->syncsm == G2SERIALCLI_SY_SYNCED);
}

char *
G2SerCLIBanner (G2SerialCLI * c)
{
  return c->banner;
}

char *
G2SerCLIResponse (G2SerialCLI * c)
{
  return c->response;
}

void
G2SerCLICommand (G2SerialCLI * c, char *cmd)
{
  c->command = cmd;
  c->cmdsm = G2SERIALCLI_CMD_PENDING;
  if (c->response)
    {
      free (c->response);
      c->response = NULL;
    }
}

int
G2SerCLICommandDone (G2SerialCLI * c)
{
  return (c->cmdsm == G2SERIALCLI_CMD_DONE);
}
