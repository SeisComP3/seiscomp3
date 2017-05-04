%{
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
 *  Copyright (C) 1993-2014 Spread Concepts LLC <info@spreadconcepts.com>
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



#include "arch.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef ARCH_PC_WIN95
#include <sys/types.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/param.h>

#else /* ARCH_PC_WIN95 */
#include <winsock.h>
#endif /* ARCH_PC_WIN95 */

#include "spu_alarm.h"
#include "configuration.h"
#include "spu_memory.h"
#include "spu_objects.h"
#include "conf_body.h"
#include "acm.h"

        int     line_num, semantic_errors;
 extern char    *yytext;
 extern int     yyerror(char *str);
 extern void    yywarn(char *str);
 extern int     yylex();

 static int     num_procs = 0;
 static int     segment_procs = 0;
 static int     segments = 0;
 static int     rvec_num = 0;
 static int     procs_interfaces = 0;

 static int     authentication_configured = 0;

#define MAX_ALARM_FORMAT 40
 static char    alarm_format[MAX_ALARM_FORMAT];
 static int     alarm_precise = 0;
 static int     alarm_custom_format = 0;

void    parser_init()
{
    num_procs = 0;
    segment_procs = 0;
    segments = 0;
    rvec_num = 0;
    procs_interfaces = 0;
}

static char *segment2str(int seg) {
  static char ipstr[40];
  int id = Config->segments[seg].bcast_address;
  sprintf(ipstr, "%d.%d.%d.%d:%d",
  	(id & 0xff000000)>>24,
  	(id & 0xff0000)>>16,
  	(id & 0xff00)>>8,
  	(id & 0xff),
	Config->segments[seg].port);
  return ipstr;
}
static void alarm_print_proc(proc *p, int port) {
  if(port == p->port)
    Alarm(CONF_SYS, "\t%20s: %d.%d.%d.%d\n", p->name,
  	  (p->id & 0xff000000)>>24,
  	  (p->id & 0xff0000)>>16,
  	  (p->id & 0xff00)>>8,
  	  (p->id & 0xff));
  else
    Alarm(CONF_SYS, "\t%20s: %d.%d.%d.%d:%d\n", p->name,
  	  (p->id & 0xff000000)>>24,
  	  (p->id & 0xff0000)>>16,
  	  (p->id & 0xff00)>>8,
  	  (p->id & 0xff),
	  p->port);
}

static int32u name2ip(char *name) {
  int anip, i1, i2, i3, i4;
  struct hostent *host_ptr;

  host_ptr = gethostbyname(name);
  
  if ( host_ptr == 0)
    Alarm( EXIT, "Conf_init: no such host %s\n",
	   name);
  
  memcpy(&anip, host_ptr->h_addr_list[0], 
	 sizeof(int32) );
  anip = htonl( anip );
  i1= ( anip & 0xff000000 ) >> 24;
  i2= ( anip & 0x00ff0000 ) >> 16;
  i3= ( anip & 0x0000ff00 ) >>  8;
  i4=   anip & 0x000000ff;
  return ((i1<<24)|(i2<<16)|(i3<<8)|i4);
}

static  void expand_filename(char *out_string, int str_size, const char *in_string)
{
  const char *in_loc;
  char *out_loc;
  char hostn[MAXHOSTNAMELEN+1];
  
  for ( in_loc = in_string, out_loc = out_string; out_loc - out_string < str_size; in_loc++ )
  {
          if (*in_loc == '%' ) {
                  switch( in_loc[1] ) {
                  case 'h':
                  case 'H':
                          gethostname(hostn, sizeof(hostn) );
                          out_loc += snprintf(out_loc, str_size - (out_loc - out_string), "%s", hostn); 
                          in_loc++;
                          continue;
                  default:
                          break;
                  }

          }
          *out_loc = *in_loc;
          out_loc++;
          if (*in_loc == '\0') break;
  }
  out_string[str_size-1] = '\0';
}

static  int 	get_parsed_proc_info( char *name, proc *p )
{
	int	i;

	for ( i=0; i < num_procs; i++ )
	{
		if ( strcmp( Config->allprocs[i].name, name ) == 0 )
		{
			*p = Config->allprocs[i];
			return( i );
		}
	}
	return( -1 );
}
/* convert_segment_to_string()
 * char * segstr : output string
 * int strsize : length of output string space
 * segment *seg : input segment structure
 * int return : length of string written or -1 if error (like string not have room)
 * 
 *
 * The format of the returned string will be as shown below with each segment appended
 * to the string. Each use of IPB will be replaced with the broadcast IP address, port
 * with the port. The optional section is a list of interfaces tagged with D or C
 * and idnetified by ip address. 
 *
 * "Segment IP:port host1name host1ip (ALL/ANY/C/D/M IP)+ host2name host2ip (ALL/ANY/C/D/M IP )+ ..."
 *
 */
static  int    convert_segment_to_string(char *segstr, int strsize, segment *seg)
{
    int         i,j;
    size_t      curlen = 0;
    char        temp_str[200];

    sprintf(temp_str, "Segment %d.%d.%d.%d:%d ", 
            (seg->bcast_address & 0xff000000)>>24, 
            (seg->bcast_address & 0xff0000)>>16, 
            (seg->bcast_address & 0xff00)>>8, 
            (seg->bcast_address & 0xff), 
            seg->port );

    strncat( segstr, temp_str, strsize - curlen);
    curlen += strlen(temp_str);

    for (i = 0; i < seg->num_procs; i++) {
        sprintf(temp_str, "%s %d.%d.%d.%d ", 
                seg->procs[i]->name, 
                (seg->procs[i]->id & 0xff000000)>>24, 
                (seg->procs[i]->id & 0xff0000)>>16, 
                (seg->procs[i]->id & 0xff00)>>8, 
                (seg->procs[i]->id & 0xff) );
        strncat( segstr, temp_str, strsize - curlen);
        curlen += strlen(temp_str);

        /* Now add all interfaces */
        for ( j=0 ; j < seg->procs[i]->num_if; j++) {
            /* add addional interface specs to string */
            if ( seg->procs[i]->ifc[j].type & IFTYPE_ANY )
            {
                strncat( segstr, "ANY ", strsize - curlen);
                curlen += 4;
            }
            if ( seg->procs[i]->ifc[j].type & IFTYPE_DAEMON )
            {
                strncat( segstr, "D ", strsize - curlen);
                curlen += 2;
            }
            if ( seg->procs[i]->ifc[j].type & IFTYPE_CLIENT )
            {
                strncat( segstr, "C ", strsize - curlen);
                curlen += 2;
            }
            if ( seg->procs[i]->ifc[j].type & IFTYPE_MONITOR )
            {
                strncat( segstr, "M ", strsize - curlen);
                curlen += 2;
            }
            sprintf(temp_str, "%d.%d.%d.%d ", 
                (seg->procs[i]->ifc[j].ip & 0xff000000)>>24, 
                (seg->procs[i]->ifc[j].ip & 0xff0000)>>16, 
                (seg->procs[i]->ifc[j].ip & 0xff00)>>8, 
                (seg->procs[i]->ifc[j].ip & 0xff) );
            strncat( segstr, temp_str, strsize - curlen);
            curlen += strlen(temp_str);
        }
    }

    /* terminate each segment by a newline */
    strncat( segstr, "\n", strsize - curlen);
    curlen += 1;

    if ((int) curlen > strsize) {
        /* ran out of space in string -- should never happen. */
        Alarmp( SPLOG_ERROR, CONF_SYS, "config_parse.y:convert_segment_to_string: The segment string is too long! %d characters attemped is more then %d characters allowed", curlen, strsize);
        Alarmp( SPLOG_ERROR, CONF_SYS, "config_parse.y:convert_segment_to_string:The error occured on segment %d.%d.%d.%d. Successful string was: %s\n",
                (seg->bcast_address & 0xff000000)>>24, 
                (seg->bcast_address & 0xff0000)>>16, 
                (seg->bcast_address & 0xff00)>>8, 
                (seg->bcast_address & 0xff), 
                segstr);
        return(-1);
    }

    Alarmp( SPLOG_DEBUG, CONF_SYS, "config_parse.y:convert_segment_to_string:The segment string is %d characters long:\n%s", curlen, segstr);
    return(curlen);
}

#define PROC_NAME_CHECK( stoken ) { \
                                            char strbuf[80]; \
                                            int ret; \
                                            proc p; \
                                            if ( strlen((stoken)) >= MAX_PROC_NAME ) { \
                                                snprintf(strbuf, 80, "Too long name(%d max): %s)\n", MAX_PROC_NAME, (stoken)); \
                                                return (yyerror(strbuf)); \
                                            } \
                                            ret = get_parsed_proc_info( stoken, &p ); \
                                            if (ret >= 0) { \
                                                snprintf(strbuf, 80, "Name not unique. name: %s equals (%s, %d.%d.%d.%d)\n", (stoken), p.name, IP1(p.id), IP2(p.id), IP3(p.id), IP4(p.id) ); \
                                                return (yyerror(strbuf)); \
                                            } \
                                         }
#define PROCS_CHECK( num_procs, stoken ) { \
                                            char strbuf[80]; \
                                            if ( (num_procs) >= MAX_PROCS_RING ) { \
                                                snprintf(strbuf, 80, "%s (Too many daemons configured--%d max)\n", (stoken), MAX_PROCS_RING); \
                                                return (yyerror(strbuf)); \
                                            } \
                                         }
#define SEGMENT_CHECK( num_segments, stoken )  { \
                                            char strbuf[80]; \
                                            if ( (num_segments) >= MAX_SEGMENTS ) { \
                                                snprintf(strbuf, 80, "%s (Too many segments configured--%d max)\n", (stoken), MAX_SEGMENTS); \
                                                return( yyerror(strbuf)); \
                                            } \
                                         }
#define SEGMENT_SIZE_CHECK( num_procs, stoken )  { \
                                            char strbuf[80]; \
                                            if ( (num_procs) >= MAX_PROCS_SEGMENT ) { \
                                                snprintf(strbuf, 80, "%s (Too many daemons configured in segment--%d max)\n", (stoken), MAX_PROCS_SEGMENT); \
                                                return( yyerror(strbuf)); \
                                            } \
                                         }
#define INTERFACE_NUM_CHECK( num_ifs, stoken )  { \
                                            char strbuf[80]; \
                                            if ( (num_ifs) >= MAX_INTERFACES_PROC ) { \
                                                snprintf(strbuf, 80, "%s (Too many interfaces configured in proc--%d max)\n", (stoken), MAX_INTERFACES_PROC); \
                                                return( yyerror(strbuf)); \
                                            } \
                                         }


%}
%start Config
%token SEGMENT EVENTLOGFILE EVENTTIMESTAMP EVENTPRECISETIMESTAMP EVENTPRIORITY IPADDR NUMBER COLON
%token PDEBUG PINFO PWARNING PERROR PCRITICAL PFATAL
%token OPENBRACE CLOSEBRACE EQUALS STRING
%token DEBUGFLAGS BANG
%token DDEBUG DEXIT DPRINT DDATA_LINK DNETWORK DPROTOCOL DSESSION
%token DCONF DMEMB DFLOW_CONTROL DSTATUS DEVENTS DGROUPS DMEMORY
%token DSKIPLIST DACM DSECURITY DALL DNONE
%token DEBUGINITIALSEQUENCE
%token DANGEROUSMONITOR SOCKETPORTREUSE RUNTIMEDIR SPUSER SPGROUP ALLOWEDAUTHMETHODS REQUIREDAUTHMETHODS ACCESSCONTROLPOLICY
%token MAXSESSIONMESSAGES
%token WINDOW PERSONALWINDOW ACCELERATEDRING ACCELERATEDWINDOW
%token TOKENTIMEOUT HURRYTIMEOUT ALIVETIMEOUT JOINTIMEOUT REPTIMEOUT SEGTIMEOUT GATHERTIMEOUT FORMTIMEOUT LOOKUPTIMEOUT
%token SP_BOOL SP_TRIVAL LINKPROTOCOL PHOP PTCPHOP
%token IMONITOR ICLIENT IDAEMON
%token ROUTEMATRIX LINKCOST
%%
Config		:	ConfigStructs
                        {
			  Config->num_segments = segments;
			  Config->num_total_procs = num_procs;
			  Alarm(CONF_SYS, "Finished configuration file.\n");
                          Alarmp( SPLOG_DEBUG, CONF_SYS, "config_parse.y:The full segment string is %d characters long:\n%s", strlen(ConfStringRep), ConfStringRep);
			}


ConfigStructs	:	SegmentStruct ConfigStructs
		|	ParamStruct ConfigStructs
		|	RouteStruct ConfigStructs
		|
		;

AlarmBit	:	DDEBUG { $$ = $1; }
		|	DEXIT { $$ = $1; }
		|	DPRINT { $$ = $1; }
		|	DDATA_LINK { $$ = $1; }
		|	DNETWORK { $$ = $1; }
		|	DPROTOCOL { $$ = $1; }
		|	DSESSION { $$ = $1; }
		|	DCONF { $$ = $1; }
		|	DMEMB { $$ = $1; }
		|	DFLOW_CONTROL { $$ = $1; }
		|	DSTATUS { $$ = $1; }
		|	DEVENTS { $$ = $1; }
		|	DGROUPS { $$ = $1; }
		|	DMEMORY { $$ = $1; }
		|	DSKIPLIST { $$ = $1; }
		|	DACM { $$ = $1; }
		|	DSECURITY { $$ = $1; }
		|	DALL { $$ = $1; }
		|	DNONE { $$ = $1; }
		;

AlarmBitComp	:	AlarmBitComp AlarmBit
			{
			  $$.mask = ($1.mask | $2.mask);
			}
		|	AlarmBitComp BANG AlarmBit
			{
			  $$.mask = ($1.mask & ~($3.mask));
			}
		|	{ $$.mask = NONE; }
		;
PriorityLevel   :       PDEBUG { $$ = $1; }
                |       PINFO { $$ = $1; }
                |       PWARNING { $$ = $1; }
                |       PERROR { $$ = $1; }
                |       PCRITICAL { $$ = $1; }
                |       PFATAL { $$ = $1; }
                ;

ParamStruct	:	DEBUGFLAGS EQUALS OPENBRACE AlarmBitComp CLOSEBRACE
			{
			  if (! Alarm_get_interactive() ) {
                            Alarm_clear_types(ALL);
			    Alarm_set_types($4.mask);
			    Alarm(CONF_SYS, "Set Alarm mask to: %x\n", Alarm_get_types());
                          }
			}
                |       EVENTPRIORITY EQUALS PriorityLevel
                        {
                            if (! Alarm_get_interactive() ) {
                                Alarm_set_priority($3.number);
                            }
                        }

		|	EVENTLOGFILE EQUALS STRING
			{
			  if (! Alarm_get_interactive() ) {
                            char file_buf[MAXPATHLEN];
                            expand_filename(file_buf, MAXPATHLEN, $3.string);
                            Alarm_set_output(file_buf);
                          }
			}
		|	EVENTTIMESTAMP EQUALS STRING
			{
			  if (! Alarm_get_interactive() ) {
                              strncpy(alarm_format, $3.string, MAX_ALARM_FORMAT);
                              alarm_custom_format = 1;
                              if (alarm_precise) {
                                  Alarm_enable_timestamp_high_res(alarm_format);
                              } else {
                                  Alarm_enable_timestamp(alarm_format);
                              }
                          }
			}
		|	EVENTTIMESTAMP
			{
			  if (! Alarm_get_interactive() ) {
                              if (alarm_precise) {
                                  Alarm_enable_timestamp_high_res(NULL);
                              } else {
                                  Alarm_enable_timestamp(NULL);
                              }
                          }
			}
		|	EVENTPRECISETIMESTAMP
			{
			  if (! Alarm_get_interactive() ) {
                              alarm_precise = 1;
                              if (alarm_custom_format) {
                                  Alarm_enable_timestamp_high_res(alarm_format);
                              } else {
                                  Alarm_enable_timestamp_high_res(NULL);
                              }
                          }
			}
		|	DANGEROUSMONITOR EQUALS SP_BOOL
			{
			  if (! Alarm_get_interactive() ) {
                            Conf_set_dangerous_monitor_state($3.boolean);
                          }
			}
                |       SOCKETPORTREUSE EQUALS SP_TRIVAL
                        {
                            port_reuse state;
                            if ($3.number == 1)
                            {
                                state = port_reuse_on;
                            }
                            else if ($3.number == 0)
                            {
                                state = port_reuse_off;
                            }
                            else
                            {
                                /* Default to AUTO. */
                                state = port_reuse_auto;
                            }
                            Conf_set_port_reuse_type(state);
                        }
                |       RUNTIMEDIR EQUALS STRING
                        {
                            Conf_set_runtime_dir($3.string);
                        }
                |       DEBUGINITIALSEQUENCE
                        {
                            Conf_set_debug_initial_sequence();
                        }
                |       SPUSER EQUALS STRING
                        {
                            Conf_set_user($3.string);
                        }
                |       SPGROUP EQUALS STRING
                        {
                            Conf_set_group($3.string);
                        }
                |       ALLOWEDAUTHMETHODS EQUALS STRING
                        {
                            char auth_list[MAX_AUTH_LIST_LEN];
                            int i, len;
                            char *c_ptr;
                            if (!authentication_configured) {
                                Acm_auth_set_disabled("NULL");
                            }
                            authentication_configured = 1;

                            strncpy(auth_list, $3.string, MAX_AUTH_LIST_LEN);
                            len = strlen(auth_list); 
                            for (i=0; i < len; )
                            {
                                c_ptr = strchr(&auth_list[i], ' ');
                                if (c_ptr != NULL)
                                {
                                    *c_ptr = '\0';
                                }
                                Acm_auth_set_enabled(&auth_list[i]);    
                                i += strlen(&auth_list[i]);
                                i++; /* for null */
                            }
                        }
                |       REQUIREDAUTHMETHODS EQUALS STRING
                        {
                            char auth_list[MAX_AUTH_LIST_LEN];
                            int i, len;
                            char *c_ptr;
                            if (!authentication_configured) {
                                Acm_auth_set_disabled("NULL");
                            }
                            authentication_configured = 1;

                            strncpy(auth_list, $3.string, MAX_AUTH_LIST_LEN);
                            len = strlen(auth_list); 
                            for (i=0; i < len; )
                            {
                                c_ptr = strchr(&auth_list[i], ' ');
                                if (c_ptr != NULL)
                                {
                                    *c_ptr = '\0';
                                } 
                                Acm_auth_set_required(&auth_list[i]);    
                                i += strlen(&auth_list[i]);
                                i++; /* for null */
                            }
                        }
                |       ACCESSCONTROLPOLICY EQUALS STRING
                        {
                            int ret;
                            ret = Acm_acp_set_policy($3.string);
                            if (!ret)
                            {
                                    yyerror("Invalid Access Control Policy name. Make sure it is spelled right and any needed mocdules are loaded");
                            }
                        }
		|	MAXSESSIONMESSAGES EQUALS NUMBER
			{
                            Conf_set_max_session_messages($3.number);
			}
		|	LINKPROTOCOL EQUALS PHOP
			{
                            Conf_set_link_protocol(HOP_PROT);
			}
		|	LINKPROTOCOL EQUALS PTCPHOP
			{
                            Conf_set_link_protocol(TCP_PROT);
			}
		|	WINDOW EQUALS NUMBER
			{
			    Conf_set_window($3.number);
			}
		|	PERSONALWINDOW EQUALS NUMBER
			{
			    Conf_set_personal_window($3.number);
			}
		|	ACCELERATEDRING EQUALS SP_BOOL
			{
			    Conf_set_accelerated_ring_flag(TRUE);
			    Conf_set_accelerated_ring($3.boolean);
			}
		|	ACCELERATEDWINDOW EQUALS NUMBER
			{
			    Conf_set_accelerated_window($3.number);
			}
		|	TOKENTIMEOUT EQUALS NUMBER
			{
			    Conf_set_token_timeout($3.number);
			}
		|	HURRYTIMEOUT EQUALS NUMBER
			{
			    Conf_set_hurry_timeout($3.number);
			}
		|	ALIVETIMEOUT EQUALS NUMBER
			{
			    Conf_set_alive_timeout($3.number);
			}
		|	JOINTIMEOUT EQUALS NUMBER
			{
			    Conf_set_join_timeout($3.number);
			}
		|	REPTIMEOUT EQUALS NUMBER
			{
			    Conf_set_rep_timeout($3.number);
			}
		|	SEGTIMEOUT EQUALS NUMBER
			{
			    Conf_set_seg_timeout($3.number);
			}
		|	GATHERTIMEOUT EQUALS NUMBER
			{
			    Conf_set_gather_timeout($3.number);
			}
		|	FORMTIMEOUT EQUALS NUMBER
			{
			    Conf_set_form_timeout($3.number);
			}
		|	LOOKUPTIMEOUT EQUALS NUMBER
			{
			    Conf_set_lookup_timeout($3.number);
			}

SegmentStruct	:    SEGMENT IPADDR OPENBRACE Segmentparams CLOSEBRACE
                        { int i;
                          int added_len;
                          SEGMENT_CHECK( segments, inet_ntoa($2.ip.addr) );
			  Config->segments[segments].num_procs = segment_procs;
			  Config->segments[segments].port = $2.ip.port;
			  Config->segments[segments].bcast_address =
			    $2.ip.addr.s_addr;
			  if(Config->segments[segments].port == 0)
			    Config->segments[segments].port = DEFAULT_SPREAD_PORT;
			  Alarm(CONF_SYS, "Successfully configured Segment %d [%s] with %d procs:\n",
				segments,
				segment2str(segments),
				segment_procs);
			  for(i=(num_procs - segment_procs); i<num_procs; i++) {
                                  /* This '1' is to keep each proc with the same port as the segment.*/
			    if( 1 || Config->allprocs[i].port==0)  {
			      Config->allprocs[i].port=
				Config->segments[segments].port;
			    }
			    alarm_print_proc(&(Config->allprocs[i]),
			    	Config->segments[segments].port);
			  }
                          /* generate string representation of segment */
                          added_len = convert_segment_to_string(&ConfStringRep[ConfStringLen], MAX_CONF_STRING - ConfStringLen, &Config->segments[segments] );
                          if (added_len == -1 )
                              yyerror("Failed to update string with segment!\n");
                          ConfStringLen += added_len;

			  segments++;
			  segment_procs = 0;
			}
		;

Segmentparams	:	Segmentparam Segmentparams
		|
		;

Segmentparam	:	STRING IPADDR OPENBRACE Interfaceparams CLOSEBRACE
                        { 
                          PROC_NAME_CHECK( $1.string );
                          PROCS_CHECK( num_procs, $1.string );
                          SEGMENT_CHECK( segments, $1.string );
                          SEGMENT_SIZE_CHECK( segment_procs, $1.string );
                          if (procs_interfaces == 0)
                                  yyerror("Interfaces section declared but no actual interface addresses defined\n");
                          strcpy(Config->allprocs[num_procs].name, $1.string);
                          Config->allprocs[num_procs].id = $2.ip.addr.s_addr;
 		          Config->allprocs[num_procs].port = $2.ip.port;
			  Config->allprocs[num_procs].seg_index = segments;
			  Config->allprocs[num_procs].index_in_seg = segment_procs;
                          Config->allprocs[num_procs].num_if = procs_interfaces;
			  Config->segments[segments].procs[segment_procs] = 
                              &(Config->allprocs[num_procs]);
			  num_procs++;
			  segment_procs++;
                          procs_interfaces = 0;
			}
		|	STRING OPENBRACE Interfaceparams CLOSEBRACE
			{ 
                          PROC_NAME_CHECK( $1.string );
                          PROCS_CHECK( num_procs, $1.string );
                          SEGMENT_CHECK( segments, $1.string );
                          SEGMENT_SIZE_CHECK( segment_procs, $1.string );
                          if (procs_interfaces == 0)
                                  yyerror("Interfaces section declared but no actual interface addresses defined\n");
                          strcpy(Config->allprocs[num_procs].name, $1.string);
                          Config->allprocs[num_procs].id =
			    name2ip(Config->allprocs[num_procs].name);
 		          Config->allprocs[num_procs].port = 0;
			  Config->allprocs[num_procs].seg_index = segments;
			  Config->allprocs[num_procs].index_in_seg = segment_procs;
                          Config->allprocs[num_procs].num_if = procs_interfaces;
			  Config->segments[segments].procs[segment_procs] = 
                              &(Config->allprocs[num_procs]);
			  num_procs++;
			  segment_procs++;
                          procs_interfaces = 0;
			}
                |       STRING IPADDR
			{ 
                          PROC_NAME_CHECK( $1.string );
                          PROCS_CHECK( num_procs, $1.string );
                          SEGMENT_CHECK( segments, $1.string );
                          SEGMENT_SIZE_CHECK( segment_procs, $1.string );
                          strcpy(Config->allprocs[num_procs].name, $1.string);
                          Config->allprocs[num_procs].id = $2.ip.addr.s_addr;
 		          Config->allprocs[num_procs].port = $2.ip.port;
			  Config->allprocs[num_procs].seg_index = segments;
			  Config->allprocs[num_procs].index_in_seg = segment_procs;
                          Config->allprocs[num_procs].num_if = 1;
                          Config->allprocs[num_procs].ifc[0].ip = Config->allprocs[num_procs].id;
                          Config->allprocs[num_procs].ifc[0].port = Config->allprocs[num_procs].port;
                          Config->allprocs[num_procs].ifc[0].type = IFTYPE_ALL | IFTYPE_ANY;
			  Config->segments[segments].procs[segment_procs] = 
                              &(Config->allprocs[num_procs]);
			  num_procs++;
			  segment_procs++;
                          procs_interfaces = 0;
			}
		|	STRING
			{ 
                          PROC_NAME_CHECK( $1.string );
                          PROCS_CHECK( num_procs, $1.string );
                          SEGMENT_CHECK( segments, $1.string );
                          SEGMENT_SIZE_CHECK( segment_procs, $1.string );
                          strcpy(Config->allprocs[num_procs].name, $1.string);
                          Config->allprocs[num_procs].id =
			    name2ip(Config->allprocs[num_procs].name);
 		          Config->allprocs[num_procs].port = 0;
			  Config->allprocs[num_procs].seg_index = segments;
			  Config->allprocs[num_procs].index_in_seg = segment_procs;
                          Config->allprocs[num_procs].num_if = 1;
                          Config->allprocs[num_procs].ifc[0].ip = Config->allprocs[num_procs].id;
                          Config->allprocs[num_procs].ifc[0].port = Config->allprocs[num_procs].port;
                          Config->allprocs[num_procs].ifc[0].type = IFTYPE_ALL | IFTYPE_ANY;
			  Config->segments[segments].procs[segment_procs] = 
                              &(Config->allprocs[num_procs]);
			  num_procs++;
			  segment_procs++;
                          procs_interfaces = 0;
			}
		;

IfType  	:	IMONITOR { $$ = $1; }
		|	ICLIENT { $$ = $1; }
		|	IDAEMON { $$ = $1; }
		;

IfTypeComp	:       IfTypeComp IfType
			{
			  $$.mask = ($1.mask | $2.mask);
			}
		|	{ $$.mask = 0; }
		;

Interfaceparams	:	Interfaceparam Interfaceparams
		|
		;

Interfaceparam	:	IfTypeComp IPADDR
			{ 
                          PROCS_CHECK( num_procs, $1.string );
                          SEGMENT_CHECK( segments, $1.string );
                          SEGMENT_SIZE_CHECK( segment_procs, $1.string );
                          INTERFACE_NUM_CHECK( procs_interfaces, $1.string );
                          Config->allprocs[num_procs].ifc[procs_interfaces].ip = $2.ip.addr.s_addr;
                          Config->allprocs[num_procs].ifc[procs_interfaces].port = $2.ip.port;
                          if ($1.mask == 0)
                                  Config->allprocs[num_procs].ifc[procs_interfaces].type = IFTYPE_ALL;
                          else 
                                  Config->allprocs[num_procs].ifc[procs_interfaces].type = $1.mask;
                          procs_interfaces++;
			}
		;

RouteStruct	:    ROUTEMATRIX OPENBRACE Routevectors CLOSEBRACE
                        { 
			  Alarm(CONF_SYS, "Successfully configured Routing Matrix for %d Segments with %d rows in the routing matrix\n",segments, rvec_num);
			}
		;

Routevectors	:	Routevectors Routevector
		|
		;

Routevector     :       LINKCOST
                        { 
                                int rvec_element;
                                for (rvec_element = 0; rvec_element < segments; rvec_element++) {
                                        if ($1.cost[rvec_element] < 0) yyerror("Wrong number of entries for routing matrix");
                                        LinkWeights[rvec_num][rvec_element] = $1.cost[rvec_element];
                                }
                                rvec_num++;
                        }
                ;
%%
void yywarn(char *str) {
        fprintf(stderr, "-------Parse Warning-----------\n");
        fprintf(stderr, "Parser warning on or before line %d\n", line_num);
        fprintf(stderr, "Error type; %s\n", str);
        fprintf(stderr, "Offending token: %s\n", yytext);
}
int yyerror(char *str) {
  fprintf(stderr, "-------------------------------------------\n");
  fprintf(stderr, "Parser error on or before line %d\n", line_num);
  fprintf(stderr, "Error type; %s\n", str);
  fprintf(stderr, "Offending token: %s\n", yytext);
  exit(1);
}
