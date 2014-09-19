      SUBROUTINE FMPLOT(KPAPER,KFM,FNO,NRP,AZ,AIN,SYM,SUCARD)           
C------- PLOT FIRST-MOTION DIRECTIONS OF THE LOWER FOCAL HEMISPHERE     
C        IN EQUAL AREA PROJECTION, WHERE C DENOTES COMPRESSION AND      
C        D DENOTES DILATATION ------------------------------------------
C*** 10/25/85: MODIFIED FOR TWO DIFFERENT PRINTER PAPER SIZES           
	include 'H_param.f'
      CHARACTER*1 GRAPH(107,59),SYM(N_dat),TEMP                           
      CHARACTER*2 K0                                                    
      CHARACTER*4 K180                                                  
      CHARACTER*80 SUCARD                                               
      REAL*4 SE(4),AZ(N_dat),AIN(N_dat)                                     
      DATA NOY,IX,IY,NOY1,NOX2,NOY2/59,39,24,57,48,30/                  
      DATA RMAX,XSCALE,YSCALE,ADD/3.937008,0.101064,0.169643,4.75/      
C-----------------------------------------------------------------------
      NOX=95                                                            
      XPAPER=1.                                                         
      K0='0 '                                                           
      K180='180 '                                                       
      IF (KPAPER .EQ. 0) GOTO 100                                       
      NOX=107                                                           
      XPAPER=1.125                                                      
      K0=' 0'                                                           
      K180=' 180'                                                       
  100 NFMR=0                                                            
      NO=FNO                                                            
      DO 1 I=1,NRP                                                      
      IF (SYM(I) .EQ. 'N') SYM(I)=' '                                   
      IF (SYM(I) .EQ. ' ') GO TO 1                                      
      IF (SYM(I) .EQ. 'U') SYM(I)='C'                                   
      NFMR=NFMR+1                                                       
    1 CONTINUE                                                          
      IF (NFMR .LT. KFM) RETURN                                         
      WRITE(8,2)                                                        
    2 FORMAT(1H1,'  DATE    ORIGIN     LAT      LONG     DEPTH'         
     1,'    MAG NO GAP DMIN  RMS  ERH  ERZ QM')                         
      WRITE(8,5) SUCARD                                                 
    5 FORMAT(2X,A80)                                                    
      DO 10 I=1,NOX                                                     
      DO 10 J=1,NOY                                                     
   10 GRAPH(I,J)=' '                                                    
      DO 20 I=1,180                                                     
      RI = FLOAT(I) * 0.0349066                                         
      X = RMAX * COS(RI) + ADD                                          
      Y = RMAX * SIN(RI) + ADD                                          
      JX=X/XSCALE+1.5                                                   
C*** SCALE JX FOR DIFFERENT PRINTER PAPER SIZES                         
      JX = XPAPER*JX + 0.5                                              
      JY=Y/YSCALE+.5                                                    
      JY=NOY-JY-1                                                       
   20 GRAPH(JX,JY)='*'                                                  
      IT=NOX2-IX-1                                                      
C*** SCALE IT FOR DIFFERENT PRINTER PAPER SIZES                         
      IT = XPAPER*IT + 0.5                                              
      GRAPH(IT,NOY2)='-'                                                
      IT=NOX2+IX+1                                                      
C*** SCALE IT FOR DIFFERENT PRINTER PAPER SIZES                         
      IT = XPAPER*IT + 0.5                                              
      GRAPH(IT,NOY2)='-'                                                
C*** SCALE JX FOR DIFFERENT PRINTER PAPER SIZES                         
      JX = XPAPER*NOX2 + 0.5                                            
      IT=NOY2-IY-1                                                      
      GRAPH(JX,IT)='I'                                                  
      IT=NOY2+IY+1                                                      
      GRAPH(JX,IT)='I'                                                  
      DO 50 I=1,NRP                                                     
      IF (SYM(I) .EQ. ' ') GO TO 50                                     
      IF (AIN(I) .GT. 90.) GO TO 31                                     
      ANN=AIN(I)                                                        
      AZZ=AZ(I)*.0174533                                                
      GO TO 32                                                          
   31 ANN=180.-AIN(I)                                                   
      AZZ=(180.+AZ(I))*.0174533                                         
   32 R=RMAX*1.414214*SIN(ANN*.0087266)                                 
      X=R*SIN(AZZ)+ADD                                                  
      Y=R*COS(AZZ)+ADD                                                  
      JX=X/XSCALE+1.5                                                   
C*** SCALE JX FOR DIFFERENT PRINTER PAPER SIZES                         
      JX = XPAPER*JX + 0.5                                              
      JY=Y/YSCALE+.5                                                    
      JY=NOY-JY-1                                                       
      TEMP=GRAPH(JX,JY)                                                 
C-----OVER-WRITE TEMP IF IT IS EQUAL TO BLANK,DOT,*,+,OR -              
      IF ((TEMP.EQ.' ').OR.(TEMP.EQ.'*').OR.(TEMP.EQ.'+')               
     1.OR.(TEMP.EQ.'-').OR.(TEMP.EQ.'.')) GO TO 47                      
C-----TEMP IS OCCUPIED SO IF SYS(I)=+ OR - SKIP THIS STATION            
      IF ((SYM(I).EQ.'+').OR.(SYM(I).EQ.'-')) GO TO 50                  
      IF (SYM(I) .EQ. 'C') GO TO 40                                     
      IF (GRAPH(JX,JY) .NE. 'D') GO TO 35                               
      GRAPH(JX,JY)='E'                                                  
      GO TO 50                                                          
   35 IF (GRAPH(JX,JY) .NE. 'E') GO TO 37                               
      GRAPH(JX,JY)='F'                                                  
      GO TO 50                                                          
   37 IF (GRAPH(JX,JY) .EQ. 'F') GO TO 50                               
      GRAPH(JX,JY)='X'                                                  
      GO TO 50                                                          
   40 IF (GRAPH(JX,JY) .NE. 'C') GO TO 43                               
      GRAPH(JX,JY)='B'                                                  
      GO TO 50                                                          
   43 IF (GRAPH(JX,JY) .NE. 'B') GO TO 45                               
      GRAPH(JX,JY)='A'                                                  
      GO TO 50                                                          
   45 IF (GRAPH(JX,JY) .EQ. 'A') GO TO 50                               
      GRAPH(JX,JY)='X'                                                  
      GO TO 50                                                          
   47 GRAPH(JX,JY)=SYM(I)                                               
   50 CONTINUE                                                          
C*** SCALE JX FOR DIFFERENT PRINTER PAPER SIZES                         
      JX = XPAPER*NOX2 + 0.5                                            
      GRAPH(JX,NOY2)='*'                                                
      WRITE(8,61) K0                                                    
   61 FORMAT(1H0,67X,A2)                                                
      DO 80 I=3,NOY1                                                    
      IF (I .EQ .NOY2) GO TO 70                                         
      IF (KPAPER .EQ. 0) WRITE(8,65) (GRAPH(J,I),J=1,NOX)               
   65 FORMAT(1H ,20X,95A1)                                              
      IF (KPAPER .EQ. 1) WRITE(8,66) (GRAPH(J,I),J=1,NOX)               
   66 FORMAT(1H ,15X,107A1)                                             
      GO TO 80                                                          
   70 IF (KPAPER .EQ. 0) WRITE(8,75) (GRAPH(J,I),J=1,NOX)               
   75 FORMAT(1H ,16X,'270 ',95A1,' 90')                                 
      IF (KPAPER .EQ. 1) WRITE(8,76) (GRAPH(J,I),J=1,NOX)               
   76 FORMAT(1H ,11X,'270 ',107A1,' 90')                                
   80 CONTINUE                                                          
      WRITE(8,85) K180                                                  
   85 FORMAT(67X,A4)                                                    
      RETURN                                                            
      END                                                               
      SUBROUTINE MISING(NSTA,LAT,LON,NS,MAG,TEMP,DMIN,JDX,              
     &  JMAX,LATEP,LONEP,INS,IEW)                                       
	include 'H_param.f'
C------- CHECK MISSING STATIONS ----------------------------------------
      REAL LATEP,LONEP,MAG                                              
      CHARACTER*4 NSTA(N_s)                                             
      REAL*4 LAT(N_s),LON(N_s)                                          
      REAL*4 TEMP(N_dat)                                                  
      INTEGER*4 JDX(N_s)                                                
      CHARACTER*1 INS(N_s),IEW(N_s)                                     
C-----------------------------------------------------------------------
      IHD=0                                                             
      NJ=JMAX+1                                                         
      TEMP(NJ)=TEMP(1)+360.                                             
      TDEL=25.*MAG**2                                                   
      IF (MAG .EQ. 99.9) TDEL=100.                                      
      DO 30 I=1,NS                                                      
      IF (JDX(I) .EQ. 1) GO TO 30                                       
      PHI = 0.0174532 * ((LAT(I)+LATEP)/120.)                           
      SINPHI = SIN(PHI)                                                 
      SINP2  = SINPHI**2                                                
      SINP4  = SINP2**2                                                 
      CA = 1.8553654 + 0.0062792*SINP2 + 0.0000319*SINP4                
      CB = 1.8428071 + 0.0187098*SINP2 + 0.0001583*SINP4                
      DXI = (LON(I)-LONEP) * CA * COS(PHI)                              
      DYI = (LAT(I)-LATEP) * CB                                         
      DELI=SQRT(DXI**2+DYI**2)+0.000001                                 
      IF (DELI .GT. TDEL) GO TO 30                                      
C     CHECK LATITUDE AND LONGITUDE                                      
      IF (INS(I)  .EQ. 'S') DYI=-DYI                                    
      IF (IEW(I) .EQ. 'W') DXI=-DXI                                     
      AZI=AMOD(ATAN2(DXI,DYI)*57.29578 + 360., 360.)                    
      IF (AZI .LE. TEMP(1)) AZI=AZI+360.                                
      DO 10 J=2,NJ                                                      
      IF (AZI .LT. TEMP(J)) GO TO 20                                    
   10 CONTINUE                                                          
      J=NJ                                                              
   20 EXGAP=TEMP(J)-TEMP(J-1)                                           
      RDGAP=TEMP(J)-AZI                                                 
      TGAP=AZI-TEMP(J-1)                                                
      IF (TGAP .LT. RDGAP) RDGAP=TGAP                                   
      IF ((DELI.GT.DMIN).AND.(RDGAP.LT.30.)) GO TO 30                   
      IF (AZI .GE. 360.) AZI=AZI-360.                                   
      IF (IHD .EQ. 1) GO TO 22                                          
      WRITE(8,5)                                                        
    5 FORMAT(/,10X,'MISSING STATION  DELTA   AZIM  EX-GAP  RD-GAP')     
      IHD=1                                                             
   22 WRITE(8,25) NSTA(I),DELI,AZI,EXGAP,RDGAP                          
   25 FORMAT(21X,A4,2F7.1,2F8.1)                                        
   30 CONTINUE                                                          
      RETURN                                                            
      END                                                               
      SUBROUTINE SORT(X,KEY,NO)                                         
      DIMENSION X(NO),KEY(NO)                                           
C-----------------------------------------------------------------------
      DO 1 I=1,NO                                                       
 1    KEY(I)=I                                                          
      MO=NO                                                             
 2    IF (MO-15) 21,21,23                                               
 21   IF (MO-1) 29,29,22                                                
 22   MO=2*(MO/4)+1                                                     
      GO TO 24                                                          
 23   MO=2*(MO/8)+1                                                     
 24   KO=NO-MO                                                          
      JO=1                                                              
 25   I=JO                                                              
 26   IF (X(I)-X(I+MO)) 28,28,27                                        
 27   TEMP=X(I)                                                         
      X(I)=X(I+MO)                                                      
      X(I+MO)=TEMP                                                      
      KEMP=KEY(I)                                                       
      KEY(I)=KEY(I+MO)                                                  
      KEY(I+MO)=KEMP                                                    
      I=I-MO                                                            
      IF (I-1) 28,26,26                                                 
 28   JO=JO+1                                                           
      IF (JO-KO) 25,25,2                                                
 29   RETURN                                                            
      END                                                               
      SUBROUTINE SUMOUT(ISW,NSTA,INS,IEW,IELV,DLY,FMGC,XMGC,KLAS,PRR,   
     &  CALR,ICAL,NDATE,NHRMN,LAT,LON,MDATE,MHRMN,KLSS,CALS,NS,QNO,IPUN,
     &  MNO,NRES,NXM,NFM,SR,SRSQ,SRWT,SXM,SXMSQ,SFM,SFMSQ)              
	include 'H_param.f'
C------- OUTPUT SUMMARY OF TIME AND MAGNITUDE RESIDUALS ----------------
      CHARACTER*1 INS(N_s),IEW(N_s)                                     
      CHARACTER*4 ISW,NSTA(N_s)                                         
      INTEGER*4 NRES(2,N_s)                                             
      INTEGER*4 MNO(N_s),KLAS(N_s),ICAL(N_s),KLSS(N_s),NXM(N_s),NFM(N_s)
      INTEGER*4 NDATE(N_s),NHRMN(N_s),MDATE(N_s),MHRMN(N_s),IELV(N_s)   
      REAL*4 LAT2,LON2,QNO(4)                                           
      REAL*4 DLY(2,N_s),SR(2,N_s),SRSQ(2,N_s),SRWT(2,N_s)               
      REAL*4 AVRES(4,N_s),SDRES(4,N_s)                                  
      REAL*4 LAT(N_s),LON(N_s),PRR(N_s),CALR(N_s),CALS(N_s)             
      REAL*4 XMGC(N_s),FMGC(N_s),SXM(N_s),SXMSQ(N_s),SFM(N_s),SFMSQ(N_s)
C-----------------------------------------------------------------------
      QSUM=QNO(1)+QNO(2)+QNO(3)+QNO(4)                                  
      IF (QSUM .EQ. 0.) GO TO 72                                        
      WRITE(8,5) (QNO(I),I=1,4),QSUM                                    
    5 FORMAT(1H1,' ***** CLASS:     A     B     C     D TOTAL *****'    
     1,//,7X,'NUMBER:',5F6.1)                                           
      DO 10 I=1,4                                                       
   10 QNO(I)=100.*QNO(I)/QSUM                                           
      WRITE(8,15)(QNO(I),I=1,4)                                         
   15 FORMAT(/,12X,'%:',4F6.1)                                          
      WRITE(8,20)                                                       
   20 FORMAT(///,10X,'TRAVELTIME RESIDUALS (MODEL=1)',5X                
     1,'TRAVELTIME RESIDUALS (MODEL=2)',5X,'X-MAGNITUDE RESIDUALS'      
     2,6X,'F-MAGNITUDE RESIDUALS',/,' STATION   NRES    SRWT   AVRES   S
     3DRES       NRES    SRWT   AVRES   SDRES        NXM    AVXM    SDXM
     4        NFM    AVFM    SDFM')                                     
      DO 70 I=1,NS                                                      
      DO 30 J=1,4                                                       
      AVRES(J,I)=0.                                                     
   30 SDRES(J,I)=0.                                                     
      IF (NRES(1,I) .EQ. 0) GO TO 35                                    
      AVRES(1,I)=SR(1,I)/SRWT(1,I)                                      
      SDRES(1,I)=SQRT(SRSQ(1,I)/SRWT(1,I)-AVRES(1,I)**2+0.000001)       
   35 IF (NRES(2,I) .EQ. 0) GO TO 40                                    
      AVRES(2,I)=SR(2,I)/SRWT(2,I)                                      
      SDRES(2,I)=SQRT(SRSQ(2,I)/SRWT(2,I)-AVRES(2,I)**2+0.000001)       
   40 IF (NXM(I) .EQ. 0) GO TO 50                                       
      AVRES(3,I)=SXM(I)/NXM(I)                                          
      SDRES(3,I)=SQRT(SXMSQ(I)/NXM(I)-AVRES(3,I)**2+0.000001)           
   50 IF (NFM(I) .EQ. 0) GO TO 60                                       
      AVRES(4,I)=SFM(I)/NFM(I)                                          
      SDRES(4,I)=SQRT(SFMSQ(I)/NFM(I)-AVRES(4,I)**2+0.000001)           
   60 WRITE(8,65) NSTA(I),NRES(1,I),SRWT(1,I),AVRES(1,I),SDRES(1,I)     
     1,NRES(2,I),SRWT(2,I),AVRES(2,I),SDRES(2,I),NXM(I),AVRES(3,I)      
     2,SDRES(3,I),NFM(I),AVRES(4,I),SDRES(4,I)                          
   65 FORMAT(4X,A4,2X,I5,3F8.2,6X,I5,3F8.2,2(6X,I5,2F8.2))              
   70 CONTINUE                                                          
   72 IF (IPUN .NE. 3) GO TO 200                                        
C------- PUNCH STATION LIST WITH REVISED DELAYS,XMGC,AND FMGC ----------
      IF (ISW .EQ. '1   ') GO TO 80                                     
      WRITE(8,75) INS(1),IEW(1)                                         
   75 FORMAT(1H1,' ***** NEW STATION LIST *****'                        
     1,///,    4X,'I   STN  LAT ',A1,'    LONG ',A1,'   ELV DELAY',5X   
     2,'FMGC  XMGC KL  PRR  CALR IC IS   DATE HRMN')                    
      GO TO 90                                                          
   80 WRITE(8,85) INS(1),IEW(1)                                         
   85 FORMAT(1H1,' ***** NEW STATION LIST *****'                        
     1,///,4X,'I   STN   LAT ',A1,'     LONG ',A1,'    ELV     M  DLY1  
     2DLY2  XMGC  FMGC K  CALR IC  DATE HRMN')                          
   90 DO 120 I=1,NS                                                     
      DLY(1,I)=DLY(1,I)+AVRES(1,I)                                      
      IF (ISW .EQ. '1   ') DLY(2,I)=DLY(2,I)+AVRES(2,I)                 
      XMGC(I)=XMGC(I)+AVRES(3,I)                                        
      FMGC(I)=FMGC(I)+AVRES(4,I)                                        
      LAT1=LAT(I)/60.                                                   
      LAT2=LAT(I)-60.*LAT1                                              
      LON1=LON(I)/60.                                                   
      LON2=LON(I)-60.*LON1                                              
      IF (ISW .EQ. '1   ') GO TO 115                                    
      WRITE(8,105) I,NSTA(I),LAT1,LAT2,INS(I),LON1,LON2,IEW(I),IELV(I)  
     1,DLY(1,I),FMGC(I),XMGC(I),KLSS(I),PRR(I),CALS(I),ICAL(I),NDATE(I) 
     2,NHRMN(I)                                                         
  105 FORMAT(I5,2X,A4,I2,F5.2,A1,I4,F5.2,A1,I5,F6.2,4X,F5.2,2X,F5.2,I2  
     1,1X,F4.2,1X,F6.2,I2,5X,I6,I4)                                     
      WRITE(7,110)   NSTA(I),LAT1,LAT2,INS(I),LON1,LON2,IEW(I),IELV(I)  
     1,DLY(1,I),FMGC(I),XMGC(I),KLSS(I),PRR(I),CALS(I),ICAL(I),NDATE(I) 
     2,NHRMN(I)                                                         
  110 FORMAT(2X,A4,I2,F5.2,A1,I3,F5.2,A1,I4,F6.2,T38,F5.2,T45,F5.2      
     1,I2,1X,F4.2,1X,F6.2,I2,T71,I6,I4)                                 
      GO TO 120                                                         
  115 WRITE(8,116) I,NSTA(I),LAT1,LAT2,INS(I),LON1,LON2,IEW(I),IELV(I)  
     1,MNO(I),DLY(1,I),DLY(2,I),XMGC(I),FMGC(I),KLSS(I),CALS(I),ICAL(I) 
     2,NDATE(I),NHRMN(I)                                                
  116 FORMAT(I5,2X,A4,I3,'-',F5.2,A1,I4,'-',F5.2,A1,I5,I6,2F6.2         
     1,2F6.2,I2,F6.2,I2,2X,I6,I4)                                       
      WRITE(7,117)   NSTA(I),LAT1,LAT2,INS(I),LON1,LON2,IEW(I),IELV(I)  
     1,MNO(I),DLY(1,I),DLY(2,I),XMGC(I),FMGC(I),KLSS(I),CALS(I),ICAL(I) 
     2,NDATE(I),NHRMN(I)                                                
  117 FORMAT(A4,I3,'-',F5.2,A1,I3,'-',F5.2,A1,I4,I6,2F6.2               
     1,2F6.2,I2,F6.2,I2,2X,I6,I4)                                       
  120 CONTINUE                                                          
      RETURN                                                            
C------- PUNCH STATION LIST WITH REVISED CALIBRATIONS ------------------
  200 IF (IPUN .NE. 4) RETURN                                           
      IF (ISW .EQ. '1   ') GO TO 205                                    
      WRITE(8,75) INS(1),IEW(1)                                         
      GO TO 206                                                         
  205 WRITE(8,85) INS(1),IEW(1)                                         
  206 DO 220 I=1,NS                                                     
      LAT1=LAT(I)/60.                                                   
      LAT2=LAT(I)-60.*LAT1                                              
      LON1=LON(I)/60.                                                   
      LON2=LON(I)-60.*LON1                                              
      IF (ISW .EQ. '1   ') GO TO 210                                    
      WRITE(8,105) I,NSTA(I),LAT1,LAT2,INS(I),LON1,LON2,IEW(I),IELV(I)  
     1,DLY(1,I),FMGC(I),XMGC(I),KLAS(I),PRR(I),CALR(I),ICAL(I),MDATE(I) 
     2,MHRMN(I)                                                         
      WRITE(7,110)   NSTA(I),LAT1,LAT2,INS(I),LON1,LON2,IEW(I),IELV(I)  
     1,DLY(1,I),FMGC(I),XMGC(I),KLAS(I),PRR(I),CALR(I),ICAL(I),MDATE(I) 
     2,MHRMN(I)                                                         
      GO TO 220                                                         
  210 WRITE(8,116) I,NSTA(I),LAT1,LAT2,INS(I),LON1,LON2,IEW(I),IELV(I)  
     1,MNO(I),DLY(1,I),DLY(2,I),XMGC(I),FMGC(I),KLAS(I),CALR(I),ICAL(I) 
     2,MDATE(I),MHRMN(I)                                                
      WRITE(7,117)   NSTA(I),LAT1,LAT2,INS(I),LON1,LON2,IEW(I),IELV(I)  
     1,MNO(I),DLY(1,I),DLY(2,I),XMGC(I),FMGC(I),KLAS(I),CALR(I),ICAL(I) 
     2,MDATE(I),MHRMN(I)                                                
  220 CONTINUE                                                          
      RETURN                                                            
      END                                                               
      SUBROUTINE SWMREG(TEST,IPRN,NR,KSMP,FNO,X,W,ISKP,KF,KZ,XMEAN,     
     &  B,Y,BSE,AF,ONF,FLIM)                                            
C------- COMPUTE GEIGER ADJUSTMENTS BY STEP-WISE MULTIPLE REGRESSION OF 
C        TRAVEL TIME RESIDUALS -----------------------------------------
	include 'H_param.f'
      INTEGER*4 ISKP(4),IDX(4),KSMP(N_s)                                
      REAL*4 AF(3),V(3),PF(3),TEST(15),A(7,7),T(7,7)                    
      REAL*4 B(4),Y(4),BSE(4),XMEAN(4),XSUM(4),SIGMA(4),S(4,4)          
      REAL*4 W(N_dat),X(4,N_dat)                                            
      DATA L,M,MM,M1/3,4,7,5/                                           
C-----------------------------------------------------------------------
      KFLAG=0                                                           
      SVTEST = TEST(3)                                                  
      ONF=0.0                                                           
      FLIM = TEST(3)                                                    
      DO 2 I=1,3                                                        
      AF(I)=-1.00                                                       
    2 CONTINUE                                                          
      DO 5 I=1,NR                                                       
      ONF=ONF + W(I)*(1-KSMP(I))                                        
    5 CONTINUE                                                          
      DO 10 I=1,MM                                                      
      DO 10 J=1,MM                                                      
   10 A(I,J)=0.                                                         
C-----COMPUTE MEANS,STANDARD DEVIATIONS,AND CORRECTED SUMS OF SQUARE    
      DO 40 I=1,M                                                       
      XSUM(I)=0.                                                        
      XMEAN(I)=0.                                                       
      DO 40 J=1,M                                                       
   40 S(I,J)=0.                                                         
      DO 50 K=1,NR                                                      
      DO 50 I=1,M                                                       
      TEMP=X(I,K)*W(K)                                                  
      ETMP=TEMP*(1-KSMP(K))                                             
      XSUM(I)=XSUM(I)+ETMP                                              
      DO 50 J=I,M                                                       
   50 S(I,J)=S(I,J)+TEMP*X(J,K)                                         
      DO 70 I=1,M                                                       
      IF (ONF .EQ. 0.) GO TO 65                                         
      XMEAN(I)=XSUM(I)/ONF                                              
      DO 60 J=I,M                                                       
   60 S(I,J)=S(I,J)-XSUM(I)*XSUM(J)/ONF                                 
   65 A(I,I)=1.                                                         
      IF (S(I,I) .LT. 0.000001) S(I,I)=0.000001                         
      SIGMA(I)=SQRT(S(I,I))                                             
   70 CONTINUE                                                          
C-----COMPUTE AND AUGMENT CORRELATION MATRIX A                          
      DO 80 I=1,L                                                       
      I1=I+1                                                            
      DO 80 J=I1,M                                                      
      A(I,J)=S(I,J)/(SIGMA(I)*SIGMA(J))                                 
   80 A(J,I)=A(I,J)                                                     
      PHI=FNO-1.                                                        
      DO 120 I=M1,MM                                                    
      A(I-M,I)=1.                                                       
  120 A(I,I-M)=-1.                                                      
      DO 140 I=1,M                                                      
      B(I)=0.                                                           
      Y(I)=0.                                                           
      BSE(I)=0.                                                         
  140 IDX(I)=0                                                          
      IF (IPRN .LT. 3) GO TO 150                                        
      WRITE(8,45)                                                       
   45 FORMAT(///, '***** DATA *****',//,4X,'K',8X,'W'                   
     1,14X,'X1',14X,'X2',14X,'X3',14X,'X4',/)                           
      DO 47 K=1,NR                                                      
      WRITE(8,46) K,W(K),(X(I,K),I=1,M)                                 
   46 FORMAT(I5,8E16.8)                                                 
   47 CONTINUE                                                          
      WRITE(8,75) (XMEAN(I),I=1,M)                                      
   75 FORMAT(/,' MEAN',16X,8E16.8)                                      
      WRITE(8,76) (SIGMA(I),I=1,M)                                      
   76 FORMAT(/,' SIGMA',15X,7E16.8)                                     
      WRITE(8,77)                                                       
   77 FORMAT(///,' ***** CORRECTED SUMS OF SQUARES MATRIX *****',/)     
      DO 78 I=1,M                                                       
   78 WRITE(8,95) (S(I,J),J=1,M)                                        
      WRITE(8,85)                                                       
   85 FORMAT(///,' ***** CORRELATION MATRIX R *****',/)                 
      DO 90 I=1,M                                                       
   90 WRITE(8,95) (A(I,J),J=1,M)                                        
   95 FORMAT(7E18.8)                                                    
C-----STEPWISE MULTIPLE REGRESSION                                      
      WRITE(8,125) NR,L,TEST(3)                                         
  125 FORMAT(///, '********** STEPWISE MULTIPLE REGRESSION ANALYSIS'    
     1,' **********',//' NUMBER OF DATA....................',I5         
     2,              /,' NUMBER OF INDEPENDENT VARIABLES...',I5         
     3,              /,' CRITICAL F-VALUE..................',F8.2)      
  150 DO 300 NSTEP=1,L                                                  
      NU=0                                                              
      MU=0                                                              
      IF (IPRN .LT. 3) GO TO 155                                        
      WRITE(8,154) NSTEP,KZ,KF                                          
  154 FORMAT(//,' ***** STEP NO.',I2,' *****',5X,'KZ =',I2,5X,'KF =',I2)
C-----FIND VARIABLE TO ENTER REGRESSION                                 
  155 VMAX=0.                                                           
      MAX=NSTEP                                                         
      DO 160 I=1,L                                                      
      IF(ISKP(I).EQ.1) GO TO 160                                        
      IF (IDX(I) .EQ. 1) GO TO 160                                      
      IF ((I.EQ.3).AND.(KZ.EQ.1)) GO TO 160                             
      V(I)=A(I,M)*A(M,I)/A(I,I)                                         
      IF (V(I) .LE. VMAX) GO TO 160                                     
      VMAX=V(I)                                                         
      MAX=I                                                             
  160 CONTINUE                                                          
      F=0.0                                                             
      IF(VMAX.EQ.0.0) GO TO 163                                         
      F=(PHI-1.)*VMAX/(A(M,M)-VMAX)                                     
      IF(F .GE. 1000.) F=999.99                                         
  163 AF(MAX)=F                                                         
      IF(KF .GE. 2) GO TO 165                                           
      IF (F .LT. TEST(3)) GO TO 400                                     
  165 IF ((MAX.EQ.3).AND.(KZ.EQ.1)) GO TO 300                           
      NU=MAX                                                            
      IDX(NU)=1                                                         
      PHI=PHI-1.                                                        
C-----COMPUTE MATRIX T FOR THE ENTRANCE OF VARIABLE X(NU)               
      DO 170 J=1,MM                                                     
  170 T(NU,J)=A(NU,J)/A(NU,NU)                                          
      DO 180 I=1,MM                                                     
      IF (I .EQ. NU) GO TO 180                                          
      DO 175 J=1,MM                                                     
  175 T(I,J)=A(I,J)-A(I,NU)*A(NU,J)/A(NU,NU)                            
  180 CONTINUE                                                          
      DO 190 I=1,MM                                                     
      DO 190 J=1,MM                                                     
  190 A(I,J)=T(I,J)                                                     
      DO 200 I=1,L                                                      
      IF (IDX(I) .EQ. 0) GO TO 200                                      
      IF (ABS(A(M,M)*A(I+M,I+M)) .LT. .000001 ) GO TO 195               
      PF(I)=PHI*A(I,M)**2/(A(M,M)*A(I+M,I+M))                           
      IF(PF(I) .GE. 1000.0) PF(I)=999.99                                
      AF(I) = PF(I)                                                     
      GO TO 200                                                         
  195 PF(I) = 999.99                                                    
  200 CONTINUE                                                          
      IF (IPRN .LT. 3) GO TO 210                                        
      CALL ANSWER(A,S,XMEAN,SIGMA,IDX,PHI,L,M,MM,PF,NU,'ENTERING')      
  210 IF (KF .EQ. 2) GO TO 300                                          
      IF(KF .GE. 3) GO TO 450                                           
C-----FIND VARIABLE TO LEAVE REGRESSION                                 
      DO 250 K=1,L                                                      
      IF (IDX(K) .EQ. 0) GO TO 250                                      
      IF (PF(K) .GE. TEST(3)) GO TO 250                                 
      MU=K                                                              
      F=PF(MU)                                                          
      IDX(MU)=0                                                         
      PHI=PHI+1.                                                        
      DO 220 J=1,MM                                                     
  220 T(MU,J)=A(MU,J)/A(MU+M,MU+M)                                      
      DO 230 I=1,MM                                                     
      IF (I .EQ. MU) GO TO 230                                          
      DO 225 J=1,MM                                                     
      IF (J .EQ. MU) GO TO 225                                          
      T(I,J)=A(I,J)-A(I,MU+M)*A(MU+M,J)/A(MU+M,MU+M)                    
  225 CONTINUE                                                          
  230 CONTINUE                                                          
      DO 240 I=1,MM                                                     
      IF (I .EQ. MU) GO TO 240                                          
      T(I,MU)=A(I,MU)-A(I,MU+M)/A(MU+M,MU+M)                            
  240 CONTINUE                                                          
      DO 245 I=1,MM                                                     
      DO 245 J=1,MM                                                     
  245 A(I,J)=T(I,J)                                                     
      IF (IPRN .LT. 3) GO TO 250                                        
      CALL ANSWER(A,S,XMEAN,SIGMA,IDX,PHI,L,M,MM,PF,MU,'LEAVING')       
  250 CONTINUE                                                          
  300 CONTINUE                                                          
C-----CHECK TERMINATION CONDITION                                       
  400 KOUT=0                                                            
      DO 410 I=1,L                                                      
  410 KOUT=KOUT+IDX(I)                                                  
      B(4)=XMEAN(M)                                                     
      IF (KOUT .NE. 0) GO TO 450                                        
      IF(KF .NE. 1) GO TO 420                                           
      KF = 3                                                            
      GO TO 150                                                         
  420 TEST(3)= TEST(3)/TEST(6)                                          
      FLIM=TEST(3)                                                      
      KF=1                                                              
      KFLAG = 0                                                         
      IF(TEST(6) .GT. 1.) GO TO 150                                     
      KFLAG = 1                                                         
      KF = 4                                                            
      GO TO 150                                                         
C-----COMPUTE REGRESSION CONSTANT,COEFFICIENTS,AND STANDARD ERRORS      
  450 YSE=77.7                                                          
      IF (PHI .GE. 1) YSE=SIGMA(M)*SQRT(ABS(A(M,M)/PHI))                
      DO 500 I=1,L                                                      
      IF (IDX(I) .EQ. 0) GO TO 500                                      
      B(I)=A(I,M)*SQRT(S(M,M)/S(I,I))                                   
      BSE(I)=YSE*SQRT(ABS(A(I+M,I+M)/S(I,I)))                           
      IF(KF .NE. 3) Y(I)=B(I)                                           
      IF(KFLAG .EQ. 0) GO TO 480                                        
      IF(ABS(B(I)) .LE. TEST(6)*BSE(I)) Y(I)=0.                         
  480 IF(PHI .LT. 1.) BSE(I) = 0.                                       
      B(4)=B(4)-Y(I)*XMEAN(I)                                           
  500 CONTINUE                                                          
      IF(KF .NE. 3) Y(4)=B(4)                                           
      TEST(3)=SVTEST                                                    
      RETURN                                                            
      END                                                               
