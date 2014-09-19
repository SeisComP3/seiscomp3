/*
 *   The "j" parameters (1 st line) are intended to be user settable:
 */

#define jmod 160  /* Maximum number of (rough) model points.		*/
#define jslo 350  /* Maximum number of discrete ray parameters.		*/
#define jdep 450  /* Maximum number of discrete model slowness samples.	*/
		  /* Note:  jdep always need to be larger than jslo by	*/
		  /* approximately 50% to account for multi-valuedness	*/
		  /* due to high slowness zones.			*/
#define jsrc 150  /* Maximum number of discrete model slowness samples	*/
		  /* above the maximum source depth of interest.	*/
#define jbrh  20  /* Maximum number of tau branches (model		*/
		  /* discontinuities plus one).				*/
#define jlvz   5  /* Maximum number of low velocity zones.		*/
#define jseg  30  /* Maximum number of different types of travel-times	*/
		  /* considered.					*/
#define jbrn 100  /* Maximum number of different travel-time branches	*/
		  /* to be searched.					*/
#define jout 2250 /* Maximum length of all travel-time branches strung	*/
		  /* together.						*/
/*
      parameter(jmod=160,jslo=350,jdep=450,jsrc=150,jbrh=20,jlvz=5)
      parameter(jseg=30,jbrn=100,jout=2250)
*/
/*
 * The parameters actually used are all derivatives of the "j"
 * parameters and cannot be changed by the user.
 */

#define	nmd0  (jmod)
#define	nsl1  (jslo+1)
#define	ndp1  (jdep+1)
#define	nsr0  (jsrc)
#define	nbr1  (jbrh+2)
#define	nbr2  (2*jbrh)
#define	ncp0  (2*(jbrh+jlvz))
#define	nlvz0 jlvz

/*
      parameter(nmd0=jmod,nsl1=jslo+1,ndp1=jdep+1,nsr0=jsrc)
      parameter(nbr1=jbrh+2,nbr2=jbrh*2,ncp0=2*(jbrh+jlvz),nlvz0=jlvz)
*/
