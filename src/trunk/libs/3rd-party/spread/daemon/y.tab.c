/* A Bison parser, made by GNU Bison 2.3.  */

/* Skeleton implementation for Bison's Yacc-like parsers in C

   Copyright (C) 1984, 1989, 1990, 2000, 2001, 2002, 2003, 2004, 2005, 2006
   Free Software Foundation, Inc.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.

   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

/* C LALR(1) parser skeleton written by Richard Stallman, by
   simplifying the original so-called "semantic" parser.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output.  */
#define YYBISON 1

/* Bison version.  */
#define YYBISON_VERSION "2.3"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 0

/* Using locations.  */
#define YYLSP_NEEDED 0



/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
     SEGMENT = 258,
     EVENTLOGFILE = 259,
     EVENTTIMESTAMP = 260,
     EVENTPRECISETIMESTAMP = 261,
     EVENTPRIORITY = 262,
     IPADDR = 263,
     NUMBER = 264,
     COLON = 265,
     PDEBUG = 266,
     PINFO = 267,
     PWARNING = 268,
     PERROR = 269,
     PCRITICAL = 270,
     PFATAL = 271,
     OPENBRACE = 272,
     CLOSEBRACE = 273,
     EQUALS = 274,
     STRING = 275,
     DEBUGFLAGS = 276,
     BANG = 277,
     DDEBUG = 278,
     DEXIT = 279,
     DPRINT = 280,
     DDATA_LINK = 281,
     DNETWORK = 282,
     DPROTOCOL = 283,
     DSESSION = 284,
     DCONF = 285,
     DMEMB = 286,
     DFLOW_CONTROL = 287,
     DSTATUS = 288,
     DEVENTS = 289,
     DGROUPS = 290,
     DMEMORY = 291,
     DSKIPLIST = 292,
     DACM = 293,
     DSECURITY = 294,
     DALL = 295,
     DNONE = 296,
     DEBUGINITIALSEQUENCE = 297,
     DANGEROUSMONITOR = 298,
     SOCKETPORTREUSE = 299,
     RUNTIMEDIR = 300,
     SPUSER = 301,
     SPGROUP = 302,
     ALLOWEDAUTHMETHODS = 303,
     REQUIREDAUTHMETHODS = 304,
     ACCESSCONTROLPOLICY = 305,
     MAXSESSIONMESSAGES = 306,
     WINDOW = 307,
     PERSONALWINDOW = 308,
     ACCELERATEDRING = 309,
     ACCELERATEDWINDOW = 310,
     TOKENTIMEOUT = 311,
     HURRYTIMEOUT = 312,
     ALIVETIMEOUT = 313,
     JOINTIMEOUT = 314,
     REPTIMEOUT = 315,
     SEGTIMEOUT = 316,
     GATHERTIMEOUT = 317,
     FORMTIMEOUT = 318,
     LOOKUPTIMEOUT = 319,
     SP_BOOL = 320,
     SP_TRIVAL = 321,
     LINKPROTOCOL = 322,
     PHOP = 323,
     PTCPHOP = 324,
     IMONITOR = 325,
     ICLIENT = 326,
     IDAEMON = 327,
     ROUTEMATRIX = 328,
     LINKCOST = 329
   };
#endif
/* Tokens.  */
#define SEGMENT 258
#define EVENTLOGFILE 259
#define EVENTTIMESTAMP 260
#define EVENTPRECISETIMESTAMP 261
#define EVENTPRIORITY 262
#define IPADDR 263
#define NUMBER 264
#define COLON 265
#define PDEBUG 266
#define PINFO 267
#define PWARNING 268
#define PERROR 269
#define PCRITICAL 270
#define PFATAL 271
#define OPENBRACE 272
#define CLOSEBRACE 273
#define EQUALS 274
#define STRING 275
#define DEBUGFLAGS 276
#define BANG 277
#define DDEBUG 278
#define DEXIT 279
#define DPRINT 280
#define DDATA_LINK 281
#define DNETWORK 282
#define DPROTOCOL 283
#define DSESSION 284
#define DCONF 285
#define DMEMB 286
#define DFLOW_CONTROL 287
#define DSTATUS 288
#define DEVENTS 289
#define DGROUPS 290
#define DMEMORY 291
#define DSKIPLIST 292
#define DACM 293
#define DSECURITY 294
#define DALL 295
#define DNONE 296
#define DEBUGINITIALSEQUENCE 297
#define DANGEROUSMONITOR 298
#define SOCKETPORTREUSE 299
#define RUNTIMEDIR 300
#define SPUSER 301
#define SPGROUP 302
#define ALLOWEDAUTHMETHODS 303
#define REQUIREDAUTHMETHODS 304
#define ACCESSCONTROLPOLICY 305
#define MAXSESSIONMESSAGES 306
#define WINDOW 307
#define PERSONALWINDOW 308
#define ACCELERATEDRING 309
#define ACCELERATEDWINDOW 310
#define TOKENTIMEOUT 311
#define HURRYTIMEOUT 312
#define ALIVETIMEOUT 313
#define JOINTIMEOUT 314
#define REPTIMEOUT 315
#define SEGTIMEOUT 316
#define GATHERTIMEOUT 317
#define FORMTIMEOUT 318
#define LOOKUPTIMEOUT 319
#define SP_BOOL 320
#define SP_TRIVAL 321
#define LINKPROTOCOL 322
#define PHOP 323
#define PTCPHOP 324
#define IMONITOR 325
#define ICLIENT 326
#define IDAEMON 327
#define ROUTEMATRIX 328
#define LINKCOST 329




/* Copy the first part of user declarations.  */
#line 1 "./config_parse.y"

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




/* Enabling traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif

/* Enabling verbose error messages.  */
#ifdef YYERROR_VERBOSE
# undef YYERROR_VERBOSE
# define YYERROR_VERBOSE 1
#else
# define YYERROR_VERBOSE 0
#endif

/* Enabling the token table.  */
#ifndef YYTOKEN_TABLE
# define YYTOKEN_TABLE 0
#endif

#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef int YYSTYPE;
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif



/* Copy the second part of user declarations.  */


/* Line 216 of yacc.c.  */
#line 573 "y.tab.c"

#ifdef short
# undef short
#endif

#ifdef YYTYPE_UINT8
typedef YYTYPE_UINT8 yytype_uint8;
#else
typedef unsigned char yytype_uint8;
#endif

#ifdef YYTYPE_INT8
typedef YYTYPE_INT8 yytype_int8;
#elif (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
typedef signed char yytype_int8;
#else
typedef short int yytype_int8;
#endif

#ifdef YYTYPE_UINT16
typedef YYTYPE_UINT16 yytype_uint16;
#else
typedef unsigned short int yytype_uint16;
#endif

#ifdef YYTYPE_INT16
typedef YYTYPE_INT16 yytype_int16;
#else
typedef short int yytype_int16;
#endif

#ifndef YYSIZE_T
# ifdef __SIZE_TYPE__
#  define YYSIZE_T __SIZE_TYPE__
# elif defined size_t
#  define YYSIZE_T size_t
# elif ! defined YYSIZE_T && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# else
#  define YYSIZE_T unsigned int
# endif
#endif

#define YYSIZE_MAXIMUM ((YYSIZE_T) -1)

#ifndef YY_
# if defined YYENABLE_NLS && YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> /* INFRINGES ON USER NAME SPACE */
#   define YY_(msgid) dgettext ("bison-runtime", msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(msgid) msgid
# endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YYUSE(e) ((void) (e))
#else
# define YYUSE(e) /* empty */
#endif

/* Identity function, used to suppress warnings about constant conditions.  */
#ifndef lint
# define YYID(n) (n)
#else
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static int
YYID (int i)
#else
static int
YYID (i)
    int i;
#endif
{
  return i;
}
#endif

#if ! defined yyoverflow || YYERROR_VERBOSE

/* The parser invokes alloca or malloc; define the necessary symbols.  */

# ifdef YYSTACK_USE_ALLOCA
#  if YYSTACK_USE_ALLOCA
#   ifdef __GNUC__
#    define YYSTACK_ALLOC __builtin_alloca
#   elif defined __BUILTIN_VA_ARG_INCR
#    include <alloca.h> /* INFRINGES ON USER NAME SPACE */
#   elif defined _AIX
#    define YYSTACK_ALLOC __alloca
#   elif defined _MSC_VER
#    include <malloc.h> /* INFRINGES ON USER NAME SPACE */
#    define alloca _alloca
#   else
#    define YYSTACK_ALLOC alloca
#    if ! defined _ALLOCA_H && ! defined _STDLIB_H && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
#     include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#     ifndef _STDLIB_H
#      define _STDLIB_H 1
#     endif
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's `empty if-body' warning.  */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (YYID (0))
#  ifndef YYSTACK_ALLOC_MAXIMUM
    /* The OS might guarantee only one guard page at the bottom of the stack,
       and a page size can be as small as 4096 bytes.  So we cannot safely
       invoke alloca (N) if N exceeds 4096.  Use a slightly smaller number
       to allow for a few compiler-allocated temporary stack slots.  */
#   define YYSTACK_ALLOC_MAXIMUM 4032 /* reasonable circa 2006 */
#  endif
# else
#  define YYSTACK_ALLOC YYMALLOC
#  define YYSTACK_FREE YYFREE
#  ifndef YYSTACK_ALLOC_MAXIMUM
#   define YYSTACK_ALLOC_MAXIMUM YYSIZE_MAXIMUM
#  endif
#  if (defined __cplusplus && ! defined _STDLIB_H \
       && ! ((defined YYMALLOC || defined malloc) \
	     && (defined YYFREE || defined free)))
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   ifndef _STDLIB_H
#    define _STDLIB_H 1
#   endif
#  endif
#  ifndef YYMALLOC
#   define YYMALLOC malloc
#   if ! defined malloc && ! defined _STDLIB_H && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
void *malloc (YYSIZE_T); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifndef YYFREE
#   define YYFREE free
#   if ! defined free && ! defined _STDLIB_H && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
void free (void *); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
# endif
#endif /* ! defined yyoverflow || YYERROR_VERBOSE */


#if (! defined yyoverflow \
     && (! defined __cplusplus \
	 || (defined YYSTYPE_IS_TRIVIAL && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  yytype_int16 yyss;
  YYSTYPE yyvs;
  };

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (sizeof (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (sizeof (yytype_int16) + sizeof (YYSTYPE)) \
      + YYSTACK_GAP_MAXIMUM)

/* Copy COUNT objects from FROM to TO.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined __GNUC__ && 1 < __GNUC__
#   define YYCOPY(To, From, Count) \
      __builtin_memcpy (To, From, (Count) * sizeof (*(From)))
#  else
#   define YYCOPY(To, From, Count)		\
      do					\
	{					\
	  YYSIZE_T yyi;				\
	  for (yyi = 0; yyi < (Count); yyi++)	\
	    (To)[yyi] = (From)[yyi];		\
	}					\
      while (YYID (0))
#  endif
# endif

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack)					\
    do									\
      {									\
	YYSIZE_T yynewbytes;						\
	YYCOPY (&yyptr->Stack, Stack, yysize);				\
	Stack = &yyptr->Stack;						\
	yynewbytes = yystacksize * sizeof (*Stack) + YYSTACK_GAP_MAXIMUM; \
	yyptr += yynewbytes / sizeof (*yyptr);				\
      }									\
    while (YYID (0))

#endif

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  66
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   162

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  75
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  17
/* YYNRULES -- Number of rules.  */
#define YYNRULES  84
/* YYNRULES -- Number of states.  */
#define YYNSTATES  153

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   329

#define YYTRANSLATE(YYX)						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const yytype_uint8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     1,     2,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    69,    70,    71,    72,    73,    74
};

#if YYDEBUG
/* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
   YYRHS.  */
static const yytype_uint8 yyprhs[] =
{
       0,     0,     3,     5,     8,    11,    14,    15,    17,    19,
      21,    23,    25,    27,    29,    31,    33,    35,    37,    39,
      41,    43,    45,    47,    49,    51,    53,    56,    60,    61,
      63,    65,    67,    69,    71,    73,    79,    83,    87,    91,
      93,    95,    99,   103,   107,   109,   113,   117,   121,   125,
     129,   133,   137,   141,   145,   149,   153,   157,   161,   165,
     169,   173,   177,   181,   185,   189,   193,   199,   202,   203,
     209,   214,   217,   219,   221,   223,   225,   228,   229,   232,
     233,   236,   241,   244,   245
};

/* YYRHS -- A `-1'-separated list of the rules' RHS.  */
static const yytype_int8 yyrhs[] =
{
      76,     0,    -1,    77,    -1,    82,    77,    -1,    81,    77,
      -1,    89,    77,    -1,    -1,    23,    -1,    24,    -1,    25,
      -1,    26,    -1,    27,    -1,    28,    -1,    29,    -1,    30,
      -1,    31,    -1,    32,    -1,    33,    -1,    34,    -1,    35,
      -1,    36,    -1,    37,    -1,    38,    -1,    39,    -1,    40,
      -1,    41,    -1,    79,    78,    -1,    79,    22,    78,    -1,
      -1,    11,    -1,    12,    -1,    13,    -1,    14,    -1,    15,
      -1,    16,    -1,    21,    19,    17,    79,    18,    -1,     7,
      19,    80,    -1,     4,    19,    20,    -1,     5,    19,    20,
      -1,     5,    -1,     6,    -1,    43,    19,    65,    -1,    44,
      19,    66,    -1,    45,    19,    20,    -1,    42,    -1,    46,
      19,    20,    -1,    47,    19,    20,    -1,    48,    19,    20,
      -1,    49,    19,    20,    -1,    50,    19,    20,    -1,    51,
      19,     9,    -1,    67,    19,    68,    -1,    67,    19,    69,
      -1,    52,    19,     9,    -1,    53,    19,     9,    -1,    54,
      19,    65,    -1,    55,    19,     9,    -1,    56,    19,     9,
      -1,    57,    19,     9,    -1,    58,    19,     9,    -1,    59,
      19,     9,    -1,    60,    19,     9,    -1,    61,    19,     9,
      -1,    62,    19,     9,    -1,    63,    19,     9,    -1,    64,
      19,     9,    -1,     3,     8,    17,    83,    18,    -1,    84,
      83,    -1,    -1,    20,     8,    17,    87,    18,    -1,    20,
      17,    87,    18,    -1,    20,     8,    -1,    20,    -1,    70,
      -1,    71,    -1,    72,    -1,    86,    85,    -1,    -1,    88,
      87,    -1,    -1,    86,     8,    -1,    73,    17,    90,    18,
      -1,    90,    91,    -1,    -1,    74,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   336,   336,   345,   346,   347,   348,   351,   352,   353,
     354,   355,   356,   357,   358,   359,   360,   361,   362,   363,
     364,   365,   366,   367,   368,   369,   372,   376,   380,   382,
     383,   384,   385,   386,   387,   390,   398,   405,   413,   425,
     435,   446,   452,   470,   474,   478,   482,   486,   510,   534,
     543,   547,   551,   555,   559,   563,   568,   572,   576,   580,
     584,   588,   592,   596,   600,   604,   609,   643,   644,   647,
     667,   688,   709,   733,   734,   735,   738,   742,   745,   746,
     749,   765,   771,   772,   775
};
#endif

#if YYDEBUG || YYERROR_VERBOSE || YYTOKEN_TABLE
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "SEGMENT", "EVENTLOGFILE",
  "EVENTTIMESTAMP", "EVENTPRECISETIMESTAMP", "EVENTPRIORITY", "IPADDR",
  "NUMBER", "COLON", "PDEBUG", "PINFO", "PWARNING", "PERROR", "PCRITICAL",
  "PFATAL", "OPENBRACE", "CLOSEBRACE", "EQUALS", "STRING", "DEBUGFLAGS",
  "BANG", "DDEBUG", "DEXIT", "DPRINT", "DDATA_LINK", "DNETWORK",
  "DPROTOCOL", "DSESSION", "DCONF", "DMEMB", "DFLOW_CONTROL", "DSTATUS",
  "DEVENTS", "DGROUPS", "DMEMORY", "DSKIPLIST", "DACM", "DSECURITY",
  "DALL", "DNONE", "DEBUGINITIALSEQUENCE", "DANGEROUSMONITOR",
  "SOCKETPORTREUSE", "RUNTIMEDIR", "SPUSER", "SPGROUP",
  "ALLOWEDAUTHMETHODS", "REQUIREDAUTHMETHODS", "ACCESSCONTROLPOLICY",
  "MAXSESSIONMESSAGES", "WINDOW", "PERSONALWINDOW", "ACCELERATEDRING",
  "ACCELERATEDWINDOW", "TOKENTIMEOUT", "HURRYTIMEOUT", "ALIVETIMEOUT",
  "JOINTIMEOUT", "REPTIMEOUT", "SEGTIMEOUT", "GATHERTIMEOUT",
  "FORMTIMEOUT", "LOOKUPTIMEOUT", "SP_BOOL", "SP_TRIVAL", "LINKPROTOCOL",
  "PHOP", "PTCPHOP", "IMONITOR", "ICLIENT", "IDAEMON", "ROUTEMATRIX",
  "LINKCOST", "$accept", "Config", "ConfigStructs", "AlarmBit",
  "AlarmBitComp", "PriorityLevel", "ParamStruct", "SegmentStruct",
  "Segmentparams", "Segmentparam", "IfType", "IfTypeComp",
  "Interfaceparams", "Interfaceparam", "RouteStruct", "Routevectors",
  "Routevector", 0
};
#endif

# ifdef YYPRINT
/* YYTOKNUM[YYLEX-NUM] -- Internal token number corresponding to
   token YYLEX-NUM.  */
static const yytype_uint16 yytoknum[] =
{
       0,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,   266,   267,   268,   269,   270,   271,   272,   273,   274,
     275,   276,   277,   278,   279,   280,   281,   282,   283,   284,
     285,   286,   287,   288,   289,   290,   291,   292,   293,   294,
     295,   296,   297,   298,   299,   300,   301,   302,   303,   304,
     305,   306,   307,   308,   309,   310,   311,   312,   313,   314,
     315,   316,   317,   318,   319,   320,   321,   322,   323,   324,
     325,   326,   327,   328,   329
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint8 yyr1[] =
{
       0,    75,    76,    77,    77,    77,    77,    78,    78,    78,
      78,    78,    78,    78,    78,    78,    78,    78,    78,    78,
      78,    78,    78,    78,    78,    78,    79,    79,    79,    80,
      80,    80,    80,    80,    80,    81,    81,    81,    81,    81,
      81,    81,    81,    81,    81,    81,    81,    81,    81,    81,
      81,    81,    81,    81,    81,    81,    81,    81,    81,    81,
      81,    81,    81,    81,    81,    81,    82,    83,    83,    84,
      84,    84,    84,    85,    85,    85,    86,    86,    87,    87,
      88,    89,    90,    90,    91
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     1,     2,     2,     2,     0,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     2,     3,     0,     1,
       1,     1,     1,     1,     1,     5,     3,     3,     3,     1,
       1,     3,     3,     3,     1,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     5,     2,     0,     5,
       4,     2,     1,     1,     1,     1,     2,     0,     2,     0,
       2,     4,     2,     0,     1
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
       6,     0,     0,    39,    40,     0,     0,    44,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     2,     6,     6,     6,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    83,     1,     4,     3,     5,
      68,    37,    38,    29,    30,    31,    32,    33,    34,    36,
      28,    41,    42,    43,    45,    46,    47,    48,    49,    50,
      53,    54,    55,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    65,    51,    52,     0,    72,     0,    68,     0,
      81,    84,    82,    71,    77,    66,    67,    35,     0,     7,
       8,     9,    10,    11,    12,    13,    14,    15,    16,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    77,
       0,     0,    77,    27,     0,    80,    73,    74,    75,    76,
      70,    78,    69
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,    32,    33,   138,   109,    79,    34,    35,   107,   108,
     149,   140,   141,   142,    36,   105,   112
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -135
static const yytype_int16 yypact[] =
{
      -3,     7,    -2,    19,  -135,    44,    46,  -135,    47,    48,
      49,    50,    57,    58,    85,    86,    88,    89,    90,    91,
      92,    93,    94,    95,    96,    97,    98,    99,   100,   101,
     102,   105,    75,  -135,    -3,    -3,    -3,   106,   104,   107,
      87,   108,    41,    60,   109,   110,   111,   112,   113,   114,
     119,   126,   127,    72,   129,   130,   131,   132,   133,   134,
     135,   136,   137,   138,   -55,  -135,  -135,  -135,  -135,  -135,
     128,  -135,  -135,  -135,  -135,  -135,  -135,  -135,  -135,  -135,
    -135,  -135,  -135,  -135,  -135,  -135,  -135,  -135,  -135,  -135,
    -135,  -135,  -135,  -135,  -135,  -135,  -135,  -135,  -135,  -135,
    -135,  -135,  -135,  -135,  -135,   -12,    -1,   139,   128,    56,
    -135,  -135,  -135,   141,   142,  -135,  -135,  -135,    -4,  -135,
    -135,  -135,  -135,  -135,  -135,  -135,  -135,  -135,  -135,  -135,
    -135,  -135,  -135,  -135,  -135,  -135,  -135,  -135,  -135,   142,
       1,   143,   142,  -135,   144,  -135,  -135,  -135,  -135,  -135,
    -135,  -135,  -135
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -135,  -135,   -24,    31,  -135,  -135,  -135,  -135,    42,  -135,
    -135,  -135,  -134,  -135,  -135,  -135,  -135
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -80
static const yytype_int16 yytable[] =
{
       1,     2,     3,     4,     5,   144,   110,   113,   151,   145,
      67,    68,    69,   103,   104,    37,   114,    38,     6,   119,
     120,   121,   122,   123,   124,   125,   126,   127,   128,   129,
     130,   131,   132,   133,   134,   135,   136,   137,    39,     7,
       8,     9,    10,    11,    12,    13,    14,    15,    16,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,    29,   111,    40,    30,    41,    42,    43,    44,    45,
      31,   146,   147,   148,   117,    66,    46,    47,   118,   119,
     120,   121,   122,   123,   124,   125,   126,   127,   128,   129,
     130,   131,   132,   133,   134,   135,   136,   137,    73,    74,
      75,    76,    77,    78,    48,    49,    81,    50,    51,    52,
      53,    54,    55,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    65,    70,    71,    80,    82,    72,    89,    83,
      84,    85,    86,    87,    88,    90,    91,    92,    93,    94,
      95,    96,    97,    98,    99,   100,   101,   102,   106,   143,
     116,     0,     0,     0,     0,     0,     0,   115,   139,     0,
     -79,   150,   152
};

static const yytype_int16 yycheck[] =
{
       3,     4,     5,     6,     7,   139,    18,     8,   142,     8,
      34,    35,    36,    68,    69,     8,    17,    19,    21,    23,
      24,    25,    26,    27,    28,    29,    30,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    19,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    74,    19,    67,    19,    19,    19,    19,    19,
      73,    70,    71,    72,    18,     0,    19,    19,    22,    23,
      24,    25,    26,    27,    28,    29,    30,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    11,    12,
      13,    14,    15,    16,    19,    19,    65,    19,    19,    19,
      19,    19,    19,    19,    19,    19,    19,    19,    19,    19,
      19,    19,    17,    17,    20,    17,    66,    20,     9,    20,
      20,    20,    20,    20,    20,     9,     9,    65,     9,     9,
       9,     9,     9,     9,     9,     9,     9,     9,    20,   118,
     108,    -1,    -1,    -1,    -1,    -1,    -1,    18,    17,    -1,
      18,    18,    18
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,     3,     4,     5,     6,     7,    21,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      67,    73,    76,    77,    81,    82,    89,     8,    19,    19,
      19,    19,    19,    19,    19,    19,    19,    19,    19,    19,
      19,    19,    19,    19,    19,    19,    19,    19,    19,    19,
      19,    19,    19,    19,    19,    17,     0,    77,    77,    77,
      17,    20,    20,    11,    12,    13,    14,    15,    16,    80,
      17,    65,    66,    20,    20,    20,    20,    20,    20,     9,
       9,     9,    65,     9,     9,     9,     9,     9,     9,     9,
       9,     9,     9,    68,    69,    90,    20,    83,    84,    79,
      18,    74,    91,     8,    17,    18,    83,    18,    22,    23,
      24,    25,    26,    27,    28,    29,    30,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    78,    17,
      86,    87,    88,    78,    87,     8,    70,    71,    72,    85,
      18,    87,    18
};

#define yyerrok		(yyerrstatus = 0)
#define yyclearin	(yychar = YYEMPTY)
#define YYEMPTY		(-2)
#define YYEOF		0

#define YYACCEPT	goto yyacceptlab
#define YYABORT		goto yyabortlab
#define YYERROR		goto yyerrorlab


/* Like YYERROR except do call yyerror.  This remains here temporarily
   to ease the transition to the new meaning of YYERROR, for GCC.
   Once GCC version 2 has supplanted version 1, this can go.  */

#define YYFAIL		goto yyerrlab

#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)					\
do								\
  if (yychar == YYEMPTY && yylen == 1)				\
    {								\
      yychar = (Token);						\
      yylval = (Value);						\
      yytoken = YYTRANSLATE (yychar);				\
      YYPOPSTACK (1);						\
      goto yybackup;						\
    }								\
  else								\
    {								\
      yyerror (YY_("syntax error: cannot back up")); \
      YYERROR;							\
    }								\
while (YYID (0))


#define YYTERROR	1
#define YYERRCODE	256


/* YYLLOC_DEFAULT -- Set CURRENT to span from RHS[1] to RHS[N].
   If N is 0, then set CURRENT to the empty location which ends
   the previous symbol: RHS[0] (always defined).  */

#define YYRHSLOC(Rhs, K) ((Rhs)[K])
#ifndef YYLLOC_DEFAULT
# define YYLLOC_DEFAULT(Current, Rhs, N)				\
    do									\
      if (YYID (N))                                                    \
	{								\
	  (Current).first_line   = YYRHSLOC (Rhs, 1).first_line;	\
	  (Current).first_column = YYRHSLOC (Rhs, 1).first_column;	\
	  (Current).last_line    = YYRHSLOC (Rhs, N).last_line;		\
	  (Current).last_column  = YYRHSLOC (Rhs, N).last_column;	\
	}								\
      else								\
	{								\
	  (Current).first_line   = (Current).last_line   =		\
	    YYRHSLOC (Rhs, 0).last_line;				\
	  (Current).first_column = (Current).last_column =		\
	    YYRHSLOC (Rhs, 0).last_column;				\
	}								\
    while (YYID (0))
#endif


/* YY_LOCATION_PRINT -- Print the location on the stream.
   This macro was not mandated originally: define only if we know
   we won't break user code: when these are the locations we know.  */

#ifndef YY_LOCATION_PRINT
# if defined YYLTYPE_IS_TRIVIAL && YYLTYPE_IS_TRIVIAL
#  define YY_LOCATION_PRINT(File, Loc)			\
     fprintf (File, "%d.%d-%d.%d",			\
	      (Loc).first_line, (Loc).first_column,	\
	      (Loc).last_line,  (Loc).last_column)
# else
#  define YY_LOCATION_PRINT(File, Loc) ((void) 0)
# endif
#endif


/* YYLEX -- calling `yylex' with the right arguments.  */

#ifdef YYLEX_PARAM
# define YYLEX yylex (YYLEX_PARAM)
#else
# define YYLEX yylex ()
#endif

/* Enable debugging if requested.  */
#if YYDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)			\
do {						\
  if (yydebug)					\
    YYFPRINTF Args;				\
} while (YYID (0))

# define YY_SYMBOL_PRINT(Title, Type, Value, Location)			  \
do {									  \
  if (yydebug)								  \
    {									  \
      YYFPRINTF (stderr, "%s ", Title);					  \
      yy_symbol_print (stderr,						  \
		  Type, Value); \
      YYFPRINTF (stderr, "\n");						  \
    }									  \
} while (YYID (0))


/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

/*ARGSUSED*/
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_symbol_value_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep)
#else
static void
yy_symbol_value_print (yyoutput, yytype, yyvaluep)
    FILE *yyoutput;
    int yytype;
    YYSTYPE const * const yyvaluep;
#endif
{
  if (!yyvaluep)
    return;
# ifdef YYPRINT
  if (yytype < YYNTOKENS)
    YYPRINT (yyoutput, yytoknum[yytype], *yyvaluep);
# else
  YYUSE (yyoutput);
# endif
  switch (yytype)
    {
      default:
	break;
    }
}


/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_symbol_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep)
#else
static void
yy_symbol_print (yyoutput, yytype, yyvaluep)
    FILE *yyoutput;
    int yytype;
    YYSTYPE const * const yyvaluep;
#endif
{
  if (yytype < YYNTOKENS)
    YYFPRINTF (yyoutput, "token %s (", yytname[yytype]);
  else
    YYFPRINTF (yyoutput, "nterm %s (", yytname[yytype]);

  yy_symbol_value_print (yyoutput, yytype, yyvaluep);
  YYFPRINTF (yyoutput, ")");
}

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_stack_print (yytype_int16 *bottom, yytype_int16 *top)
#else
static void
yy_stack_print (bottom, top)
    yytype_int16 *bottom;
    yytype_int16 *top;
#endif
{
  YYFPRINTF (stderr, "Stack now");
  for (; bottom <= top; ++bottom)
    YYFPRINTF (stderr, " %d", *bottom);
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)				\
do {								\
  if (yydebug)							\
    yy_stack_print ((Bottom), (Top));				\
} while (YYID (0))


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_reduce_print (YYSTYPE *yyvsp, int yyrule)
#else
static void
yy_reduce_print (yyvsp, yyrule)
    YYSTYPE *yyvsp;
    int yyrule;
#endif
{
  int yynrhs = yyr2[yyrule];
  int yyi;
  unsigned long int yylno = yyrline[yyrule];
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %lu):\n",
	     yyrule - 1, yylno);
  /* The symbols being reduced.  */
  for (yyi = 0; yyi < yynrhs; yyi++)
    {
      fprintf (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr, yyrhs[yyprhs[yyrule] + yyi],
		       &(yyvsp[(yyi + 1) - (yynrhs)])
		       		       );
      fprintf (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)		\
do {					\
  if (yydebug)				\
    yy_reduce_print (yyvsp, Rule); \
} while (YYID (0))

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !YYDEBUG */
# define YYDPRINTF(Args)
# define YY_SYMBOL_PRINT(Title, Type, Value, Location)
# define YY_STACK_PRINT(Bottom, Top)
# define YY_REDUCE_PRINT(Rule)
#endif /* !YYDEBUG */


/* YYINITDEPTH -- initial size of the parser's stacks.  */
#ifndef	YYINITDEPTH
# define YYINITDEPTH 200
#endif

/* YYMAXDEPTH -- maximum size the stacks can grow to (effective only
   if the built-in stack extension method is used).

   Do not make this value too large; the results are undefined if
   YYSTACK_ALLOC_MAXIMUM < YYSTACK_BYTES (YYMAXDEPTH)
   evaluated with infinite-precision integer arithmetic.  */

#ifndef YYMAXDEPTH
# define YYMAXDEPTH 10000
#endif



#if YYERROR_VERBOSE

# ifndef yystrlen
#  if defined __GLIBC__ && defined _STRING_H
#   define yystrlen strlen
#  else
/* Return the length of YYSTR.  */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static YYSIZE_T
yystrlen (const char *yystr)
#else
static YYSIZE_T
yystrlen (yystr)
    const char *yystr;
#endif
{
  YYSIZE_T yylen;
  for (yylen = 0; yystr[yylen]; yylen++)
    continue;
  return yylen;
}
#  endif
# endif

# ifndef yystpcpy
#  if defined __GLIBC__ && defined _STRING_H && defined _GNU_SOURCE
#   define yystpcpy stpcpy
#  else
/* Copy YYSRC to YYDEST, returning the address of the terminating '\0' in
   YYDEST.  */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static char *
yystpcpy (char *yydest, const char *yysrc)
#else
static char *
yystpcpy (yydest, yysrc)
    char *yydest;
    const char *yysrc;
#endif
{
  char *yyd = yydest;
  const char *yys = yysrc;

  while ((*yyd++ = *yys++) != '\0')
    continue;

  return yyd - 1;
}
#  endif
# endif

# ifndef yytnamerr
/* Copy to YYRES the contents of YYSTR after stripping away unnecessary
   quotes and backslashes, so that it's suitable for yyerror.  The
   heuristic is that double-quoting is unnecessary unless the string
   contains an apostrophe, a comma, or backslash (other than
   backslash-backslash).  YYSTR is taken from yytname.  If YYRES is
   null, do not copy; instead, return the length of what the result
   would have been.  */
static YYSIZE_T
yytnamerr (char *yyres, const char *yystr)
{
  if (*yystr == '"')
    {
      YYSIZE_T yyn = 0;
      char const *yyp = yystr;

      for (;;)
	switch (*++yyp)
	  {
	  case '\'':
	  case ',':
	    goto do_not_strip_quotes;

	  case '\\':
	    if (*++yyp != '\\')
	      goto do_not_strip_quotes;
	    /* Fall through.  */
	  default:
	    if (yyres)
	      yyres[yyn] = *yyp;
	    yyn++;
	    break;

	  case '"':
	    if (yyres)
	      yyres[yyn] = '\0';
	    return yyn;
	  }
    do_not_strip_quotes: ;
    }

  if (! yyres)
    return yystrlen (yystr);

  return yystpcpy (yyres, yystr) - yyres;
}
# endif

/* Copy into YYRESULT an error message about the unexpected token
   YYCHAR while in state YYSTATE.  Return the number of bytes copied,
   including the terminating null byte.  If YYRESULT is null, do not
   copy anything; just return the number of bytes that would be
   copied.  As a special case, return 0 if an ordinary "syntax error"
   message will do.  Return YYSIZE_MAXIMUM if overflow occurs during
   size calculation.  */
static YYSIZE_T
yysyntax_error (char *yyresult, int yystate, int yychar)
{
  int yyn = yypact[yystate];

  if (! (YYPACT_NINF < yyn && yyn <= YYLAST))
    return 0;
  else
    {
      int yytype = YYTRANSLATE (yychar);
      YYSIZE_T yysize0 = yytnamerr (0, yytname[yytype]);
      YYSIZE_T yysize = yysize0;
      YYSIZE_T yysize1;
      int yysize_overflow = 0;
      enum { YYERROR_VERBOSE_ARGS_MAXIMUM = 5 };
      char const *yyarg[YYERROR_VERBOSE_ARGS_MAXIMUM];
      int yyx;

# if 0
      /* This is so xgettext sees the translatable formats that are
	 constructed on the fly.  */
      YY_("syntax error, unexpected %s");
      YY_("syntax error, unexpected %s, expecting %s");
      YY_("syntax error, unexpected %s, expecting %s or %s");
      YY_("syntax error, unexpected %s, expecting %s or %s or %s");
      YY_("syntax error, unexpected %s, expecting %s or %s or %s or %s");
# endif
      char *yyfmt;
      char const *yyf;
      static char const yyunexpected[] = "syntax error, unexpected %s";
      static char const yyexpecting[] = ", expecting %s";
      static char const yyor[] = " or %s";
      char yyformat[sizeof yyunexpected
		    + sizeof yyexpecting - 1
		    + ((YYERROR_VERBOSE_ARGS_MAXIMUM - 2)
		       * (sizeof yyor - 1))];
      char const *yyprefix = yyexpecting;

      /* Start YYX at -YYN if negative to avoid negative indexes in
	 YYCHECK.  */
      int yyxbegin = yyn < 0 ? -yyn : 0;

      /* Stay within bounds of both yycheck and yytname.  */
      int yychecklim = YYLAST - yyn + 1;
      int yyxend = yychecklim < YYNTOKENS ? yychecklim : YYNTOKENS;
      int yycount = 1;

      yyarg[0] = yytname[yytype];
      yyfmt = yystpcpy (yyformat, yyunexpected);

      for (yyx = yyxbegin; yyx < yyxend; ++yyx)
	if (yycheck[yyx + yyn] == yyx && yyx != YYTERROR)
	  {
	    if (yycount == YYERROR_VERBOSE_ARGS_MAXIMUM)
	      {
		yycount = 1;
		yysize = yysize0;
		yyformat[sizeof yyunexpected - 1] = '\0';
		break;
	      }
	    yyarg[yycount++] = yytname[yyx];
	    yysize1 = yysize + yytnamerr (0, yytname[yyx]);
	    yysize_overflow |= (yysize1 < yysize);
	    yysize = yysize1;
	    yyfmt = yystpcpy (yyfmt, yyprefix);
	    yyprefix = yyor;
	  }

      yyf = YY_(yyformat);
      yysize1 = yysize + yystrlen (yyf);
      yysize_overflow |= (yysize1 < yysize);
      yysize = yysize1;

      if (yysize_overflow)
	return YYSIZE_MAXIMUM;

      if (yyresult)
	{
	  /* Avoid sprintf, as that infringes on the user's name space.
	     Don't have undefined behavior even if the translation
	     produced a string with the wrong number of "%s"s.  */
	  char *yyp = yyresult;
	  int yyi = 0;
	  while ((*yyp = *yyf) != '\0')
	    {
	      if (*yyp == '%' && yyf[1] == 's' && yyi < yycount)
		{
		  yyp += yytnamerr (yyp, yyarg[yyi++]);
		  yyf += 2;
		}
	      else
		{
		  yyp++;
		  yyf++;
		}
	    }
	}
      return yysize;
    }
}
#endif /* YYERROR_VERBOSE */


/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

/*ARGSUSED*/
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yydestruct (const char *yymsg, int yytype, YYSTYPE *yyvaluep)
#else
static void
yydestruct (yymsg, yytype, yyvaluep)
    const char *yymsg;
    int yytype;
    YYSTYPE *yyvaluep;
#endif
{
  YYUSE (yyvaluep);

  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yytype, yyvaluep, yylocationp);

  switch (yytype)
    {

      default:
	break;
    }
}


/* Prevent warnings from -Wmissing-prototypes.  */

#ifdef YYPARSE_PARAM
#if defined __STDC__ || defined __cplusplus
int yyparse (void *YYPARSE_PARAM);
#else
int yyparse ();
#endif
#else /* ! YYPARSE_PARAM */
#if defined __STDC__ || defined __cplusplus
int yyparse (void);
#else
int yyparse ();
#endif
#endif /* ! YYPARSE_PARAM */



/* The look-ahead symbol.  */
int yychar;

/* The semantic value of the look-ahead symbol.  */
YYSTYPE yylval;

/* Number of syntax errors so far.  */
int yynerrs;



/*----------.
| yyparse.  |
`----------*/

#ifdef YYPARSE_PARAM
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
int
yyparse (void *YYPARSE_PARAM)
#else
int
yyparse (YYPARSE_PARAM)
    void *YYPARSE_PARAM;
#endif
#else /* ! YYPARSE_PARAM */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
int
yyparse (void)
#else
int
yyparse ()

#endif
#endif
{
  
  int yystate;
  int yyn;
  int yyresult;
  /* Number of tokens to shift before error messages enabled.  */
  int yyerrstatus;
  /* Look-ahead token as an internal (translated) token number.  */
  int yytoken = 0;
#if YYERROR_VERBOSE
  /* Buffer for error messages, and its allocated size.  */
  char yymsgbuf[128];
  char *yymsg = yymsgbuf;
  YYSIZE_T yymsg_alloc = sizeof yymsgbuf;
#endif

  /* Three stacks and their tools:
     `yyss': related to states,
     `yyvs': related to semantic values,
     `yyls': related to locations.

     Refer to the stacks thru separate pointers, to allow yyoverflow
     to reallocate them elsewhere.  */

  /* The state stack.  */
  yytype_int16 yyssa[YYINITDEPTH];
  yytype_int16 *yyss = yyssa;
  yytype_int16 *yyssp;

  /* The semantic value stack.  */
  YYSTYPE yyvsa[YYINITDEPTH];
  YYSTYPE *yyvs = yyvsa;
  YYSTYPE *yyvsp;



#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N))

  YYSIZE_T yystacksize = YYINITDEPTH;

  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;


  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY;		/* Cause a token to be read.  */

  /* Initialize stack pointers.
     Waste one element of value and location stack
     so that they stay on the same level as the state stack.
     The wasted elements are never initialized.  */

  yyssp = yyss;
  yyvsp = yyvs;

  goto yysetstate;

/*------------------------------------------------------------.
| yynewstate -- Push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
 yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed.  So pushing a state here evens the stacks.  */
  yyssp++;

 yysetstate:
  *yyssp = yystate;

  if (yyss + yystacksize - 1 <= yyssp)
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYSIZE_T yysize = yyssp - yyss + 1;

#ifdef yyoverflow
      {
	/* Give user a chance to reallocate the stack.  Use copies of
	   these so that the &'s don't force the real ones into
	   memory.  */
	YYSTYPE *yyvs1 = yyvs;
	yytype_int16 *yyss1 = yyss;


	/* Each stack pointer address is followed by the size of the
	   data in use in that stack, in bytes.  This used to be a
	   conditional around just the two extra args, but that might
	   be undefined if yyoverflow is a macro.  */
	yyoverflow (YY_("memory exhausted"),
		    &yyss1, yysize * sizeof (*yyssp),
		    &yyvs1, yysize * sizeof (*yyvsp),

		    &yystacksize);

	yyss = yyss1;
	yyvs = yyvs1;
      }
#else /* no yyoverflow */
# ifndef YYSTACK_RELOCATE
      goto yyexhaustedlab;
# else
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
	goto yyexhaustedlab;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
	yystacksize = YYMAXDEPTH;

      {
	yytype_int16 *yyss1 = yyss;
	union yyalloc *yyptr =
	  (union yyalloc *) YYSTACK_ALLOC (YYSTACK_BYTES (yystacksize));
	if (! yyptr)
	  goto yyexhaustedlab;
	YYSTACK_RELOCATE (yyss);
	YYSTACK_RELOCATE (yyvs);

#  undef YYSTACK_RELOCATE
	if (yyss1 != yyssa)
	  YYSTACK_FREE (yyss1);
      }
# endif
#endif /* no yyoverflow */

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;


      YYDPRINTF ((stderr, "Stack size increased to %lu\n",
		  (unsigned long int) yystacksize));

      if (yyss + yystacksize - 1 <= yyssp)
	YYABORT;
    }

  YYDPRINTF ((stderr, "Entering state %d\n", yystate));

  goto yybackup;

/*-----------.
| yybackup.  |
`-----------*/
yybackup:

  /* Do appropriate processing given the current state.  Read a
     look-ahead token if we need one and don't already have one.  */

  /* First try to decide what to do without reference to look-ahead token.  */
  yyn = yypact[yystate];
  if (yyn == YYPACT_NINF)
    goto yydefault;

  /* Not known => get a look-ahead token if don't already have one.  */

  /* YYCHAR is either YYEMPTY or YYEOF or a valid look-ahead symbol.  */
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token: "));
      yychar = YYLEX;
    }

  if (yychar <= YYEOF)
    {
      yychar = yytoken = YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else
    {
      yytoken = YYTRANSLATE (yychar);
      YY_SYMBOL_PRINT ("Next token is", yytoken, &yylval, &yylloc);
    }

  /* If the proper action on seeing token YYTOKEN is to reduce or to
     detect an error, take that action.  */
  yyn += yytoken;
  if (yyn < 0 || YYLAST < yyn || yycheck[yyn] != yytoken)
    goto yydefault;
  yyn = yytable[yyn];
  if (yyn <= 0)
    {
      if (yyn == 0 || yyn == YYTABLE_NINF)
	goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }

  if (yyn == YYFINAL)
    YYACCEPT;

  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  /* Shift the look-ahead token.  */
  YY_SYMBOL_PRINT ("Shifting", yytoken, &yylval, &yylloc);

  /* Discard the shifted token unless it is eof.  */
  if (yychar != YYEOF)
    yychar = YYEMPTY;

  yystate = yyn;
  *++yyvsp = yylval;

  goto yynewstate;


/*-----------------------------------------------------------.
| yydefault -- do the default action for the current state.  |
`-----------------------------------------------------------*/
yydefault:
  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;
  goto yyreduce;


/*-----------------------------.
| yyreduce -- Do a reduction.  |
`-----------------------------*/
yyreduce:
  /* yyn is the number of a rule to reduce with.  */
  yylen = yyr2[yyn];

  /* If YYLEN is nonzero, implement the default value of the action:
     `$$ = $1'.

     Otherwise, the following line sets YYVAL to garbage.
     This behavior is undocumented and Bison
     users should not rely upon it.  Assigning to YYVAL
     unconditionally makes the parser a bit smaller, and it avoids a
     GCC warning that YYVAL may be used uninitialized.  */
  yyval = yyvsp[1-yylen];


  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
        case 2:
#line 337 "./config_parse.y"
    {
			  Config->num_segments = segments;
			  Config->num_total_procs = num_procs;
			  Alarm(CONF_SYS, "Finished configuration file.\n");
                          Alarmp( SPLOG_DEBUG, CONF_SYS, "config_parse.y:The full segment string is %d characters long:\n%s", strlen(ConfStringRep), ConfStringRep);
			}
    break;

  case 7:
#line 351 "./config_parse.y"
    { (yyval) = (yyvsp[(1) - (1)]); }
    break;

  case 8:
#line 352 "./config_parse.y"
    { (yyval) = (yyvsp[(1) - (1)]); }
    break;

  case 9:
#line 353 "./config_parse.y"
    { (yyval) = (yyvsp[(1) - (1)]); }
    break;

  case 10:
#line 354 "./config_parse.y"
    { (yyval) = (yyvsp[(1) - (1)]); }
    break;

  case 11:
#line 355 "./config_parse.y"
    { (yyval) = (yyvsp[(1) - (1)]); }
    break;

  case 12:
#line 356 "./config_parse.y"
    { (yyval) = (yyvsp[(1) - (1)]); }
    break;

  case 13:
#line 357 "./config_parse.y"
    { (yyval) = (yyvsp[(1) - (1)]); }
    break;

  case 14:
#line 358 "./config_parse.y"
    { (yyval) = (yyvsp[(1) - (1)]); }
    break;

  case 15:
#line 359 "./config_parse.y"
    { (yyval) = (yyvsp[(1) - (1)]); }
    break;

  case 16:
#line 360 "./config_parse.y"
    { (yyval) = (yyvsp[(1) - (1)]); }
    break;

  case 17:
#line 361 "./config_parse.y"
    { (yyval) = (yyvsp[(1) - (1)]); }
    break;

  case 18:
#line 362 "./config_parse.y"
    { (yyval) = (yyvsp[(1) - (1)]); }
    break;

  case 19:
#line 363 "./config_parse.y"
    { (yyval) = (yyvsp[(1) - (1)]); }
    break;

  case 20:
#line 364 "./config_parse.y"
    { (yyval) = (yyvsp[(1) - (1)]); }
    break;

  case 21:
#line 365 "./config_parse.y"
    { (yyval) = (yyvsp[(1) - (1)]); }
    break;

  case 22:
#line 366 "./config_parse.y"
    { (yyval) = (yyvsp[(1) - (1)]); }
    break;

  case 23:
#line 367 "./config_parse.y"
    { (yyval) = (yyvsp[(1) - (1)]); }
    break;

  case 24:
#line 368 "./config_parse.y"
    { (yyval) = (yyvsp[(1) - (1)]); }
    break;

  case 25:
#line 369 "./config_parse.y"
    { (yyval) = (yyvsp[(1) - (1)]); }
    break;

  case 26:
#line 373 "./config_parse.y"
    {
			  (yyval).mask = ((yyvsp[(1) - (2)]).mask | (yyvsp[(2) - (2)]).mask);
			}
    break;

  case 27:
#line 377 "./config_parse.y"
    {
			  (yyval).mask = ((yyvsp[(1) - (3)]).mask & ~((yyvsp[(3) - (3)]).mask));
			}
    break;

  case 28:
#line 380 "./config_parse.y"
    { (yyval).mask = NONE; }
    break;

  case 29:
#line 382 "./config_parse.y"
    { (yyval) = (yyvsp[(1) - (1)]); }
    break;

  case 30:
#line 383 "./config_parse.y"
    { (yyval) = (yyvsp[(1) - (1)]); }
    break;

  case 31:
#line 384 "./config_parse.y"
    { (yyval) = (yyvsp[(1) - (1)]); }
    break;

  case 32:
#line 385 "./config_parse.y"
    { (yyval) = (yyvsp[(1) - (1)]); }
    break;

  case 33:
#line 386 "./config_parse.y"
    { (yyval) = (yyvsp[(1) - (1)]); }
    break;

  case 34:
#line 387 "./config_parse.y"
    { (yyval) = (yyvsp[(1) - (1)]); }
    break;

  case 35:
#line 391 "./config_parse.y"
    {
			  if (! Alarm_get_interactive() ) {
                            Alarm_clear_types(ALL);
			    Alarm_set_types((yyvsp[(4) - (5)]).mask);
			    Alarm(CONF_SYS, "Set Alarm mask to: %x\n", Alarm_get_types());
                          }
			}
    break;

  case 36:
#line 399 "./config_parse.y"
    {
                            if (! Alarm_get_interactive() ) {
                                Alarm_set_priority((yyvsp[(3) - (3)]).number);
                            }
                        }
    break;

  case 37:
#line 406 "./config_parse.y"
    {
			  if (! Alarm_get_interactive() ) {
                            char file_buf[MAXPATHLEN];
                            expand_filename(file_buf, MAXPATHLEN, (yyvsp[(3) - (3)]).string);
                            Alarm_set_output(file_buf);
                          }
			}
    break;

  case 38:
#line 414 "./config_parse.y"
    {
			  if (! Alarm_get_interactive() ) {
                              strncpy(alarm_format, (yyvsp[(3) - (3)]).string, MAX_ALARM_FORMAT);
                              alarm_custom_format = 1;
                              if (alarm_precise) {
                                  Alarm_enable_timestamp_high_res(alarm_format);
                              } else {
                                  Alarm_enable_timestamp(alarm_format);
                              }
                          }
			}
    break;

  case 39:
#line 426 "./config_parse.y"
    {
			  if (! Alarm_get_interactive() ) {
                              if (alarm_precise) {
                                  Alarm_enable_timestamp_high_res(NULL);
                              } else {
                                  Alarm_enable_timestamp(NULL);
                              }
                          }
			}
    break;

  case 40:
#line 436 "./config_parse.y"
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
    break;

  case 41:
#line 447 "./config_parse.y"
    {
			  if (! Alarm_get_interactive() ) {
                            Conf_set_dangerous_monitor_state((yyvsp[(3) - (3)]).boolean);
                          }
			}
    break;

  case 42:
#line 453 "./config_parse.y"
    {
                            port_reuse state;
                            if ((yyvsp[(3) - (3)]).number == 1)
                            {
                                state = port_reuse_on;
                            }
                            else if ((yyvsp[(3) - (3)]).number == 0)
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
    break;

  case 43:
#line 471 "./config_parse.y"
    {
                            Conf_set_runtime_dir((yyvsp[(3) - (3)]).string);
                        }
    break;

  case 44:
#line 475 "./config_parse.y"
    {
                            Conf_set_debug_initial_sequence();
                        }
    break;

  case 45:
#line 479 "./config_parse.y"
    {
                            Conf_set_user((yyvsp[(3) - (3)]).string);
                        }
    break;

  case 46:
#line 483 "./config_parse.y"
    {
                            Conf_set_group((yyvsp[(3) - (3)]).string);
                        }
    break;

  case 47:
#line 487 "./config_parse.y"
    {
                            char auth_list[MAX_AUTH_LIST_LEN];
                            int i, len;
                            char *c_ptr;
                            if (!authentication_configured) {
                                Acm_auth_set_disabled("NULL");
                            }
                            authentication_configured = 1;

                            strncpy(auth_list, (yyvsp[(3) - (3)]).string, MAX_AUTH_LIST_LEN);
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
    break;

  case 48:
#line 511 "./config_parse.y"
    {
                            char auth_list[MAX_AUTH_LIST_LEN];
                            int i, len;
                            char *c_ptr;
                            if (!authentication_configured) {
                                Acm_auth_set_disabled("NULL");
                            }
                            authentication_configured = 1;

                            strncpy(auth_list, (yyvsp[(3) - (3)]).string, MAX_AUTH_LIST_LEN);
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
    break;

  case 49:
#line 535 "./config_parse.y"
    {
                            int ret;
                            ret = Acm_acp_set_policy((yyvsp[(3) - (3)]).string);
                            if (!ret)
                            {
                                    yyerror("Invalid Access Control Policy name. Make sure it is spelled right and any needed mocdules are loaded");
                            }
                        }
    break;

  case 50:
#line 544 "./config_parse.y"
    {
                            Conf_set_max_session_messages((yyvsp[(3) - (3)]).number);
			}
    break;

  case 51:
#line 548 "./config_parse.y"
    {
                            Conf_set_link_protocol(HOP_PROT);
			}
    break;

  case 52:
#line 552 "./config_parse.y"
    {
                            Conf_set_link_protocol(TCP_PROT);
			}
    break;

  case 53:
#line 556 "./config_parse.y"
    {
			    Conf_set_window((yyvsp[(3) - (3)]).number);
			}
    break;

  case 54:
#line 560 "./config_parse.y"
    {
			    Conf_set_personal_window((yyvsp[(3) - (3)]).number);
			}
    break;

  case 55:
#line 564 "./config_parse.y"
    {
			    Conf_set_accelerated_ring_flag(TRUE);
			    Conf_set_accelerated_ring((yyvsp[(3) - (3)]).boolean);
			}
    break;

  case 56:
#line 569 "./config_parse.y"
    {
			    Conf_set_accelerated_window((yyvsp[(3) - (3)]).number);
			}
    break;

  case 57:
#line 573 "./config_parse.y"
    {
			    Conf_set_token_timeout((yyvsp[(3) - (3)]).number);
			}
    break;

  case 58:
#line 577 "./config_parse.y"
    {
			    Conf_set_hurry_timeout((yyvsp[(3) - (3)]).number);
			}
    break;

  case 59:
#line 581 "./config_parse.y"
    {
			    Conf_set_alive_timeout((yyvsp[(3) - (3)]).number);
			}
    break;

  case 60:
#line 585 "./config_parse.y"
    {
			    Conf_set_join_timeout((yyvsp[(3) - (3)]).number);
			}
    break;

  case 61:
#line 589 "./config_parse.y"
    {
			    Conf_set_rep_timeout((yyvsp[(3) - (3)]).number);
			}
    break;

  case 62:
#line 593 "./config_parse.y"
    {
			    Conf_set_seg_timeout((yyvsp[(3) - (3)]).number);
			}
    break;

  case 63:
#line 597 "./config_parse.y"
    {
			    Conf_set_gather_timeout((yyvsp[(3) - (3)]).number);
			}
    break;

  case 64:
#line 601 "./config_parse.y"
    {
			    Conf_set_form_timeout((yyvsp[(3) - (3)]).number);
			}
    break;

  case 65:
#line 605 "./config_parse.y"
    {
			    Conf_set_lookup_timeout((yyvsp[(3) - (3)]).number);
			}
    break;

  case 66:
#line 610 "./config_parse.y"
    { int i;
                          int added_len;
                          SEGMENT_CHECK( segments, inet_ntoa((yyvsp[(2) - (5)]).ip.addr) );
			  Config->segments[segments].num_procs = segment_procs;
			  Config->segments[segments].port = (yyvsp[(2) - (5)]).ip.port;
			  Config->segments[segments].bcast_address =
			    (yyvsp[(2) - (5)]).ip.addr.s_addr;
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
    break;

  case 69:
#line 648 "./config_parse.y"
    { 
                          PROC_NAME_CHECK( (yyvsp[(1) - (5)]).string );
                          PROCS_CHECK( num_procs, (yyvsp[(1) - (5)]).string );
                          SEGMENT_CHECK( segments, (yyvsp[(1) - (5)]).string );
                          SEGMENT_SIZE_CHECK( segment_procs, (yyvsp[(1) - (5)]).string );
                          if (procs_interfaces == 0)
                                  yyerror("Interfaces section declared but no actual interface addresses defined\n");
                          strcpy(Config->allprocs[num_procs].name, (yyvsp[(1) - (5)]).string);
                          Config->allprocs[num_procs].id = (yyvsp[(2) - (5)]).ip.addr.s_addr;
 		          Config->allprocs[num_procs].port = (yyvsp[(2) - (5)]).ip.port;
			  Config->allprocs[num_procs].seg_index = segments;
			  Config->allprocs[num_procs].index_in_seg = segment_procs;
                          Config->allprocs[num_procs].num_if = procs_interfaces;
			  Config->segments[segments].procs[segment_procs] = 
                              &(Config->allprocs[num_procs]);
			  num_procs++;
			  segment_procs++;
                          procs_interfaces = 0;
			}
    break;

  case 70:
#line 668 "./config_parse.y"
    { 
                          PROC_NAME_CHECK( (yyvsp[(1) - (4)]).string );
                          PROCS_CHECK( num_procs, (yyvsp[(1) - (4)]).string );
                          SEGMENT_CHECK( segments, (yyvsp[(1) - (4)]).string );
                          SEGMENT_SIZE_CHECK( segment_procs, (yyvsp[(1) - (4)]).string );
                          if (procs_interfaces == 0)
                                  yyerror("Interfaces section declared but no actual interface addresses defined\n");
                          strcpy(Config->allprocs[num_procs].name, (yyvsp[(1) - (4)]).string);
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
    break;

  case 71:
#line 689 "./config_parse.y"
    { 
                          PROC_NAME_CHECK( (yyvsp[(1) - (2)]).string );
                          PROCS_CHECK( num_procs, (yyvsp[(1) - (2)]).string );
                          SEGMENT_CHECK( segments, (yyvsp[(1) - (2)]).string );
                          SEGMENT_SIZE_CHECK( segment_procs, (yyvsp[(1) - (2)]).string );
                          strcpy(Config->allprocs[num_procs].name, (yyvsp[(1) - (2)]).string);
                          Config->allprocs[num_procs].id = (yyvsp[(2) - (2)]).ip.addr.s_addr;
 		          Config->allprocs[num_procs].port = (yyvsp[(2) - (2)]).ip.port;
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
    break;

  case 72:
#line 710 "./config_parse.y"
    { 
                          PROC_NAME_CHECK( (yyvsp[(1) - (1)]).string );
                          PROCS_CHECK( num_procs, (yyvsp[(1) - (1)]).string );
                          SEGMENT_CHECK( segments, (yyvsp[(1) - (1)]).string );
                          SEGMENT_SIZE_CHECK( segment_procs, (yyvsp[(1) - (1)]).string );
                          strcpy(Config->allprocs[num_procs].name, (yyvsp[(1) - (1)]).string);
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
    break;

  case 73:
#line 733 "./config_parse.y"
    { (yyval) = (yyvsp[(1) - (1)]); }
    break;

  case 74:
#line 734 "./config_parse.y"
    { (yyval) = (yyvsp[(1) - (1)]); }
    break;

  case 75:
#line 735 "./config_parse.y"
    { (yyval) = (yyvsp[(1) - (1)]); }
    break;

  case 76:
#line 739 "./config_parse.y"
    {
			  (yyval).mask = ((yyvsp[(1) - (2)]).mask | (yyvsp[(2) - (2)]).mask);
			}
    break;

  case 77:
#line 742 "./config_parse.y"
    { (yyval).mask = 0; }
    break;

  case 80:
#line 750 "./config_parse.y"
    { 
                          PROCS_CHECK( num_procs, (yyvsp[(1) - (2)]).string );
                          SEGMENT_CHECK( segments, (yyvsp[(1) - (2)]).string );
                          SEGMENT_SIZE_CHECK( segment_procs, (yyvsp[(1) - (2)]).string );
                          INTERFACE_NUM_CHECK( procs_interfaces, (yyvsp[(1) - (2)]).string );
                          Config->allprocs[num_procs].ifc[procs_interfaces].ip = (yyvsp[(2) - (2)]).ip.addr.s_addr;
                          Config->allprocs[num_procs].ifc[procs_interfaces].port = (yyvsp[(2) - (2)]).ip.port;
                          if ((yyvsp[(1) - (2)]).mask == 0)
                                  Config->allprocs[num_procs].ifc[procs_interfaces].type = IFTYPE_ALL;
                          else 
                                  Config->allprocs[num_procs].ifc[procs_interfaces].type = (yyvsp[(1) - (2)]).mask;
                          procs_interfaces++;
			}
    break;

  case 81:
#line 766 "./config_parse.y"
    { 
			  Alarm(CONF_SYS, "Successfully configured Routing Matrix for %d Segments with %d rows in the routing matrix\n",segments, rvec_num);
			}
    break;

  case 84:
#line 776 "./config_parse.y"
    { 
                                int rvec_element;
                                for (rvec_element = 0; rvec_element < segments; rvec_element++) {
                                        if ((yyvsp[(1) - (1)]).cost[rvec_element] < 0) yyerror("Wrong number of entries for routing matrix");
                                        LinkWeights[rvec_num][rvec_element] = (yyvsp[(1) - (1)]).cost[rvec_element];
                                }
                                rvec_num++;
                        }
    break;


/* Line 1267 of yacc.c.  */
#line 2577 "y.tab.c"
      default: break;
    }
  YY_SYMBOL_PRINT ("-> $$ =", yyr1[yyn], &yyval, &yyloc);

  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);

  *++yyvsp = yyval;


  /* Now `shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */

  yyn = yyr1[yyn];

  yystate = yypgoto[yyn - YYNTOKENS] + *yyssp;
  if (0 <= yystate && yystate <= YYLAST && yycheck[yystate] == *yyssp)
    yystate = yytable[yystate];
  else
    yystate = yydefgoto[yyn - YYNTOKENS];

  goto yynewstate;


/*------------------------------------.
| yyerrlab -- here on detecting error |
`------------------------------------*/
yyerrlab:
  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
#if ! YYERROR_VERBOSE
      yyerror (YY_("syntax error"));
#else
      {
	YYSIZE_T yysize = yysyntax_error (0, yystate, yychar);
	if (yymsg_alloc < yysize && yymsg_alloc < YYSTACK_ALLOC_MAXIMUM)
	  {
	    YYSIZE_T yyalloc = 2 * yysize;
	    if (! (yysize <= yyalloc && yyalloc <= YYSTACK_ALLOC_MAXIMUM))
	      yyalloc = YYSTACK_ALLOC_MAXIMUM;
	    if (yymsg != yymsgbuf)
	      YYSTACK_FREE (yymsg);
	    yymsg = (char *) YYSTACK_ALLOC (yyalloc);
	    if (yymsg)
	      yymsg_alloc = yyalloc;
	    else
	      {
		yymsg = yymsgbuf;
		yymsg_alloc = sizeof yymsgbuf;
	      }
	  }

	if (0 < yysize && yysize <= yymsg_alloc)
	  {
	    (void) yysyntax_error (yymsg, yystate, yychar);
	    yyerror (yymsg);
	  }
	else
	  {
	    yyerror (YY_("syntax error"));
	    if (yysize != 0)
	      goto yyexhaustedlab;
	  }
      }
#endif
    }



  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse look-ahead token after an
	 error, discard it.  */

      if (yychar <= YYEOF)
	{
	  /* Return failure if at end of input.  */
	  if (yychar == YYEOF)
	    YYABORT;
	}
      else
	{
	  yydestruct ("Error: discarding",
		      yytoken, &yylval);
	  yychar = YYEMPTY;
	}
    }

  /* Else will try to reuse look-ahead token after shifting the error
     token.  */
  goto yyerrlab1;


/*---------------------------------------------------.
| yyerrorlab -- error raised explicitly by YYERROR.  |
`---------------------------------------------------*/
yyerrorlab:

  /* Pacify compilers like GCC when the user code never invokes
     YYERROR and the label yyerrorlab therefore never appears in user
     code.  */
  if (/*CONSTCOND*/ 0)
     goto yyerrorlab;

  /* Do not reclaim the symbols of the rule which action triggered
     this YYERROR.  */
  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);
  yystate = *yyssp;
  goto yyerrlab1;


/*-------------------------------------------------------------.
| yyerrlab1 -- common code for both syntax error and YYERROR.  |
`-------------------------------------------------------------*/
yyerrlab1:
  yyerrstatus = 3;	/* Each real token shifted decrements this.  */

  for (;;)
    {
      yyn = yypact[yystate];
      if (yyn != YYPACT_NINF)
	{
	  yyn += YYTERROR;
	  if (0 <= yyn && yyn <= YYLAST && yycheck[yyn] == YYTERROR)
	    {
	      yyn = yytable[yyn];
	      if (0 < yyn)
		break;
	    }
	}

      /* Pop the current state because it cannot handle the error token.  */
      if (yyssp == yyss)
	YYABORT;


      yydestruct ("Error: popping",
		  yystos[yystate], yyvsp);
      YYPOPSTACK (1);
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  if (yyn == YYFINAL)
    YYACCEPT;

  *++yyvsp = yylval;


  /* Shift the error token.  */
  YY_SYMBOL_PRINT ("Shifting", yystos[yyn], yyvsp, yylsp);

  yystate = yyn;
  goto yynewstate;


/*-------------------------------------.
| yyacceptlab -- YYACCEPT comes here.  |
`-------------------------------------*/
yyacceptlab:
  yyresult = 0;
  goto yyreturn;

/*-----------------------------------.
| yyabortlab -- YYABORT comes here.  |
`-----------------------------------*/
yyabortlab:
  yyresult = 1;
  goto yyreturn;

#ifndef yyoverflow
/*-------------------------------------------------.
| yyexhaustedlab -- memory exhaustion comes here.  |
`-------------------------------------------------*/
yyexhaustedlab:
  yyerror (YY_("memory exhausted"));
  yyresult = 2;
  /* Fall through.  */
#endif

yyreturn:
  if (yychar != YYEOF && yychar != YYEMPTY)
     yydestruct ("Cleanup: discarding lookahead",
		 yytoken, &yylval);
  /* Do not reclaim the symbols of the rule which action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (yylen);
  YY_STACK_PRINT (yyss, yyssp);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
		  yystos[*yyssp], yyvsp);
      YYPOPSTACK (1);
    }
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif
#if YYERROR_VERBOSE
  if (yymsg != yymsgbuf)
    YYSTACK_FREE (yymsg);
#endif
  /* Make sure YYID is used.  */
  return YYID (yyresult);
}


#line 785 "./config_parse.y"

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

