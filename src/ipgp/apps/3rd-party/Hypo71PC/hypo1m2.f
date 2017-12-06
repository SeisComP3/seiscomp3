          SUBROUTINE INPUT2(NSTA,KLAS,MMAX,IW,MSTA,PRMK,W,JMIN,P,S,         
     &  SRMK,WS,AMX,PRX,CALX,RMK,DT,FMP,AZRES,SYM,QRMK,KDX,LDX,JDX,     
     &  TP,WRK,KSMP,PMIN,TS,CALR,TIME1,TIME2,MDATE,                     
     &  MHRMN,NR,MJUMP)                                                 
         include 'H_param.f'
C------- INPUT PHASE LIST ----------------------------------------------
      CHARACTER*1 IW(N_s),SYM(N_d),QRMK(N_d) 
      CHARACTER*3 RMK(N_d)                                              
      CHARACTER*4 AS,IPRO,NSTA(N_s),ISW                                 
      CHARACTER*48 AHEAD                                                
      CHARACTER*4 MSTA(N_d),PRMK(N_d),SRMK(N_d),AZRES(N_d),WRK(N_d)     
      CHARACTER*80 ICARD                                                
      INTEGER*4 JMIN(N_d),KDX(N_d),LDX(N_d)                             
      INTEGER*4 JDX(N_s),KSMP(N_s),KLAS(N_s),MDATE(N_s),MHRMN(N_s)      
      REAL*4 W(N_d),P(N_d),S(N_d),WS(N_d),AMX(N_d),PRX(N_d),CALX(N_d)   
      REAL*4 DT(N_d),FMP(N_d),TP(N_d),TS(N_d)                           
      REAL*4 CALR(N_s)                                                  
      REAL*8 TIME1,TIME2                                                
      COMMON/C3/ AHEAD,IPRO,ISW                                         
      COMMON/C4/ NL,NS,KDATE,KHR,NEAR,IEXIT,IDXS                        
         common/dts/stdt(N_s)
C-----------------------------------------------------------------------
   10 PMIN=9999.                                                        
      IDXS=0                                                            
      DO 20 I=1,NS                                                      
      KSMP(I)=0                                                         
   20 JDX(I)=0                                                          
      L=1                                                               
   30 read(12,'(A)',END=300) ICARD
      read(ICARD,35)
     1 MSTA(L),PRMK(L),W(L),JTIME,jz,JMIN(L),P(L),S(L)
     2,SRMK(L),WS(L),AMX(L),PRX(L),CALP,CALX(L),RMK(L),DT(L),FMP(L)     
     3,AZRES(L),SYM(L),AS,QRMK(L),IPRO                            
         jtime=jtime*100+jz
   35 FORMAT(2A4,T8,F1.0,T10,I6,i2,I2,F5.2,T32,F5.2,A4,T40,F1.0,T44,F4.0   
     1,F3.2,F4.1,T59,F4.1,A3,F5.2,F5.0,T21,A4,T7,A1,T32,A4,T63,A1,T5,A4)
         if(msta(l).eq.'$   ')then
                  mjump=3
                  return
         endif
      IF ((MSTA(L).EQ.' ***').OR.(MSTA(L).EQ.' $$$').OR.                
     &   (MSTA(L).EQ.' ###')) GOTO 300                                  
      IF (MSTA(L).EQ.'    ') GO TO 350                                  
      IF (CALX(L) .LT. 0.01) CALX(L)=CALP                               
      IF (AS .EQ. '    ') S(L) = 999.99                                 
      DO 40 I=1,NS                                                      
      IF (MSTA(L) .EQ. NSTA(I)) GO TO 50                                
   40 CONTINUE                                                          
      WRITE(8,45) ICARD,MSTA(L)                                         
   45 FORMAT(///,' ***** ',A80,' ***** DELETED: ',A4,' NOT ON STATION L'
     1,'IST')                                                           
      GO TO 30                                                          
   50 KDX(L)=I                                                          
      LDX(L)=0                                                          
      JDX(I)=1                                                          
         dt(l)=dt(l)+stdt(i)
      IF (FMP(L) .LE. 0.) FMP(L)=0.                                     
      IF (L .GT. 1) GO TO 60                                            
      KTIME=JTIME                                                       
      KDATE=KTIME/100                                                   
      KHR=KTIME-KDATE*100                                               
   60 IF (JTIME .EQ. KTIME) GO TO 70                                    
      WRITE(8,65) ICARD                                                 
   65 FORMAT(///,' ***** ',A80,' ***** DELETED: WRONG TIME')            
      GO TO 30                                                          
   70 IF (RMK(L) .EQ. 'CAL') GO TO 200                                  
      W(L)=(4.-W(L))/4.                                                 
      IF (IW(I) .EQ. '*') W(L)=0.                                       
      TP(L)=60.*JMIN(L)+P(L)+DT(L) 
      WRK(L)='    '                                                     
      IF (W(L) .EQ. 0.) GO TO 90                                        
      IF (W(L) .GT. 0.) GO TO 89                                        
C------- SMP DATA: RESET WEIGHT ----------------------------------------
      W(L)=(4.-WS(L))/4.                                                
      KSMP(L)=1                                                         
      IF(TP(L).GE.PMIN) GO TO 95                                        
      PMIN=TP(L)                                                        
      NEAR=L                                                            
      GO TO 95                                                          
   89 IF (TP(L) .GE. PMIN) GO TO 90                                     
      PMIN=TP(L)                                                        
      NEAR=L                                                            
   90 IF (AS .EQ. '    ') GO TO 100                                     
C------- S DATA --------------------------------------------------------
      IDXS=1                                                            
      LDX(L)=1                                                          
      WS(L)=(4.-WS(L))/4.                                               
      IF (IW(I) .EQ. '*') WS(L)=0.                                      
   95 TS(L)=60.*JMIN(L)+S(L)+DT(L)                                      
  100 L = L + 1                                                         
      IF (L .LT. MMAX) GO TO 30                                         
      WRITE(8,105)                                                      
  105 FORMAT(///,' ***** ERROR: PHASE LIST EXCEEDS ARRAY DIMENSION; EXTR
     1A DATA TREATED AS NEXT EARTHQUAKE')                               
      GO TO 350                                                         
C------- CALIBRATION CHANGE IN STATION LIST ----------------------------
  200 IF (P(L) .NE. 0.) KLAS(I)=P(L)                                    
      CALR(I)=CALX(L)                                                   
      TIME2=1.D+06*KDATE+1.D+04*KHR+1.D+02*JMIN(L)                      
      IF (TIME2 .GE. TIME1) GO TO 250                                   
      WRITE(8,205)                                                      
  205 FORMAT(///,' ********** THE FOLLOWING EVENT IS OUT OF CHRONOLOGICA
     1L ORDER **********')                                              
  250 WRITE(8,255) KDATE,KHR,JMIN(L),MSTA(L),KLAS(I),CALR(I)            
  255 FORMAT(///,' ***** ',I6,1X,2I2,' ***** CALIBRATION CHANGE FOR ',A4
     1,': KLAS = ',I1,', CALR = ',F4.1)                                 
      MDATE(I)=KDATE                                                    
      MHRMN(I)=100*KHR+JMIN(L)                                          
      TIME1=TIME2                                                       
      GO TO 10                                                          
  300 MJUMP=1                                                           
      NR=L-1                                                            
      RETURN                                                            
  350 MJUMP=0                                                           
      NR=L-1                                                            
      RETURN                                                            
      END                                                               
      SUBROUTINE OUTPUT(TEST,KNO,IW,INS,IEW,DLY,FMGC,XMGC,              
     &  KLAS,PRR,CALR,ICAL,FLT,QSPA,MSTA,                               
     &  PRMK,JMIN,P,S,SRMK,AMX,PRX,CALX,RMK,DT,FMP,AZRES,QRMK,KDX,LDX,  
     &  WT,TP,T,WRK,KSMP,TS,TIME1,TIME2,DELTA,DX,DY,AVXM,               
     &  XMAG,AVFM,FMAG,MAG,FNO,X,B,Y,SE,AF,AZ,AIN,ANIN,TEMP,KEY)        
         include 'H_param.f'
C------- OUTPUT HYPOCENTER ---------------------------------------------
      CHARACTER*1 RMKO,RMK2,RMK3,RMK4,Q,QS,QD,SYM3                      
      CHARACTER*1 CLASS(4),SYMBOL(5),QRMK(N_d),IW(N_s),INS(N_s),IEW(N_s)
      CHARACTER*1 insc,iewc
      CHARACTER*3 RMK(N_d)                                              
      CHARACTER*4 ISW,XMAGOU,FMAGOU,SWTOUT,FMPOUT,RMK5,IPRO             
      CHARACTER*4 MSTA(N_d),PRMK(N_d),WRK(N_d),AZRES(N_d),SRMK(N_d)     
      CHARACTER*5 ERHOUT,SE3OUT                                         
      CHARACTER*6 MAGOUT,SKOUT,TSKOUT,SRESOU,DTKOUT,X4KOUT,TSKCAL              
      character*6 stmp
      CHARACTER*13 Coord_Cartes
      CHARACTER*48 AHEAD                                                
      CHARACTER*80 SUCARD                                               
      INTEGER*4 JMIN(N_d),KDX(N_d),LDX(N_d),KEY(N_d)                    
      INTEGER*4 KSMP(N_s),ICAL(N_s),KLAS(N_s)                           
      REAL LAT2,LON2,LATEP,LONEP,MAG,LATR,LONR                          
      REAL*4 AF(3),B(4),Y(4),SE(4),TEST(20),X(4,N_d),QSPA(9,40)         
      REAL*4 FLT(2,N_s),DLY(2,N_s)                                      
      REAL*4 AMX(N_d),PRX(N_d),CALX(N_d),CAL(N_d),XMAG(N_d)             
      REAL*4 FMP(N_d),FMAG(N_d),TEMP(N_d),DEMP(N_d)                     
      REAL*4 DELTA(N_d),DX(N_d),DY(N_d),ANIN(N_d),AIN(N_d),AZ(N_d)      
      REAL*4 WT(N_d),T(N_d),P(N_d),TP(N_d),DT(N_d),S(N_d),TS(N_d)       
      REAL*4 FMGC(N_s),XMGC(N_s),PRR(N_s),CALR(N_s)                     
      REAL*8 TIME1,TIME2                                                
         real*8 xut8,yut8
      COMMON/C1/IQ,KMS,KFM,IPUN,IMAG,IR,IPRN,KPAPER,KTEST,KAZ,KSORT,KSEL
      COMMON/C2/ ZTR,XNEAR,XFAR,POS,LATR,LONR,ONF,FLIM                  
      COMMON/C3/ AHEAD,IPRO,ISW                                         
      COMMON/C4/ NL,NS,KDATE,KHR,NEAR,IEXIT,IDXS                        
      COMMON/C5/ PMIN,XFN                                               
      COMMON/O1/ NI,INST,KNST,IPH,JPH,NDEC,JMAX,JAV,NR,NRP,KF,KP,KZ,KKF 
      COMMON/O2/ AVRPS,DMIN,RMSSQ,ADJSQ,LATEP,LONEP,Z,ZSQ,AVR,AAR,ORG   
      COMMON/O3/ SUCARD,insc,iewc                                       
      common/stdio/lui,luo,lua1,lua2
      DATA CLASS/'A','B','C','D'/                                       
      DATA SYMBOL/' ','1','2','Q','*'/                                  
c these next 2 vars are undefined in the code, added in EW7.5
      CHARACTER*1 Auto_Manuel
      CHARACTER*6 Fichier_Origine
         data is2/0/
C-----------------------------------------------------------------------
      IF ((IPRN.GE.2) .OR. (KP.EQ.1)) CALL XFMAGS                       
     & (TEST,FMGC,XMGC,KLAS,PRR,CALR,ICAL,IMAG,IR,QSPA,                 
     &  AMX,PRX,CALX,FMP,KDX,DELTA,ZSQ,NRP,CAL,NM,AVXM,SDXM,XMAG,NF,    
     &  AVFM,SDFM,FMAG,MAG)                                             
      LAT1=LATEP/60.                                                    
      LAT2=LATEP-60.*LAT1                                               
      LON1=LONEP/60.                                                    
      LON2=LONEP-60.*LON1                                               
      ADJ=SQRT(ADJSQ)                                                   
      RMS=SQRT(RMSSQ)                                                   
      JHR=KHR                                                           
      OSAVE = ORG                                                       
      IF (ORG .GE. 0.) GO TO 5                                          
      ORG=ORG+3600.                                                     
      KHR=KHR-1                                                         
    5 KMIN=ORG/60.0                                                     
      SEC=ORG-60.0*KMIN                                                 
      ERH=SQRT(SE(1)**2+SE(2)**2)                                       
      NO=FNO                                                            
      RMK2=' '                                                          
      RMKO=' '                                                          
C---- KZ=1 FOR FIXED DEPTH; ONF=0 FOR ORIGIN TIME BASED ON SMP'S        
      IF (ONF .EQ. 0.) RMKO='*'                                         
      IF (KZ .EQ. 1) RMK2='*'                                           
      JMAX=0                                                            
      DO 10 I=1,NRP                                                     
      DXI=DX(I)                                                         
      DYI=DY(I)                                                         
      IF ((DXI.EQ.0.).AND.(DYI.EQ.0.)) GO TO 6                          
      JI=KDX(I)                                                         
      IF (INS(JI) .EQ. 'S') DYI=-DYI                                    
      IF (IEW(JI) .EQ. 'W') DXI=-DXI                                    
c     AZ(I)=AMOD(ATAN2(DXI,DYI)*57.29578 + 360., 360.)                  
c     IF (IEW(JI) .EQ. 'E') DXI=-DXI
      IF (IEW(1).EQ.'E') THEN 
      IF (IEW(JI) .EQ. 'W') THEN 
      AZ(I)=AMOD(ATAN2(DXI,DYI)*57.29578 - 360., 360.)
      AZ(I)=-AZ(I)
      ELSE 
      AZ(I)=AMOD(ATAN2(DXI,DYI)*57.29578 + 360., 360.)
      GOTO 7
      ENDIF
      ENDIF
      AZ(I)=AMOD(ATAN2(DXI,DYI)*57.29578, 360.)
      IF (IEW(JI) .EQ. 'W') THEN 
      IF (AZ(I).LT.0) THEN 
      AZ(I)=-AZ(I)
      ELSE 
      AZ(I)=AMOD(ATAN2(DXI,DYI)*57.29578 - 360., 360.)
      AZ(I)=-AZ(I)
      ENDIF
      ENDIF
      GO TO 7                                                           
    6 AZ(I)= 999.                                                       
    7 CONTINUE                                                          
      AIN(I)=ASIN(ANIN(I))*57.29578                                     
      IF (AIN(I) .LT. 0.) AIN(I)=180.+AIN(I)                            
      AIN(I)=180.-AIN(I)                                                
      SWT=0.                                                            
      IF (LDX(I) .EQ. 0.) GO TO 8                                       
      KK=LDX(I)                                                         
      SWT=WT(KK)                                                        
    8 IF ((WT(I).EQ.0.).AND.(SWT.EQ.0.)) GO TO 10                       
      JMAX=JMAX+1                                                       
      TEMP(JMAX)=AZ(I)                                                  
   10 CONTINUE                                                          
      CALL SORT(TEMP,KEY,JMAX)                                          
      GAP=TEMP(1)+360.-TEMP(JMAX)                                       
      DO 20 I=2,JMAX                                                    
      DTEMP=TEMP(I)-TEMP(I-1)                                           
      IF (DTEMP .GT. GAP) GAP=DTEMP                                     
   20 CONTINUE                                                          
      IGAP=GAP+0.5                                                      
      DO 25 I=1,NRP                                                     
   25 DEMP(I)=DELTA(I)                                                  
      CALL SORT(DEMP,KEY,NRP)                                           

      DO 27 I=1,NRP                                                     
      K=KEY(I)                                                          
      SWT=0.                                                            
      IF (LDX(K) .EQ. 0.) GO TO 26                                      
      KK=LDX(K)                                                         
      SWT=WT(KK)                                                        
   26 IF ((WT(K).GT.0.).OR.(SWT.GT.0.)) GO TO 28                        
   27 CONTINUE                                                          
   28 DMIN=DEMP(I)                                                      
      IDMIN=DMIN+0.5                                                    
      OFD=Z                                                             
      TFD=2.*Z                                                          
      IF (OFD .LT. 5.) OFD=5.                                           
      IF (TFD .LT. 10.) TFD=10.                                         
      JS=4                                                              
      IF ((RMS.LT.0.50).AND.(ERH.LE.5.0)) JS=3                          
      IF ((RMS.LT.0.30).AND.(ERH.LE.2.5).AND.(SE(3).LE.5.0)) JS=2       
      IF ((RMS.LT.0.15).AND.(ERH.LE.1.0).AND.(SE(3).LE.2.0)) JS=1       
      JD=4                                                              
      IF (NO .LT. 6) GO TO 30                                           
      IF ((GAP.LE.180.).AND.(DMIN.LE.50.)) JD=3                         
      IF ((GAP.LE.135.).AND.(DMIN.LE.TFD)) JD=2                         
      IF ((GAP.LE. 90.).AND.(DMIN.LE.OFD)) JD=1                         
   30 JAV=(JS+JD+1)/2                                                   
      Q=CLASS(JAV)                                                      
      QS=CLASS(JS)                                                      
      QD=CLASS(JD)                                                      
      TIME2=SEC+1.D+02*KMIN+1.D+04*KHR+1.D+06*KDATE                     
      IF(IPRN .EQ. 0) GO TO 52                                          
      IF(NI .NE. 1) GO TO 60                                            
      IF(NDEC .GE. 1) GO TO 60                                          
      IF (JPH .EQ. 1) GO TO 60                                          
   52 KKYR=KDATE/10000                                                  
      KKMO=(KDATE-KKYR*10000)/100                                       
      KKDAY=(KDATE-KKYR*10000-KKMO*100)                                 
      JPH=1                                                             
      IF(KSEL) 501,501,505                                              
  501 WRITE(8,502)                                                      
  502 FORMAT(///)                                                       
      GO TO 535                                                         
  505 WRITE(8,506)                                                      
  506 FORMAT(1H1)                                                       
      WRITE(8,53) AHEAD,KKYR,KKMO,KKDAY,KHR,KMIN                        
   53 FORMAT(/,30X,A48,T112,I2,'/',I2,'/',I2,4X,I2,':',I2)              
  535 IF( TIME2 - TIME1 .GT. -20.)GO TO 60                              
      WRITE(8,54)                                                       
   54 FORMAT(' ***** FOLLOWING EVENT IS OUT OF ORDER *****')            
   60 IF ((KP.EQ.1) .AND. (IPRN.EQ.0)) GO TO 67                         
      IF (IPH .EQ. 1) GO TO 62                                          
      WRITE(8,61) INS(1),IEW(1)
   61 FORMAT(/,59X,'  ADJUSTMENTS (KM)  PARTIAL F-VALUES  STANDARD '
     +,'ERRORS  ADJUSTMENTS TAKEN',/
     +,'  I  ORIG  LAT ',A1                      
     2,'    LONG ',A1,'   DEPTH  DM  RMS AVRPS SKD   CF   '
     3,'DLAT  DLON    DZ  DLAT  DLON    DZ  DLAT  DLON    DZ  '
     4,'DLAT  DLON    DZ')                                                               
      IF (IPRN .EQ. 1) IPH=1                                            
   62 WRITE(8,63) NI,SEC,LAT1,LAT2,LON1,LON2,Z,RMK2,IDMIN,RMS,AVRPS,    
     1 QS,KF,QD,FLIM,B(2),B(1),B(3),AF(2),AF(1),AF(3),SE(2),SE(1),      
     2 SE(3),Y(2),Y(1),Y(3)                                             
   63 FORMAT(I3,F6.2,I3,'-',F5.2,I4,'-',F6.2,F6.2,A1,I3,F5.2,F6.2,      
     1 1X,A1,I1,A1,13F6.2)                                              
      IF (KP .EQ. 0) GO TO 100                                          
   67 JNST=KNST*10+INST                                                 
      IF (NM .EQ. 0) AVXM=0.                                            
      IF (NF .EQ. 0) AVFM=0.                                            
      MAGOUT = '      '                                                 
      IF (MAG .NE. 99.9) WRITE(MAGOUT,68) MAG                           
   68 FORMAT(F6.2)                                                      
      SE3OUT = '     '                                                  
      IF (SE(3) .NE. 0.) WRITE(SE3OUT,70) SE(3)                         
   70 FORMAT(F5.1)                                                      
      ERHOUT = '     '                                                  
      IF (ERH .NE. 0.) WRITE(ERHOUT,70) ERH                             
        insc='N'
        iewc='E'
        lat1=abs(lat1)
        lat2=abs(lat2)
        lon1=abs(lon1)
        lon2=abs(lon2)
        if (latep.ge.0) goto 73 
        insc='S'
73      if (lonep.ge.0) goto 74 
        iewc='W'
74    WRITE(8,75) insc,iewc                                             
   75 FORMAT(//,'  DATE    ORIGIN    LAT ',A1,'    LONG ',A1,'    DEPTH'
     1,'    MAG NO DM GAP M  RMS  ERH  ERZ Q SQD  ADJ IN NR  AVR  AAR'
     2,' NM AVXM SDXM NF AVFM SDFM I')
      WRITE(8,86) KDATE,RMKO,KHR,KMIN,SEC,LAT1,LAT2,LON1,LON2           
     1,Z,RMK2,MAGOUT,NO,IDMIN,IGAP,KNO,RMS,ERHOUT,SE3OUT,Q,QS           
     2,QD,ADJ,JNST,NR,AVR,AAR,NM,AVXM,SDXM,NF,AVFM,SDFM,NI              
   86 FORMAT(1X,I6,A1,2I2,F6.2,I3,'-',F5.2,I4,'-',F5.2,1X,F6.2,A1,A6    
     1,2I3,I4,I2,F5.2,2A5,2(1X,A1),'|',A1,F5.2,2I3,2F5.2,2(I3,2F5.1),I2)
      IF ((QRMK(1).NE.SYMBOL(4)).AND.(QRMK(1).NE.SYMBOL(5)))            
     1QRMK(1)=SYMBOL(1)                                                 
      SYM3=SYMBOL(KNO+1)                                                
      IF (IPUN .EQ. 0) GO TO 100                                        
      if(lua2.ne.0) 
     1 WRITE(lua2,87) KDATE,KHR,KMIN,SEC,LAT1,LAT2,iabs(LON1),
     2 abs(LON2),Z,RMK2,MAGOUT,NO,IGAP,DMIN,RMS,ERHOUT,
     3 SE3OUT,QRMK(1),Q,SYM3                                                           
      WRITE(7,87) KDATE,KHR,KMIN,SEC,LAT1,LAT2,iabs(LON1),abs(LON2)                
     1,Z,RMK2,MAGOUT,NO,IGAP,DMIN,RMS,ERHOUT,SE3OUT,QRMK(1)             
     2,Q,SYM3                                                           
   87 FORMAT(I6,1X,2I2,F6.2,I3,'-',F5.2,I4,'-',F5.2,1X,F6.2,A1,A6,I3    
     1,I4,F5.1,F5.2,2A5,3A1)                                            
  100 CONTINUE                                                          
      WRITE(SUCARD,87) KDATE,KHR,KMIN,SEC,LAT1,LAT2,LON1,LON2           
     1,Z,RMK2,MAGOUT,NO,IGAP,DMIN,RMS,ERHOUT,SE3OUT,QRMK(1),Q,SYM3      
      IF (KP .EQ. 1) GO TO 105                                          
      IF(IPRN .LE. 1) GO TO 300                                         
  105 WRITE(8,110)                                                      
  110 FORMAT(/,'  STN  DIST AZM AIN PRMK HRMN P-SEC TPOBS TPCAL'
     1,' DLY/H1 P-RES P-WT AMX PRX CALX K XMAG RMK FMP FMAG SRMK'
     2,' S-SEC TSOBS S-RES  S-WT    DT')  
c-------------------
C
C     Transformation des coordonnees Geographiques de Degres Minutes
C         decimales en coordonnees cartesiennes :
C             -par defaut en UTM avec pour fuseau le fuseau du premier
C                 passage
C             -suivant les valeur de Test(16) et Test(17)
C                 Test(16)=1 : UTM avec fuseau No Test(17) si non nul
C                 Test(16)=5 : Lambert avec Reference No Test(17)
C                 Test(16)=6 : Gauss-Laborde Reunion
C
      No_Systeme_Final=Test(16)
      No_Ref_Final=Test(17)
      IF(No_Systeme_Final.EQ.6)THEN
          Coord_Cartes='Gauss-Laborde'
          No_Ref_Final=0
      ELSE
          IF(No_Systeme_Final.EQ.5)THEN
              Coord_Cartes='  Lambert .. '
              IF(No_Ref_Final.LT.1..OR.No_Ref_Final.GT.10)No_Ref_Final=1
              WRITE(Coord_Cartes(11:12),'(I2)')No_Ref_Final
          ELSE
              No_Systeme_Final=1
              Coord_Cartes='   U-T-M ..  '
              IF(Test(16).EQ.1..AND.Test(17).GE.20.)THEN
                  No_Ref_Final=Test(17)
              ELSE
                  No_Ref_Final=0
              ENDIF
          ENDIF
      ENDIF
      CALL TRANSF(DBLE(LonEp/60.),DBLE(LatEp/60.),
     *    xut8,yut8,3,No_Systeme_Final,1,No_Ref_Final)
         X_Cartes=xut8
         Y_Cartes=yut8
      IF(No_Systeme_Final.EQ.1)THEN
          WRITE(Coord_Cartes(10:11),'(I2)')No_Ref_Final
      ELSE
          IF(iewc.EQ.'W')LonEp=-LonEp
          IF(insc.EQ.'S')LatEp=-LatEp
      ENDIF
      if(lua1.ne.0)then
c------- Cartes hyp.tri
                  if(se(2).gt.99.) se(2)= 99.99
                  if(se(3).gt.99.) se(3)= 99.99
          WRITE(lua1,'(2i6,2f8.2,6f6.2)',ERR=80)KDATE,
     *        KHR*100+kmin,X_Cartes,Y_Cartes,z,sec,se(2),se(1),
     *        se(3),rms
      endif
c-------------------
Clu8  WRITE(LU_8,'(/" STN  DIST AZM AIN PRMK HRMN P-SEC TPOBS TPCAL ",
Clu8 *    "DLY/H1 P-RES P-WT AMX PRX CALX K XMAG RMK FMP FMAG SMK S-",
Clu8 *    "SEC TSOBS S-RES S-WT    DT")',ERR=80)
C
              Altitude=-Z-Test(14)
c           WRITE(8   ,601,ERR=80)
c     *        KDATE,KHR,kmin,sec,NI,Lat1,
c     *        Lat2,ins(1),Coord_Cartes,X_Cartes,SE(1),
c     *        Lon1, Lon2,iew(1),Coord_Cartes,Y_Cartes,
c     *        SE(2),Z,Altitude,SE(3),RMS,magout,Q
C
601   format  (' Date   Heure Minute Seconde'/3I6,F9.2,T60,'Nb',
     *    ' Iterations :',I3/'Latitude',9X,':',I3,F6.2,1X,A1,T38,
     *    'X ',A13,' =',F8.2,' +/-',F8.2,' Km'/'Longitude',8X,':',I3,
     *     F6.2,1X,A1,T38,'Y ',A13,' =',F8.2,' +/-',F8.2,' Km'/
     *     'Profondeur / Ref :',F9.2,' Km',T38,'Altitude / mer ',T54,
     *     '=',F8.2,' +/-',F8.2,' Km'/'RMS',14X,':',F9.2,' s',
     *                  T38,'Magnitude',t54,'=  ',a6,
     *     T66,'Qualite :  ',A1//'Sta   Dist  Az Inc P   HhMn   Sec',
     *     '  Calc   O-C P-WT S    Sec   Calc   O-C  S-WT')
c Sta  Dist  Az Inc  P   HhMn   Sec  Calc  O-C   dly   WT 
cCAG    0.6  27 161EPU1  943 15.64  0.98  0.00 1.93                           
cTAG    0.9 198 149IPD0  943 15.60  0.94  0.01 1.29 S 4 15.90  1.25 -0.44  0.0


           WRITE(*   ,601,ERR=80)
     *        KDATE,KHR,kmin,sec,NI,Lat1,
     *        Lat2,insc,Coord_Cartes,X_Cartes,SE(1),
     *        Lon1, Lon2,iewc,Coord_Cartes,Y_Cartes,
     *        SE(2),Z,Altitude,SE(3),RMS,magout,Q
C
      nobsp = 0
      nobss = 0
      DO No1=1,NRP
          No2=No1
          IF(KSORT.EQ.1)No2=KEY(No1)
          KJI=KDX(No2)
          TPK=TP(No2)-ORG
          IF(TPK.LT.0.)TPK=TPK+3600.
          WRITE(X4KOUT,'(F6.2)')X(4,No2)
          IF((AZRES(No2).EQ.' .  ').OR.(AZRES(No2).EQ.'    ').OR.
     *        (AZRES(No2).EQ.'0.  '))X4KOUT = '      '
          RMK3=' '
          IF(XMAG(No2).NE.99.9)THEN
              IF(ABS(XMAG(No2)-AVXM).GE.0.5)RMK3='*'
          ENDIF
          RMK4=' '
          IF(FMAG(No2).NE.99.9)THEN
              IF(ABS(FMAG(No2)-AVFM).GE.0.5)RMK4='*'
          ENDIF
          XMAGOU='    '
          IF(XMAG(No2).NE.99.9)WRITE(XMAGOU,'(F4.1)')XMAG(No2)
          FMAGOU='    '
          IF(FMAG(No2).NE.99.9)WRITE(FMAGOU,'(F4.1)')FMAG(No2)
          IAZ=AZ(No2)+0.5
          IAIN=AIN(No2)+0.5
          IAMX=AMX(No2)
          IPRX=100.*PRX(No2)+0.5
          FMPOUT='    '
          IFMPK=FMP(No2)
          IF(FMP(No2).NE.0.)WRITE(FMPOUT,'(I4)')IFMPK
c------------------------- on somme les temps avec poids > 0.1
          if(wt(No2).gt.0.1)nobsp=nobsp+1
c-------------------------
          IF(LDX(No2).EQ.0)THEN
C-----CHECK FOR SMP DATA
              IF(KSMP(No2).EQ.0)THEN
                  SKOUT = '      '
                  TSKOUT = '      '
                    TSKCAL='      '         
                  SRESOU='      '
                  RMK5='    '
                  SWTOUT='    '
              ELSE
                  WRITE(SRESOU,'(F6.2)')X(4,No2)
                  RMK5='    '
                  SWTOUT='****'
                  TSK=S(No2)-P(No2)
                  WRITE(TSKOUT,'(F6.2)')TSK
              ENDIF
          ELSE
              No3=LDX(No2)
              RMK5=WRK(No3)
              SWT=WT(No3)
c------------------------ on somme les temps S avec poids > 0.1
              if(swt.gt.0.1)nobss=nobss+1
c------------------------
              TSK=TS(No2)-ORG
              WRITE(SRESOU,'(F6.2)')X(4,No3)
              WRITE(SWTOUT,'(F4.1)')SWT
              WRITE(TSKOUT,'(F6.2)')TSK
              WRITE(TSKCAL,'(F6.2)')T(no3)
          ENDIF
          DLYK=DLY(KNO,KJI)
Cisw      IF(ISW.EQ.'1   ')DLYK=FLT(KNO,KJI)
          DTKOUT='      '
          IF(DT(No2).NE.0.)WRITE(DTKOUT,'(F6.2)')DT(No2)
          IF(S(No2).NE.999.99)WRITE(SKOUT,'(F6.2)')S(No2)

c      WRITE(8,176) MSTA(K),DELTA(K),IAZ,IAIN,PRMK(K),JHR,JMIN(K),P(K)   
c     1,      TPK,T(K),DLYK,X4KOUT,WRK(K),WT(K),IAMX,IPRX,CAL(K)         
c     2,KLAS(KJI),XMAGOU,RMK3,RMK(K),FMPOUT,FMAGOU,RMK4,SRMK(K),SKOUT    
c     3,TSKOUT,SRESOU,RMK5,SWTOUT,DTKOUT,IW(KJI)                         
c  176 FORMAT(A4,F6.1,2I4,1X,A4,1X,2I2,4F6.2,A6,A2,F4.2,I4,I3,
cF6.2,I2 , A4,A1,1X,A3,A4,A4,A1,A4,3A6,A2,A4,A6,A6,A1)                   
        WRITE(8,'(1X,A4,F6.1,2I4,1X,A4,1X,2I2,4F6.2,A6,A2,F4.2,I4,I3
     1,F6.2,I2,A4,A1,1X,A3,A4,A4,A1,1X,A4,3A6,A2,A4,A6,A6,A1)') 
     2 MSTA(No2),DELTA(No2),IAZ,IAIN,PRMK(No2),JHR,JMIN(No2),P(No2)   
     3,      TPK,T(No2),DLYK,X4KOUT,WRK(No2),WT(No2),IAMX,IPRX,CAL(No2)         
     4,KLAS(KJI),XMAGOU,RMK3,RMK(No2),FMPOUT,FMAGOU,RMK4,SRMK(No2),SKOUT    
     5,TSKOUT,SRESOU,RMK5,SWTOUT,DTKOUT,IW(KJI)                         

c         write(8,*)'---------------'
c         write(8,*)p(no2),tpk,t(no2),dlyk,x4kout,wrk(no2),wt(no2)
c         write(8,*)srmk(no2),skout,tskout,sresou,swtout,rmk5
c         write(8,*)t(no3),tskcal,ts(no2),s(no2)
c         write(8,*)'---------------'
c         WRITE(8,'(A4,F6.1,2I4,1X,A4,1X,2I2,3F6.2,A6,f6.2,A2,F4.2,I4,I3,
c     *        F6.2,I2,A4,/,5x,A1,1X,A3,A4,A4,A1,
c     +        A4,5x,3A6,a6,A6,4x,A6,A6,A6,A1)',ERR=80)
c     +                  MSTA(No2),DELTA(No2),IAZ,IAIN,
c     +                  PRMK(No2),JHR,JMIN(No2),P(No2),TPK,T(No2),
c     +                  X4KOUT,dlyk,WRK(No2),WT(No2),
c     +                  IAMX,IPRX,CAL(No2),KLAS(KJI),
c     +                  XMAGOU,RMK3,RMK(No2),FMPOUT,FMAGOU,RMK4,
c     +                  SRMK(No2),SKOUT,TSKOUT,tskcal,SRESOU,SWTOUT,RMK5,
c     +          DTKOUT,IW(KJI)
               WRITE(*,'(A4,F6.1,2I4,A4,I3,I2,2F6.2,A6,F5.2,A4,3A6,1X,
     *            A4)',ERR=80)MSTA(No2),delta(No2),iaz,iain,
     *            prmk(No2),jhr,JMIN(No2),p(No2),t(No2),x4kout,
     *            wt(No2),srmk(No2),skout,tskout,sresou,swtout
c             IF(WRK(No2).NE.'    '.OR.RMK5.NE.'    ')WRITE(LU_n,'(46X,
c    *            A4,23X,A4)',ERR=80)WRK(No2),RMK5
           stmp=srmk(No2)
          IF(lua1.NE.0)THEN
c100   FORMAT(A4,1X,A1,a1,I1,1X,I6,2I2,F5.2,7X,F5.2,1X,A1,1X,I1,
c     *2F6.2,T66,F6.2)
       WRITE(Lua1,'(a4,a4,1x,i6,2i2,f5.2,6x,a6,a4,2a6,f7.3,t66,f6.2)',
     +   ERR=80)
     *            MSTA(No2),prmk(No2),KDATE,jhr,
     *            JMIN(No2),p(No2),skout,stmp(:4),x4kout,sresou,
     +         delta(no2),dt(no2)
          ENDIF
          IF(IPUN.EQ.2)THEN
              ISEC=100.*SEC
              WRITE(7   ,'(A4,3F6.1,1X,A4,F6.2,A6,F5.1,A6,1X,A3,A6,I7,
     *            2I2,2I4,A1)',ERR=80)MSTA(No2),DELTA(No2),
     *            AZ(No2),AIN(No2),PRMK(No2),TPK,X4KOUT,WT(No2),XMAGOU,
     *            RMK(No2),FMAGOU,KDATE,KHR,KMIN,ISEC,KJI,
     *            SYM3
          ENDIF
      ENDDO
      IF(lua1.ne.0)WRITE(lua1,'(1X)',ERR=80)
c*********************************************************************
c Cartes analogue a hypo71 de l`ipg sur unite 9
      numer=0
c the following 2 variables were not defined!
      Auto_Manuel = ' '
      Fichier_Origine = '      '
c      WRITE(8   ,'(i6,2i3,f5.2,i3,f6.2,i4,2f6.2,f5.1,f6.2,f5.2,2f5.1,5x,
c     *    i4,a1,2i2,A1,3F6.2,A6)',ERR=80)KDATE,KHR,kmin,sec,
c     *    Lat1,Lat2,Lon1,Lon2,z,se(3),mag,rms,
c     *    se(1),se(2),numer,q,nobsp,nobss,Auto_Manuel,X_Cartes,Y_Cartes,
c     *    Altitude,Fichier_Origine
c*********************************************************************
      IF(IPUN.EQ.2)THEN
           WRITE(7  ,*,ERR=80) ' $$$'
      ENDIF
  300 KHR=JHR
      ORG=OSAVE
C
      RETURN
C
   80 continue
         write(*,*) 'Write error'
         return
      END
