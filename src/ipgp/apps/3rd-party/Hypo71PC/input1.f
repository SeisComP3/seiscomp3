      SUBROUTINE INPUT1(NMAX,LMAX,TEST,KNO,IW,NSTA,INS,IEW,
     &  IELV,DLY,FMGC,XMGC,KLAS,PRR,CALR,ICAL,NDATE,NHRMN,LAT,LON,MDATE,
     &  MHRMN,KLSS,CALS,V,D,DEPTH,VSQ,THK,H,G,F,TID,DID,MJUMP,FLT,KSING,
     &  ZTR,XNEAR,XFAR,POS,IQ,KMS,KFM,IPUN,IMAG,IR,IPRN,KPAPER,KTEST,
     &  KSORT,KAZ,KSEL,LATR,LONR,QSPA,MNO)
	include 'H_param.f'
C------- INPUT STATION LIST,CRUSTAL MODEL,AND CONTROL CARD -------------
      CHARACTER*1 IW(N_s),INS(N_s),IEW(N_s)
      CHARACTER*4 ISW,NSTA(N_s),IPRO
      CHARACTER*48 AHEAD,BHEAD
      INTEGER*4 IELV(N_s),KLAS(N_s),ICAL(N_s),NDATE(N_s),NHRMN(N_s)
      INTEGER*4 MNO(N_s),KLSS(N_s),MDATE(N_s),MHRMN(N_s)
      REAL*4 LAT2,LON2,LATR,LONR,TEST(20),ATEST(20),QSPA(9,40)
      REAL*4 FMGC(N_s),XMGC(N_s),PRR(N_s),CALR(N_s),LAT(N_s),LON(N_s)
      REAL*4 CALS(N_s)
      REAL*4 V(21),D(21),VSQ(21),THK(21),H(21),DEPTH(21)
      REAL*4 TID(21,21),DID(21,21),F(21,21),G(4,21)
      REAL*4 DLY(2,N_s),FLT(2,N_s)
      COMMON/C3/ AHEAD,IPRO,ISW
      COMMON/C4/ NL,NS,KDATE,KHR,NEAR,IEXIT,IDXS
	common/dts/stdt(N_s)
      common/stdio/lui,luo,lua1,lua2
c***********************************************************************
c#include "common.rai"
c************** common associe au milieu heterogene dans common.rai
      common/provi/epsob,ifhet,elev(N_s)
c************** il y a un iflag dans ce programme d'ou nom ifhet ici
      dimension ibuf(1)
      character*1 dum
      data ibuf(1)/1/,lbuf/1/,nout/11/
c      call plots(ibuf,lbuf,nout)
C-----------------------------------------------------------------------
      DO 350 I=1,20
      ATEST(I) = 1.23456
  350 CONTINUE
      WRITE(8,300)
  300 FORMAT(1H1)
      IF (MJUMP-1) 1,100,200
C------- INITIALIZE TEST VARIABLES -------------------------------------
    1 TEST(1)=0.10
      TEST(2)=10.
      TEST(3)=2.
      TEST(4)=0.05
      TEST(5)=5.                                                        
      TEST(6)= 4.                                                       
      TEST(7)=-0.87                                                     
      TEST(8)=+2.00                                                     
      TEST(9)=+0.0035                                                   
      TEST(10)=100.                                                     
      TEST(11)=8.0                                                      
      TEST(12)=0.5                                                      
      TEST(13)= 1.                                                      
	test(15)=0.
	test(20)=1.
      IFLAG=0                                                           
      AHEAD = '                                                '        
C------- INPUT RESET TEST-VARIABLE CARDS AND SELECTION CARD ------------
      DO 5 I=1,25                                                       
	print *,'test',i
      read(12,4) ISW,J,TESTJ,BHEAD                                      
    4 FORMAT(A4,7X,I2,2X,F9.4,A48)                                   
	if(isw.eq.'#...') goto 5
      IF ((ISW.EQ.'    ').OR.(ISW.EQ.'1   ')) GO TO 6                   
      IF(ISW .NE. 'HEAD') GO TO 12                                      
      AHEAD = BHEAD                                                     
c************** adjonction des parametres du milieu heterogene (invisibl
      iheter=j                                                          
      epsob=testj                                                       
      GO TO 5                                                           
   12 IFLAG=1                                                           
      ATEST(J)=TESTJ                                                    
    5 CONTINUE                                                          
    6 WRITE(8,14) AHEAD                                                 
   14 FORMAT(40X,A48)                                                   
c      WRITE(8,2)                                                        
    2 FORMAT(///,' ***** PROGRAM: HYPO71PC (Version 2: ********) *****',
     1      ///,13X,'TEST(1)  TEST(2)  TEST(3)  TEST(4)  TEST(5)  TEST(6
     2)  TEST(7)  TEST(8)  TEST(9) TEST(10) TEST(11) TEST(12) TEST(13)')
      WRITE(8,3) (I,TEST(I),I=1,20)                                       
    3 FORMAT(' STANDARD ',/,5(I4,F9.4))                                       
      IF (IFLAG .EQ. 0) GO TO 8                                         
      DO 16 I = 1,20                                                    
      IF(ATEST(I) .NE. 1.23456) TEST(I)=ATEST(I)                        
   16 CONTINUE                                                          
      WRITE(8,7) (I,TEST(I),I=1,20)                                       
    7 FORMAT(' RESET TO ',/,5(I4,F9.4))                                       
C------- SQUARE SOME TEST-VARIABLES FOR LATER USE ----------------------
    8 TEST(1)=TEST(1)**2                                                
      TEST(2)=TEST(2)**2                                                
      TEST(4)=TEST(4)**2                                                
C------- INPUT STATION LIST --------------------------------------------
      IF (ISW .EQ. '1   ') GO TO 10                                     
      KNO=1                                                             
      WRITE(8,9)                                                        
    9 FORMAT(/,4X,'L     STN     LAT     LONG  ','  ELV DELAY',5X       
     1,'FMGC  XMGC KL  PRR  CALR IC      DATE HRMN  dt')
      GO TO 20                                                          
   10 WRITE(8,15)                                                       
   15 FORMAT(/,4X,'L   STN    LAT      LONG      ELV     M  DLY1  DLY2',
     1'  XMGC FMGC KL CALR IC   DATE HRMN  dt')        
   20 DO 50 L=1,NMAX                                                    
      IF (ISW .EQ. '1   ') GO TO 30                                     
       read(12,25)IW(L),NSTA(L),LAT1,LAT2,INS(L),LON1,LON2,
     1  IEW(L),IELV(L)
     2 ,DLY(1,L),FMGC(L),XMGC(L),KLAS(L),PRR(L),CALR(L),ICAL(L),NDATE(L) 
     3 ,NHRMN(L),stdt(l)
   25 FORMAT(1X,A1,A4,I2,F5.2,A1,I3,F5.2,A1,I4,F6.2,4X,F5.2,2X,F5.2,1X  
     1,I1,F5.2,F7.2,1X,I1,5X,I6,I4,f6.2)
      IF (NSTA(L) .EQ. '    ') GO TO 60                                 
      IF (INS(L) .EQ. ' ') INS(L)='N'                                   
      IF (IEW(L) .EQ. ' ') IEW(L)='W'                                   
        WRITE(8,26) L,IW(L),NSTA(L),LAT1,LAT2,INS(L),LON1,LON2,IEW(L)     
     1,IELV(L),DLY(1,L),FMGC(L),XMGC(L),KLAS(L),PRR(L),CALR(L),ICAL(L)  
     2,NDATE(L),NHRMN(L),stdt(l)
   26 FORMAT(I5,3X,A1,A4,I3,F5.2,A1,I4,F5.2,A1,I5,F6.2,4X,F5.2,2X,F5.2  
     1,1X,I1,F5.2,F7.2,1X,I1,5X,I6,I4,f6.2)
      GO TO 40                                                          
   30 read(12,35)NSTA(L),IW(L),LAT1,LAT2,INS(L),LON1,LON2,IEW(L),
     1           IELV(L),MNO(L),DLY(1,L),DLY(2,L),XMGC(L),FMGC(L),
     2           KLAS(L),CALR(L),ICAL(L),NDATE(L),NHRMN(L),stdt(l)                                        
      IF (NSTA(L) .EQ. ' ') GO TO 60                                 
      IF (INS(L) .EQ. ' ') INS(L)='N'                                   
      IF (IEW(L) .EQ. ' ') IEW(L)='W'                                   
        WRITE(8,36) L,NSTA(L),IW(L),LAT1,LAT2,INS(L),LON1,LON2,IEW(L)     
     1,IELV(L),MNO(L),DLY(1,L),DLY(2,L),XMGC(L),FMGC(L),KLAS(L),CALR(L) 
     2,ICAL(L),NDATE(L),NHRMN(L),stdt(l)                                
   36 FORMAT(I5,2X,A4,A1,I2,1X,F5.2,A1,I4,1X,F5.2,A1,I5,5X,I1           
     1,4F6.2,1X,I1,F6.2,1X,I1,2X,I6,I4,f6.2)                            
      PRR(L)=0.                                                         
   40 LAT(L)=60.*LAT1+LAT2                                              
c	if(ins(l).eq.'S') lat(l)=-lat(l)
      LON(L)=60.*LON1+LON2                                              
c	if(iew(l).eq.'W') lon(l)=-lon(l)
      MDATE(L)=NDATE(L)                                                 
      MHRMN(L)=NHRMN(L)                                                 
      KLSS(L)=KLAS(L)                                                   
      CALS(L)=CALR(L)                                                   
      elev(l)=-ielv(l)*0.001*test(20)                                            
   50 CONTINUE                                                          
35    FORMAT(A4,A1,I2,1X,F5.2,A1,I3,
     1 1X,F5.2,A1,I4,5X,I1,4F6.2,1X,I1,F6.2,1X,I1,2X,I6,I4,f6.2)
c     WRITE(8,55)                                                       
c  55 FORMAT(///,' ***** ERROR: STATION LIST EXCEEDS ARRAY DIMENSION')  
c     STOP                                                              
   60 NS=L-1                                                            
C------- INPUT CRUSTAL MODEL -------------------------------------------
  100 WRITE(8,105)                                                      
  105 FORMAT(///,7X,'CRUSTAL MODEL 1',/,5X,'VELOCITY     DEPTH')        
      DO 130 L=1,LMAX                                                   
      read(12,115) V(L),D(L)                                            
	if(v(1).lt.0.) then
		if(v(1).lt.-1.1) then
c-------- lecture de H,vp,gp,vrp,vpvs
			read(12,*)(v(i),i=1,4),rap
				v(5)=v(2)/rap
				v(6)=v(3)/rap
				v(7)=v(4)/rap
		else
c-------- lecture de H,vp,gp,vrp,vs,gs,vrs
			read(12,*)(v(i),i=1,7)
		endif
	nl=0
	print *,'modele a gradient + demi espace a ',v(1)
	print *,'VP,GP,VRP ',v(2),v(3),v(4)
	print *,'VS,GS,VRS ',v(5),v(6),v(7)
	write(8,*)'modele a gradient + demi espace a ',v(1)
	write(8,*)'VP,GP,VRP ',v(2),v(3),v(4)
	write(8,*)'VS,GS,VRS ',v(5),v(6),v(7)
	depth(1)=d(1)
	thk(1)=d(1)
	h(1)=d(1)
	goto 200
	endif
  115 FORMAT(2F7.3)                                                     
      IF (V(L) .LT. 0.01) GO TO 140                                     
      WRITE(8,125) V(L),D(L)                                            
  125 FORMAT(3X,2F10.3)                                                 
      DEPTH(L)=D(L)                                                     
      VSQ(L)=V(L)**2                                                    
  130 CONTINUE                                                          
      WRITE(8,135)                                                      
  135 FORMAT(///,' ***** ERROR: CRUSTAL MODEL EXCEEDS ARRAY DIMENSION') 
      STOP                                                              
  140 NL=L-1                                                            
      N1=NL-1                                                           
C-----LAYER THICKNESS THK,F & G TERMS                                   
      DO 145 L=1,N1                                                     
      THK(L)=D(L+1)-D(L)                                                
  145 H(L)=THK(L)                                                       
C---- COMPUTE TID AND DID                                               
      DO 150 J=1,NL                                                     
      G(1,J)=SQRT(ABS(VSQ(J)-VSQ(1)))/(V(1)*V(J))                       
      G(2,J)=SQRT(ABS(VSQ(J)-VSQ(2)))/(V(2)*V(J))                       
      G(3,J)=V(1)/SQRT(ABS(VSQ(J)-VSQ(1))+0.000001)                     
      G(4,J)=V(2)/SQRT(ABS(VSQ(J)-VSQ(2))+0.000001)                     
      IF (J .LE. 1) G(1,J)=0.                                           
      IF (J .LE. 2) G(2,J)=0.                                           
      IF (J .LE. 1) G(3,J)=0.                                           
      IF (J .LE. 2) G(4,J)=0.                                           
      DO 150 L=1,NL                                                     
      F(L,J)=1.                                                         
      IF (L .GE. J) F(L,J)=2.                                           
  150 CONTINUE                                                          
      DO 165 J=1,NL                                                     
      DO 165 M=1,NL                                                     
      TID(J,M)=0.                                                       
  165 DID(J,M)=0.                                                       
      DO 170 J=1,NL                                                     
      DO 170 M=J,NL                                                     
      IF (M .EQ. 1) GO TO 170                                           
      M1=M-1                                                            
      DO 160 L=1,M1                                                     
      SQT=SQRT(VSQ(M)-VSQ(L))                                           
      TIM=THK(L)*SQT/(V(L)*V(M))                                        
      DIM=THK(L)*V(L)/SQT                                               
      TID(J,M)=TID(J,M)+F(L,J)*TIM                                      
  160 DID(J,M)=DID(J,M)+F(L,J)*DIM                                      
  170 CONTINUE                                                          
      IF (ISW .NE. '1   ') GO TO 200                                    
C From YTRV.F ----------------------------------------------------------
      print *,'the variable first layer not implemented'
      stop
C 180 FLT(2,I)=DLY(2,I)*VC+D(2)
C------- INPUT CONTROL CARD --------------------------------------------
  200 WRITE(8,205)                                                      
  205 FORMAT(///,1x,'KS Z XNEAR XFAR  POS   IQ  KMS  KFM IPUN IMAG   IR'   
     1,' IPRN CODE   LATR      LONR')                                   
      read(12,215) KSING,ZTR,XNEAR,XFAR,POS,IQ,KMS,KFM,IPUN,IMAG,IR,IPRN
     1,KPAPER,KTEST,KAZ,KSORT,KSEL,LAT1,LAT2,LON1,LON2                  
  215 FORMAT(I1,F4.0,2F5.0,F5.2,7I5,5I1,2(I4,F6.2))                     
       WRITE(8,218) KSING,ZTR,XNEAR,XFAR,POS,IQ,KMS,KFM,IPUN,IMAG,IR,IPRN
     1,KPAPER,KTEST,KAZ,KSORT,KSEL,LAT1,LAT2,LON1,LON2                  
  218 FORMAT(1X,I1,F4.0,2F5.0,F5.2,7I5,5I1,2(I4,F6.2))                  
      LATR=60.*LAT1+LAT2                                                
      LONR=60.*LON1+LON2                                                
      IF (IR .EQ. 0) RETURN                                             
c      DO 240 I=1,IR                                                     
c      read(12,225) (QSPA(I,J),J=1,40)                                   
c  225 FORMAT(20F4.2)                                                    
c      WRITE(8,235) I,(QSPA(I,J),J=1,40)                                 
c  235 FORMAT(/,' QSPA(',I1,'): ',20F5.2,/,10X,20F5.2)                   
c  240 CONTINUE                                                          
      RETURN                                                            
      END                                                               
