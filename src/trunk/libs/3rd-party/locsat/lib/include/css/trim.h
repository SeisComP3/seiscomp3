
/*
 * SccsId:	@(#)trim.h	43.1	9/9/91
 */

/* general purpose macros to trim strings for C and pad them for fortran*/

/**
 ** WARNING!!  Do NOT call the local variable in the macros something like
 ** "int i"; if someone happens to call a macro with something like
 ** OTRIM(strings[i], 5)
 ** it will break in a nasty way ... I know, I've seen it.
 **/

#ifndef TRIM
#define TRIM(s,l) {\
			   int _NOT_i_;\
			   for (_NOT_i_=l-1; _NOT_i_>0, s[_NOT_i_-1]==' '; _NOT_i_--)\
				   ;\
			   s[_NOT_i_]='\0';\
		   }
#endif
#ifndef FPAD
#define FPAD(s,l) {\
			   int _NOT_i_;\
			   for (_NOT_i_=strlen(s); _NOT_i_ < l; s[_NOT_i_++]=' ')\
				   ;\
		   }
#endif
#ifndef BFIL
#define BFIL(s,l) {\
			   int _NOT_i_;\
			   for(_NOT_i_=0; _NOT_i_<l; s[_NOT_i_++]=' ')\
				   ;\
		   }
#endif

/*
 * macro for preparing a character array used by Oracle for transfer to 
 * a C string. This is because Oracle characters must be the size of the 
 * target field and cannot have nulls.
 */
#ifndef OTRIM
#define OTRIM(s,l) {\
			    int _NOT_i_;\
			    for(_NOT_i_=l-1; _NOT_i_ >0, s[_NOT_i_-1]==' '; _NOT_i_--)\
				    s[_NOT_i_]='\0';\
			    if (s[_NOT_i_] == ' ')\
				    s[_NOT_i_]='\0';\
		    }
#endif

