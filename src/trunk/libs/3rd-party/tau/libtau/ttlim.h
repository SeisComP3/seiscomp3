/*
 * The "j" parameters (1 st line) are intended to be user settable:
 * jsrc   Maximum number of discrete model slowness samples above
 * the maximum source depth of interest.
 *
 * 	jseg	Maximum number of different types of travel-times considered.
 * 	jbrn	Maximum number of different travel-time branches to be searched.
 * 	jout	Maximum length of all travel-time branches strung together.
 * 	jtsm	Maximum length of the tau depth increments.
 * 	jxsm	Maximum number of x-values needed for the depth increments.
 * 	jbrnu	Maximum length of the up-going branches.
 * 	jbrna	Maximum length of branches which may need re-interpolation.
 */

#define jsrc	150
#define jseg	30
#define jbrn	100
#define jout	2250
#define jtsm	350
#define jxsm	jbrn
#define jbrnu	jbrn
#define jbrna	jbrn
/*
 * A few derived parameters are also needed.
 */

#define jrec   jtsm+jxsm
#define jtsm0   jtsm+1
