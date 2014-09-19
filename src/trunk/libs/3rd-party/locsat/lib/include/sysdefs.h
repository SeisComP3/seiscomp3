#ifndef SYSDEFS_H
#define SYSDEFS_H

#ifdef	SCCSID
static char	SccsId_sysdefs_h[] = "@(#)sysdefs.h	43.1	9/9/91";
#endif

#ifndef _TYPES_
#include	<sys/types.h>
#endif

#ifndef _TIME_
#include	<sys/time.h>
#endif

/*
 * Provide definitions of standard library functions.
 */

#ifdef __STDC__
extern int	access (char *path, int mode);
extern long	atol (char *v);
extern int	open(char *f, int op, int mode);
extern int	close (int fd);
extern int	read (int fd, char *buf, int length);
extern off_t	lseek (int fd, off_t o, int mode);
extern double	atof (char *s);
extern int	atoi (char *s);
extern void	exit (int status);
extern char	*index (char *s, int ch);
extern char	*rindex (char *s, int ch);
extern int	getopt(int argc, char **argv, char *optstring);
extern int	gettimeofday(struct timeval *timep, struct timezone *tzp);
extern int	fork();

extern int	strcmp(char *a, char *b);
extern int	strlen(char *s);
extern int	strncmp(char *a, char *b, int n);
extern char 	*strcat(char *a, char *b);
extern char 	*strcpy(char *a, char *b);
extern char	*strdup(char *s);
extern char 	*strncat(char *a, char *b, int n);
extern char 	*strncpy(char *a, char *b, int n);
extern int	bzero (char *a, int len);
extern void	bcopy (char *src, char *dst, int len);
extern int	bcmp (char *b1, char *b2, int len);

extern time_t	time (time_t *t);
extern char	*malloc (unsigned len);
extern int	free (char *p);
extern char	*calloc (unsigned nelem, unsigned elemsize);
extern char	*realloc (char *p, unsigned len);
extern char	*getenv (char *name);
extern void	qsort (char *base, int nel, int width, int (*compar)());
#ifdef FILE
extern int	fprintf (FILE *stream, const char *format, ...);
extern int	fscanf (FILE *stream, const char *format, ...);
extern int	fflush (FILE *stream);
extern int	fclose (FILE *stream);
extern int	fread (char *buf, int size, int count, FILE *stream);
extern int	fwrite (char *buf, int size, int count, FILE *stream);
extern int	fgetc (FILE *stream);
extern int	ungetc (int c, FILE *stream);
extern int	_filbuf();		/* UGH!!! */
extern int	_flsbuf ();		/* UGH!!! */
#ifndef getc
extern int	getc (FILE *stream);
#endif /*getc*/
#else  /*FILE*/
#ifndef __stdio_h	/* For /usr/5include/stdio.h */
extern char *	sprintf (const char *format, ...);
#endif /*!__stdio_h*/
#endif /*FILE*/

extern void	perror (char *message);


extern int	printf (const char *format, ...);
extern int	scanf (const char *format, ...);
extern int	sscanf (const char *string, const char *format, ...);
extern void	execl(char *name, char *arg0, ...);
/**
 ** This is declared as unsigned in isis.h, we had it here as void, which
 ** caused conflicts when including both includefiles and compiling
 ** with a strict compiler.  Also, the arg type is int in isis, we
 ** had it as unsigned in this file.  They're both changed to int
 ** here until we can decide who is right and fix the problem.
 **/
extern unsigned	sleep(unsigned seconds);
#ifdef WNOHANG
extern int	wait(union wait *statusp);
extern int	wait3(union wait *statusp, int options, struct rusage *ruse);
#endif /* WNOHANG */
#ifdef __STAT_HEADER__
extern int 	stat(char *path, struct stat *buf);
extern int	lstat(char *path, struct stat *buf);
extern int	fstat(int fd, struct stat *buf);
#endif /* __STAT_HEADER__ */

#else /* not __STDC__ */
extern double	atof ();
extern int	atoi ();
extern char	*index ();
extern char	*rindex ();

extern int	strcmp();
/* extern int	strlen(); */   /* K.S. 1-Dec-97, removed line */
extern int	strncmp();
extern char 	*strcat();
extern char 	*strcpy();
extern char	*strdup();
extern char 	*strncat();
extern char 	*strncpy();
extern int	bzero ();
extern void	bcopy ();
extern char	*malloc ();
extern int	free ();
extern char	*calloc ();
extern char	*realloc ();
extern char	*getenv ();
extern void	qsort ();

#ifdef FILE
extern int	fflush ();
extern int	fprintf ();
extern int	fscanf ();
extern int	fgetc ();
extern int	ungetc ();
#ifndef getc
extern int	getc ();
#endif /*getc*/
#else  /*FILE*/
#ifndef __stdio_h	/* /usr/5include/stdio.h */
/* extern char *	sprintf (); */   /* K.S. 1-Dec-97, removed line */
#endif /*!__stdio_h*/
#endif /*FILE*/
extern int	printf ();
extern int	scanf ();
extern int	sscanf ();
extern int	fork();
extern int	gettimeofday();
#endif /* __STDC__ */
#endif /* SYSDEFS_H */

