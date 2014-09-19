
c     Parameter declarations

      integer*4 iunit, maxcorrsta, maxnode, maxphs, maxsta, maxtab
      integer*4 maxtyp

c     Station correction input file unit number
      parameter (iunit = 21)

c     Maximum permissable different station correction types
      parameter (maxtyp = 3)

c     Maximum number of total stations permitted
      parameter (maxsta = 9999)

c     Maximum number of stations WITH corrections permitted
      parameter (maxcorrsta = 15)

c     Maximum permissable number of nodes in lat/lon directions
      parameter (maxnode = 15)

c     Maximum average number of tables per station/correction-type pair
      parameter (maxtab = 15)

c     Maximum permissable number of phases
      parameter (maxphs = 25)


      integer*2 cumulsrcs, indexstacor, nlats, nlons, nodecumul, numsrcs
      integer*2 stacor
      real*4    splat, splon, xlat1, xlat2, xlon1, xlon2

      common /corrs/ splat(maxtyp,maxcorrsta,2,maxtab*(maxnode-1)),
     &               splon(maxtyp,maxcorrsta,2,maxtab*(maxnode-1)),
     &               xlat1(maxtyp,maxcorrsta,2,maxtab),
     &               xlat2(maxtyp,maxcorrsta,2,maxtab),
     &               xlon1(maxtyp,maxcorrsta,2,maxtab),
     &               xlon2(maxtyp,maxcorrsta,2,maxtab),
     &               stacor(maxtyp,maxcorrsta,2,maxnode*maxnode*maxtab),
     &               numsrcs(maxtyp,maxcorrsta,2,maxphs),
     &               cumulsrcs(maxtyp,maxcorrsta,2,maxphs),
     &               nodecumul(maxtyp,maxcorrsta,2,maxtab),
     &               nlats(maxtyp,maxcorrsta,2,maxtab),
     &               nlons(maxtyp,maxcorrsta,2,maxtab),
     &               indexstacor(maxtyp,maxsta,2)

