
#ifndef SL_GLOBMATCH_H
#define SL_GLOBMATCH_H 1

#ifdef  __cplusplus
extern "C" {
#endif

  
#ifndef SL_GLOBMATCH_NEGATE
#define SL_GLOBMATCH_NEGATE '^'       /* std char set negation char */
#endif


int sl_globmatch(char *string, char *pattern);


#ifdef  __cplusplus
}
#endif

#endif /* globmatch.h  */
