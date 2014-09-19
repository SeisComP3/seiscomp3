
/*
 * SccsId:	@(#)pfile.h	43.1	9/9/91
 */

/*
 *  PFILE.H
 *  
 *  A header file to be included with programs that call the
 *  parameter-file subroutine package.
 */

#define PFILE_H

typedef double pdb_time;
typedef double tim_t;

typedef struct {
	char pname[10];
	short ptype;
	short preps;
	short pindflag;
} pdef_t;

typedef struct {
	float freal;
	float fimag;
} fcomplx_t;

typedef struct {
	double dreal;
	double dimag;
} dcomplx_t;

#define TRUE	1
#define FALSE	0

/* the following values should be returned to the shell on exiting */

#define SHOK	0
#define SHERR	-1

#define SUCCESS	1
#define BADNREC	0
#define DELNREC (-2)
#define NOTFOUND 0
#define ERROR	(-1)
#define LERROR	(-1L)
#define LNOMATCH (-2L)
#define NOMATCH	(-2)
#define OVERFLOW (-3)

#define FAILED	(-1L)

#define EXACT	0
#define NEAREST	1

#define	NTYPES	8
#define CHAR	0
#define SHORT	1
#define LONG	2
#define FLOAT	3
#define DOUBLE	4
#define TIME	5
#define FCOMPLX	6
#define DCOMPLX	7

#define DIR	0
#define IND	1
#define RorW    2

#define MAGIC	(short)0164646	/* this number identifies a parameter file */

#define picreat	prcreat
#define pigetp	prgetp
#define pigetc	prgetc
#define pigetr	prgetr
#define pidelr	prrmr
#define pindex	pinox

/* defines for NULL values */

#define NULLC	'\377'
#define NULLS	(short)0100000
#define NULLL	(long)020000000000L
#define NULLF	170141173319264427000000000000000000000.
#define NULLD	NULLF
#define NULLT	NULLF
extern fcomplx_t NULLFX;
extern dcomplx_t NULLDX;


/* typedef for intermediate time structure */

typedef struct {
	short tyr;
	short tdoy;
	short tmon;
	short tdom;
	short thr;
	short tmin;
	short tsec;
	float tfract;
} intime_t;

#define LEAP(year) (year%4==0 && year%100 != 0 || year%400==0)
#define EQNULLFX(complx) ((complx.freal==NULLF)&&(complx.fimag==NULLF))
#define EQNULLDX(complx) ((complx.dreal==NULLD)&&(complx.dimag==NULLD))

char *prgetc();
long prappr();
long prrif();
long pimax();
long pimin();
long pinewr();
long pirec();
long pistep();
long p_step();
pdb_time maketime();
char *timestr();
