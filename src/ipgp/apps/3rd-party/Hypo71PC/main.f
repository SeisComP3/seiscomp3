C*** HYPO71PC FORTRAN: FORTRAN 77 VERSION OF HYPO71 (REVISED) BY LEE AND
C***                   LAHR (USGS OPEN-FILE REPORT 75-311, 1975).
C*** MODIFICATIONS BY W. H. K. LEE AND C. M. VALDES, NOV. 29, 1985.
C-----------------------------------------------------------------------
	include 'H_param.f'
      CHARACTER*1 SYM(N_d),QRMK(N_d),IW(N_s),INS(N_s),IEW(N_s)
      CHARACTER*1 insc,iewc
      CHARACTER*3 RMK(N_d)
      character*6 magout
      character*1 rmk2
	real*4 lon2,lat2
      CHARACTER*4 IPRO,ISW
      CHARACTER*4 NSTA(N_s)
      CHARACTER*4 MSTA(N_d),PRMK(N_d),SRMK(N_d),AZRES(N_d),WRK(N_d)
      CHARACTER*14 FINPUT,FPRINT,FPUNCH,BLANKS,fresum,fbull1,ftri
      CHARACTER*48 AHEAD
      CHARACTER*80 SUCARD
      INTEGER*4 KDX(N_d),LDX(N_d),JMIN(N_d)
      INTEGER*4 NXM(N_s),NFM(N_s),ICAL(N_s),JDX(N_s),IELV(N_s),MNO(N_s)
      INTEGER*4 KLAS(N_s),KLSS(N_s),KSMP(N_s),NRES(2,N_s)
      INTEGER*4 MDATE(N_s),MHRMN(N_s),NDATE(N_s),NHRMN(N_s)
      REAL*4 V(21),D(21),THK(21),H(21),DEPTH(21),VSQ(21)
      REAL*4 TID(21,21),DID(21,21),F(21,21),G(4,21)
      REAL*4 XMAG(N_d),FMAG(N_d),AMX(N_d),PRX(N_d),CALX(N_d),FMP(N_d)
      REAL*4 W(N_d),P(N_d),TP(N_d),S(N_d),WS(N_d),TS(N_d),DT(N_d)
      REAL*4 LATR,LONR,QNO(4),TEST(20),QSPA(9,40)
      REAL*4 SXM(N_s),SXMSQ(N_s),SFM(N_s),SFMSQ(N_s),CALS(N_s)
      REAL*4 LAT(N_s),LON(N_s),PRR(N_s),CALR(N_s),FMGC(N_s),XMGC(N_s)
      REAL*4 SR(2,N_s),SRSQ(2,N_s),SRWT(2,N_s),DLY(2,N_s),FLT(2,N_s)
      REAL*8 TIME1,TIME2
	real *4 mag
	real*4 delta(N_d)
      COMMON/C1/IQ,KMS,KFM,IPUN,IMAG,IR,IPRN,KPAPER,KTEST,KAZ,KSORT,KSEL
      COMMON/C2/ ZTR,XNEAR,XFAR,POS,LATR,LONR,ONF,FLIM
      COMMON/C3/ AHEAD,IPRO,ISW
      COMMON/C4/ NL,NS,KDATE,KHR,NEAR,IEXIT,IDXS
      COMMON/C5/ PMIN,XFN
      COMMON/O1/ NI,INST,KNST,IPH,JPH,NDEC,JMAX,JAV,NR,NRP,KF,KP,KZ,KKF
      COMMON/O2/ AVRPS,DMIN,RMSSQ,ADJSQ,LATEP,LONEP,Z,ZSQ,AVR,AAR,ORG
      COMMON/O3/ SUCARD,insc,iewc
      COMMON/O4/ delta
c************************************ common pour iflag
      common/provi/epsob,iflag,elev(N_s)
      common/stdio/ lui,luo,lua1,lua2
      DATA LMAX,MMAX,NMAX/21,N_d,N_s/
      DATA BLANKS/'              '/
      data lui,luo,lua1,lua2/5,6,20,14/
C-----------------------------------------------------------------------
c      write(luo,5)
    5 FORMAT(///,' **** RUNNING HYPO71PC (Version 2: ********) **** ',/)
C
C** ASK USER FOR INPUT AND OUTPUT FILES
C
      FINPUT = BLANKS
      FPRINT = BLANKS
      FPUNCH = BLANKS
      fresum = blanks
C      write(luo,11)
   11 FORMAT(/,' Enter FILENAME for HYPO71PC input data ',
     &      '(CR = HYPO71PC.INP)?',/)
      READ(lui,12,END=13) FINPUT
   12 FORMAT(A14)
   13 continue
C 	write(luo,14)
   14 FORMAT(/,' Enter FILENAME for HYPO71PC printed output ',
     &      '(CR = HYPO71PC.PRT)?',/)
      read(lui,12,END=15) FPRINT
   15 continue
C 	write(luo,16)
   16 FORMAT(/,' Enter FILENAME for HYPO71PC punched output ',
     &      '(CR = HYPO71PC.PUN)?',/)
      read(lui,12,END=17) FPUNCH
   17 continue
C 	write(luo,18)
   18 FORMAT(/,' Enter FILENAME for HYPO71PC resumed output ',
     &      '(CR = HYPO71PC.RES)?',/)
      read(lui,12,end=19) fresum
   19 continue
C 	write(luo,20)
   20 FORMAT(/,' Enter FILENAME for HYPO71PC milieu input ',
     &      '(CR = no input    )?',/)
      read(lui,12,end=27) fbull1
   27 continue
C 	write(luo,28)
   28 FORMAT(/,' Enter FILENAME for HYPO71PC reloc output ',
     &      '(CR = no output   )?',/)
      read(lui,12,end=21) ftri
   21 IF (FINPUT .EQ. BLANKS) FINPUT = 'HYPO71.INP'
      IF (FPRINT .EQ. BLANKS) FPRINT = 'HYPO71.PRT'
      IF (FPUNCH .EQ. BLANKS) FPUNCH = 'HYPO71.PUN'
      IF (fresum .EQ. BLANKS) fresum = 'HYPO71.RES'
      IF (fbull1 .EQ. BLANKS) lua2=0
      IF (ftri   .EQ. BLANKS) lua1=0
C
C** OPEN THE INPUT AND OUTPUT FILES
C
c---------------------------------------------
      OPEN(8,FILE=FPRINT,STATUS='unknown')
      write(8,22) FINPUT,FPRINT,FPUNCH,fresum,fbull1,ftri
   22 FORMAT(//,' Input date file   --> ',A14,/,
     &          ' Output print file --> ',A14,/,
     &          ' Output punch file --> ',A14,/,
     &          ' Output resum file --> ',A14,/,
     &          ' Input milieu file --> ',A14,/,
     &          ' Ouput reloc  file --> ',A14,/)
      OPEN(7,FILE=FPUNCH,STATUS='unknown')
      open(9,file=fresum,status='unknown')
      OPEN(12,FILE=FINPUT,STATUS='OLD')
      if(lua2.ne.0) open(lua2,file=fbull1,status='unknown')
      if(lua1.ne.0) open(lua1,file=ftri,status='unknown')
C
   30 MJUMP=0
C------- INPUT STATION LIST, CRUSTAL MODEL, & CONTROL CARD -------------
c      write(luo,36)                                                       
   36 FORMAT(/,' ... Setting up station list, crustal model, & control',
     &         ' card',/)                                               
   40 CALL INPUT1(NMAX,LMAX,TEST,KNO,IW,NSTA,INS,IEW,                   
     &  IELV,DLY,FMGC,XMGC,KLAS,PRR,CALR,ICAL,NDATE,NHRMN,LAT,LON,MDATE,
     &  MHRMN,KLSS,CALS,V,D,DEPTH,VSQ,THK,H,G,F,TID,DID,MJUMP,FLT,KSING,
     &  ZTR,XNEAR,XFAR,POS,IQ,KMS,KFM,IPUN,IMAG,IR,IPRN,KPAPER,KTEST,   
     &  KSORT,KAZ,KSEL,LATR,LONR,QSPA,MNO)                              
      IF(IPUN .EQ. 0) GO TO 44                                          
      WRITE(7,41) INS(1),IEW(1)                                         
   41 FORMAT(' DATE    ORIGIN    LAT ',A1,'    LONG ',A1,'    DEPTH    M
     1AG NO GAP DMIN  RMS  ERH  ERZ QM')                                
C-------- INITIALIZE SUMMARY OF RESIDUALS ------------------------------
   44 DO 48 L=1,NS                                                      
      NRES(1,L)=0                                                       
      NRES(2,L)=0                                                       
      NXM(L)=0                                                          
      NFM(L)=0                                                          
      SR(1,L)=0.                                                        
      SR(2,L)=0.                                                        
      SRSQ(1,L)=0.                                                      
      SRSQ(2,L)=0.                                                      
      SRWT(1,L)=0.                                                      
      SRWT(2,L)=0.                                                      
      SXM(L)=0.                                                         
      SXMSQ(L)=0.                                                       
      SFM(L)=0.                                                         
      SFMSQ(L)=0.                                                       
   48 CONTINUE                                                          
      DO 49 I=1,4                                                       
   49 QNO(I)=0.                                                         
      XFN=XFAR-XNEAR+0.000001                                           
      TIME1=0.D+00                                                      
c***************************                                            
50	write(luo,'(79(1h*))')
      iflag=0                                                           
      CALL INPUT2(NSTA,KLAS,MMAX,IW,MSTA,PRMK,W,JMIN,P,S,               
     &  SRMK,WS,AMX,PRX,CALX,RMK,DT,FMP,AZRES,SYM,QRMK,KDX,LDX,JDX,     
     &  TP,WRK,KSMP,PMIN,TS,CALR,TIME1,TIME2,MDATE,                     
     &  MHRMN,NR,MJUMP)                                                 
C------- TO PROCESS ONE EARTHQUAKE -------------------------------------
      IF (MJUMP .EQ. 1) GO TO 900                                       
	if(mjump.eq.3) goto 50
      IF (NR .GE. 1) GO TO 100                                          
      WRITE(8,55)                                                       
   55 FORMAT( ///,' ***** EXTRA BLANK CARD ENCOUNTERED *****')          
      GO TO 50                                                          
  100 CONTINUE                                                          
      KKF = 0                                                           
      KYEAR = KDATE/10000                                               
      KMONTH = (KDATE - 10000*KYEAR)/100                                
      KDAY = KDATE - 10000*KYEAR - 100*KMONTH                           
c     write(luo,101) KYEAR,KMONTH,KDAY,KHR,JMIN(1)                        
  101 FORMAT(/,' ... Locating earthquake on ',I2,'/',I2,'/',I2,         
     &        2X,I2,':',I2,/)                                           

      CALL SINGLE(TEST,KNO,IW,NSTA,INS,IEW,DLY,FMGC,                    
     &  XMGC,KLAS,PRR,CALR,ICAL,LAT,LON,V,D,DEPTH,VSQ,THK,H,G,F,        
     &  TID,DID,FLT,QSPA,MSTA,PRMK,W,JMIN,P,S,SRMK,WS,AMX,PRX,CALX,RMK, 
     &  DT,FMP,AZRES,SYM,QRMK,KDX,LDX,JDX,TP,WRK,KSMP,TS,TIME1,TIME2,   
     &  AVXM,AVFM,XMAG,FMAG,NRES,SR,SRSQ,SRWT,QNO,MNO)                  

      iflag=0                                                           
  103 IF (KKF .EQ. 0) GOTO 105                                          
c-------- bulletenisation des seismes pas localises
      write(luo,104)                                                      
	do k=1,nrp
	print *,delta(k),amx(k),fmp(k)
	enddo
  104 FORMAT(' *** INSUFFICIENT DATA FOR LOCATING THIS QUAKE ***')      
	call xfmags
     & (TEST,FMGC,XMGC,KLAS,PRR,CALR,ICAL,IMAG,IR,QSPA,                 
     &  AMX,PRX,CALX,FMP,KDX,DELTA,ZSQ,NRP,CAL,NM,AVXM,SDXM,XMAG,NF,    
     &  AVFM,SDFM,FMAG,MAG)                                             
c	print *,'near ',near
c	do kj=1,nrp
c		jjj=kdx(kj)
c		print *,nsta(jjj)
c		print *,lon(jjj)/60,lat(jjj)/60.
c		print *,ktime,jmin(kj),p(kj)
c	enddo
	z=ztr
	kmin=jmin(1)
	igap=360
	sec=p(1)
	jjj=kdx(1)
	lat2=lat(jjj)
	lat1=int(lat(jjj)/60)
	lat2=lat2-60*lat1
	lon2=lon(jjj)
	lon1=lon(jjj)/60
	lon2=lon2-60*lon1
      MAGOUT = '      '                                                 
      IF (MAG .NE. 99.9) WRITE(MAGOUT,68) MAG                           
   68 FORMAT(F6.2)                                                      
	print *,'magnitude ',mag
      WRITE(8,86) KDATE,RMKO,KHR,KMIN,SEC,LAT1,LAT2,LON1,LON2           
     1,Z,RMK2,MAGOUT,NO,IDMIN,IGAP,KNO,RMS,ERHOUT,SE3OUT,Q,QS           
     2,QD,ADJ,JNST,NR,AVR,AAR,NM,AVXM,SDXM,NF,AVFM,SDFM,NI              
	if(lua2.ne.0) write(lua2,*) ' '

	rmk2='*'
      if(lua2.ne.0) WRITE(lua2,87) KDATE,KHR,KMIN,SEC,LAT1,LAT2,
     1 abs(LON1),abs(LON2)                
     1,Z,RMK2,MAGOUT,Nrp,IGAP,0.,0.,'     ','     ',QRMK(1)             
     2,'D','4'                                                           
   87 FORMAT(I6,1X,2I2,F6.2,I3,'-',F5.2,I4,'-',F5.2,1X,F6.2,A1,A6,I3    
     1,I4,F5.1,F5.2,2A5,3A1)                                            
   86 FORMAT(1X,I6,A1,2I2,F6.2,I3,'-',F5.2,I4,'-',F5.2,1X,F6.2,A1,A6    
     1,2I3,I4,I2,F5.2,2A5,2(1X,A1),'|',A1,F5.2,2I3,2F5.2,2(I3,2F5.1),I2)
c-------- fin bulletenisation des seismes pas localises
      GOTO 109                                                          
  105 continue
  109 IF (IEXIT .EQ. 1) GO TO 50                                        
C------- COMPUTE SUMMARY OF MAGNITUDE RESIDUALS ----------------------- 
      IF (JAV .GT. IQ) GO TO 50                                         
      DO 150 I=1,NRP                                                    
      IF  (XMAG(I) .EQ. 99.9) GO TO 120                                 
      JI=KDX(I)                                                         
      DXMAG=XMAG(I)-AVXM                                                
      NXM(JI)=NXM(JI)+1                                                 
      SXM(JI)=SXM(JI)+DXMAG                                             
      SXMSQ(JI)=SXMSQ(JI)+DXMAG**2                                      
  120 IF  (FMAG(I) .EQ. 99.9) GO TO 150                                 
      JI=KDX(I)                                                         
      DFMAG=FMAG(I)-AVFM                                                
      NFM(JI)=NFM(JI)+1                                                 
      SFM(JI)=SFM(JI)+DFMAG                                             
      SFMSQ(JI)=SFMSQ(JI)+DFMAG**2                                      
  150 CONTINUE                                                          
      GO TO 50                                                          
  900 CONTINUE                                                          
C------- END OF ONE DATA SET: PRINT SUMMARY OF RESIDUALS & RETURN ------
      CALL SUMOUT(ISW,NSTA,INS,IEW,IELV,DLY,FMGC,XMGC,KLAS,PRR,         
     &  CALR,ICAL,NDATE,NHRMN,LAT,LON,MDATE,MHRMN,KLSS,CALS,NS,QNO,IPUN,
     &  MNO,NRES,NXM,NFM,SR,SRSQ,SRWT,SXM,SXMSQ,SFM,SFMSQ)              
      IF (MSTA(NR+1) .EQ. ' ***') GO TO 30                              
      MJUMP=1                                                           
      IF (MSTA(NR+1) .EQ. ' $$$') GO TO 40                              
      MJUMP=2                                                           
      IF (MSTA(NR+1) .EQ. ' ###') GO TO 40                              
c     call endof                                                        
      STOP                                                              
      END                                                               
