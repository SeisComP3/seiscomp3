/* A Bison parser, made by GNU Bison 2.3.  */

/* Skeleton interface for Bison's Yacc-like parsers in C

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
     TOKENTIMEOUT = 309,
     HURRYTIMEOUT = 310,
     ALIVETIMEOUT = 311,
     JOINTIMEOUT = 312,
     REPTIMEOUT = 313,
     SEGTIMEOUT = 314,
     GATHERTIMEOUT = 315,
     FORMTIMEOUT = 316,
     LOOKUPTIMEOUT = 317,
     SP_BOOL = 318,
     SP_TRIVAL = 319,
     LINKPROTOCOL = 320,
     PHOP = 321,
     PTCPHOP = 322,
     IMONITOR = 323,
     ICLIENT = 324,
     IDAEMON = 325,
     ROUTEMATRIX = 326,
     LINKCOST = 327
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
#define TOKENTIMEOUT 309
#define HURRYTIMEOUT 310
#define ALIVETIMEOUT 311
#define JOINTIMEOUT 312
#define REPTIMEOUT 313
#define SEGTIMEOUT 314
#define GATHERTIMEOUT 315
#define FORMTIMEOUT 316
#define LOOKUPTIMEOUT 317
#define SP_BOOL 318
#define SP_TRIVAL 319
#define LINKPROTOCOL 320
#define PHOP 321
#define PTCPHOP 322
#define IMONITOR 323
#define ICLIENT 324
#define IDAEMON 325
#define ROUTEMATRIX 326
#define LINKCOST 327




#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef int YYSTYPE;
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif

extern YYSTYPE yylval;

