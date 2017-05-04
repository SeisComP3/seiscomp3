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
#include <string.h>

#ifndef  ARCH_PC_WIN95
#include <sys/types.h>
#include <netinet/in.h>

#else /* ARCH_PC_WIN95 */

#include <winsock.h>
#define YY_NO_UNISTD_H  /* Do not include unistd.h header in windows compiles */

#endif /* ARCH_PC_WIN95 */

#include "conf_body.h"
#include "y.tab.h"

extern int line_num;
extern int semantic_errors;
extern int yyerror(char *str);
extern void yywarn(char *str);
%}
qstring \"[^\"]*\"|\'[^\']*\'
string  [^ \t\r\n#{}]+
true    [Tt][Rr][Uu][Ee]
yes     [Yy][Ee][Ss]
on      [Oo][Nn]
false   [Ff][Aa][Ll][Ss][Ee]
no      [Nn][Oo]
off     [Oo][Ff][Ff]
auto    [Aa][Uu][Tt][Oo]
ipaddr  [0-9]{1,3}\.[0-9]{1,3}\.[0-9]{1,3}\.[0-9]{1,3}
ipport  [0-9]{1,3}\.[0-9]{1,3}\.[0-9]{1,3}\.[0-9]{1,3}:[0-9]{1,5}
%option noyywrap
%%
#.*                             {} /* Comments */
[ \t\r]                         {} /* White space */
\n                              { line_num++;}
"{"                             { return OPENBRACE; }
"}"                             { return CLOSEBRACE; }
"="                             { return EQUALS; }
":"                             { return COLON; }
!/.+ 				{ return BANG; }
Spread_Segment                  { return SEGMENT; }
EventLogFile                    { return EVENTLOGFILE; }
EventTimeStamp                  { return EVENTTIMESTAMP; }
EventPreciseTimeStamp           { return EVENTPRECISETIMESTAMP; }
EventPriority                   { return EVENTPRIORITY; }
DebugFlags                      { return DEBUGFLAGS; }
DangerousMonitor                { return DANGEROUSMONITOR; }
SocketPortReuse                 { return SOCKETPORTREUSE; }
DebugInitialSequence            { return DEBUGINITIALSEQUENCE; }
RuntimeDir                      { return RUNTIMEDIR; }
DaemonUser                      { return SPUSER; }
DaemonGroup                     { return SPGROUP; }
RequiredAuthMethods             { return REQUIREDAUTHMETHODS; }
AllowedAuthMethods              { return ALLOWEDAUTHMETHODS; }
AccessControlPolicy             { return ACCESSCONTROLPOLICY; }
MaxSessionMessages              { return MAXSESSIONMESSAGES; }
LinkProtocol                    { return LINKPROTOCOL; }
Hop                             { return PHOP; }
TcpHop                          { return PTCPHOP; }
RouteMatrix                     { return ROUTEMATRIX; }
Window				{ return WINDOW; }
PersonalWindow			{ return PERSONALWINDOW; }
AcceleratedRing                 { return ACCELERATEDRING; }
AcceleratedWindow               { return ACCELERATEDWINDOW; }
TokenTimeout                    { return TOKENTIMEOUT; }
HurryTimeout                    { return HURRYTIMEOUT; }
AliveTimeout                    { return ALIVETIMEOUT; }
JoinTimeout                     { return JOINTIMEOUT; }
RepTimeout                      { return REPTIMEOUT; }
SegTimeout                      { return SEGTIMEOUT; }
GatherTimeout                   { return GATHERTIMEOUT; }
FormTimeout                     { return FORMTIMEOUT; }
LookupTimeout                   { return LOOKUPTIMEOUT; }
{true}|{yes}                    { yylval.boolean = TRUE; return SP_BOOL; }
{false}|{no}                    { yylval.boolean = FALSE; return SP_BOOL; }
{on}                            { yylval.number = 1; return SP_TRIVAL; }
{off}                           { yylval.number = 0; return SP_TRIVAL; }
{auto}                          { yylval.number = 2; return SP_TRIVAL; }
DEBUG	 			{ yylval.mask = 0x00000001; return DDEBUG; }
EXIT				{ yylval.mask = 0x00000002; return DEXIT; }
PRINT				{ yylval.mask = 0x00000004; return DPRINT; }
DATA_LINK			{ yylval.mask = 0x00000010; return DDATA_LINK; }
NETWORK				{ yylval.mask = 0x00000020; return DNETWORK; }
PROTOCOL			{ yylval.mask = 0x00000040; return DPROTOCOL; }
SESSION				{ yylval.mask = 0x00000080; return DSESSION; }
CONFIGURATION			{ yylval.mask = 0x00000100; return DCONF; }
MEMBERSHIP			{ yylval.mask = 0x00000200; return DMEMB; }
FLOW_CONTROL			{ yylval.mask = 0x00000400; return DFLOW_CONTROL; }
STATUS				{ yylval.mask = 0x00000800; return DSTATUS; }
EVENTS				{ yylval.mask = 0x00001000; return DEVENTS; }
GROUPS				{ yylval.mask = 0x00002000; return DGROUPS; }
MEMORY				{ yylval.mask = 0x00010000; return DMEMORY; }
SKIPLIST			{ yylval.mask = 0x00200000; return DSKIPLIST; }
ACM     			{ yylval.mask = 0x00400000; return DACM; }
SECURITY     			{ yylval.mask = 0x00800000; return DSECURITY; }
ALL				{ yylval.mask = 0xffffffff; return DALL; }
NONE				{ yylval.mask = 0x00000000; return DNONE; }
M                               { yylval.mask = IFTYPE_MONITOR; return IMONITOR; }
C                               { yylval.mask = IFTYPE_CLIENT; return ICLIENT; }
D                               { yylval.mask = IFTYPE_DAEMON; return IDAEMON; }
pDEBUG                          { yylval.number = 1; return PDEBUG; }
INFO                            { yylval.number = 2; return PINFO; }
WARNING                         { yylval.number = 3; return PWARNING; }
ERROR                           { yylval.number = 4; return PERROR; }
CRITICAL                        { yylval.number = 5; return PCRITICAL; }
FATAL                           { yylval.number = 6; return PFATAL; }

{ipport}                        { struct in_addr inaddr;
                                  int a1,a2,a3,a4,a5;
                                  char *c1,*c2,*c3,*c4,*c5;
                                  c1=strdup(yytext);
                                  c2=strchr(c1, '.'); *(c2++)=0;
                                  c3=strchr(c2, '.'); *(c3++)=0;
                                  c4=strchr(c3, '.'); *(c4++)=0;
                                  c5=strchr(c4, ':'); *(c5++)=0;
                                  a1=atoi(c1); a2=atoi(c2);
                                  a3=atoi(c3); a4=atoi(c4);
                                  a5=atoi(c5);
                                  free(c1);
                                  if ((a1 < 0) || (a1 > 255) ||
				      (a2 < 0) || (a2 > 255) ||
				      (a3 < 0) || (a3 > 255) ||
				      (a4 < 0) || (a4 > 255))
                                    yyerror("Invalid IP");
                                  inaddr.s_addr =
                                      ((a1<<24)|(a2<<16)|(a3<<8)|a4);
                                  yylval.ip.addr = inaddr;
                                  yylval.ip.port = a5;
                                  return IPADDR;
                                }
{ipaddr}                        { struct in_addr inaddr;
                                  int a1,a2,a3,a4;
                                  char *c1,*c2,*c3,*c4;
                                  c1=strdup(yytext);
                                  c2=strchr(c1, '.'); *(c2++)=0;
                                  c3=strchr(c2, '.'); *(c3++)=0;
                                  c4=strchr(c3, '.'); *(c4++)=0;
                                  a1=atoi(c1); a2=atoi(c2);
                                  a3=atoi(c3); a4=atoi(c4);
                                  free(c1);
                                  if ((a1 < 0) || (a1 > 255) ||
				      (a2 < 0) || (a2 > 255) ||
				      (a3 < 0) || (a3 > 255) ||
				      (a4 < 0) || (a4 > 255))
                                    yyerror("Invalid IP");
                                  inaddr.s_addr =
                                      ((a1<<24)|(a2<<16)|(a3<<8)|a4);
                                  yylval.ip.addr = inaddr;
                                  yylval.ip.port = 0;
                                  return IPADDR;
                                }
[0-9]{1,6}                   {
                                  yylval.number = atoi(yytext);
                                  return NUMBER;
                             }
([0-9]{1,3}[ \t]*)+[ \t]*    { 
                                  int fcost, i, done;
                                  char *c;
                                  char *ccur;
                                  i = 0;
                                  done = 0;
                                  c = ccur = strdup(yytext);
                                  while(!done) {
                                          fcost = atoi(ccur);
                                          if (fcost < 0) yyerror("Invalid Negative Cost");
                                          if (fcost > 100) {
                                                  yywarn("Cost clamped to 100");
                                                  fcost = 100;
                                          }
                                          yylval.cost[i] = fcost;
                                          i++;
                                          ccur = strchr(ccur, ' '); 
                                          if (ccur == NULL) done = 1;
                                          else ccur++;
                                  }
                                  free(c);
                                  for( ; i < MAX_SEGMENTS; i++) {
                                          yylval.cost[i] = -1;
                                  }
                                  return LINKCOST;
                                }
{qstring}                       { int l = strlen(yytext);
                                  yytext[l-1] = 0;
                                  yylval.string = strdup(yytext+1);
                                  return STRING; }
{string}                        { yylval.string = strdup(yytext);
                                  return STRING; }
%%
