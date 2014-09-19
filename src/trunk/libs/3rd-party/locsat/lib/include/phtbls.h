
c    Common block used by routines in tttab for creating 
c      travel-time tables

c SccsId:  @(#)phtbls.h	36.1	3/6/89

      common/phtbls/phtc(7,15),phdc(5,11),
     1              tsth(45,9),phco(8,11),phhc(15),phlc(15),
     2              faze(34),wndmn(34),wndmx(34),
     3              ibfac(180,8),idxphz(34),indp(34),irespz(34),nid(34),
     4              schmin(34),schmax(34)
      integer*4 schmin,schmax
c      integer*2 ibfac,idxphz,indp,irespz,nid
