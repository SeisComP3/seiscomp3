c     $copy module g.iaapaq contains the declarations and equivalences
c       which define the layout of an aapaq record.

c  SccsId:  @(#)iaapaq.h	36.1  3/6/89

              parameter (maxzaa=60)

      common/paqcha/chanel,  aaphas
      character*14 chanel, aaphas

      common /paqzre/ iaapaq
c      integer*2  iaapaq
c      integer*2   iassoc, ixphas,
c     *  kode2 , kode3 ,  d8, lrnum, iregon
      dimension iaapaq (maxzaa)
c
      equivalence  (iaapaq(  1), ipaqyr)
      equivalence  (iaapaq(  3), ipaqtm)
      equivalence  (iaapaq(  5), clssss)
      equivalence  (iaapaq(  7), elssss)
      equivalence  (iaapaq(  9), kessss)
      equivalence  (iaapaq( 13), invelx)
      equivalence  (iaapaq( 15), invely)
      equivalence  (iaapaq( 17), isgvel)
      equivalence  (iaapaq( 19), bmclat)
      equivalence  (iaapaq( 21), bmlong)
      equivalence  (iaapaq( 23), bmazim)
      equivalence  (iaapaq( 25), signoi)
      equivalence  (iaapaq( 27), amplit)
      equivalence  (iaapaq( 29), period)
      equivalence  (iaapaq( 31), smarta)
      equivalence  (iaapaq( 33), nevnum)
      equivalence  (iaapaq( 35), resid )
      equivalence  (iaapaq( 37), iassoc)
      equivalence  (iaapaq( 38), ixphas)
      equivalence  (iaapaq( 39), aaspra)
      equivalence  (iaapaq( 41), aaeman)
      equivalence  (iaapaq( 43), kode2 )
      equivalence  (iaapaq( 44), kode3 )
      equivalence  (iaapaq( 45), aafsta)
      equivalence  (iaapaq( 47), d8)
      equivalence  (iaapaq( 48), lrnum )
      equivalence  (iaapaq( 49), iregon)
      equivalence  (iaapaq(50), stnois)
      equivalence  (iaapaq(52), stbias )
