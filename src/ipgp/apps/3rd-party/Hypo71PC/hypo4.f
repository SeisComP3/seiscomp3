      SUBROUTINE XFMAGS(TEST,FMGC,XMGC,KLAS,PRR,CALR,ICAL,IMAG,IR,QSPA, 
     &  AMX,PRX,CALX,FMP,KDX,DELTA,ZSQ,NRP,CAL,NM,AVXM,SDXM,XMAG,NF,    
     &  AVFM,SDFM,FMAG,MAG)                                             
C------- COMPUTE X-MAGNITUDE AND F-MAGNITUDE ---------------------------
	include 'H_param.f'
      INTEGER*4 KDX(N_dat),ICAL(N_sta),KLAS(N_sta)                            
      REAL*4 MAG,TEST(20),QSPA(9,40),RSPA(8,20)                         
      REAL*4 TEMP1(8,5),TEMP2(8,5),TEMP3(8,5),TEMP4(8,5)                
      REAL*4 DELTA(N_dat),CAL(N_dat),XMAG(N_dat),FMAG(N_dat)
      REAL*4 AMX(N_dat),PRX(N_dat),CALX(N_dat),FMP(N_dat)
      REAL*4 FMGC(N_sta),XMGC(N_sta),PRR(N_sta),CALR(N_sta) 
      DATA ZMC1,ZMC2,PWC1,PWC2/0.15,3.38,0.80,1.50/                     
      DATA TEMP1/-0.02, 1.05,-0.15,-0.13, 0.66, 0.55, 0.17, 0.42,       
     2           0.14, 1.18,-0.01, 0.01, 0.79, 0.66, 0.27, 0.64,        
     3           0.30, 1.29, 0.12, 0.14, 0.90, 0.76, 0.35, 0.84,        
     4           0.43, 1.40, 0.25, 0.27, 1.00, 0.86, 0.43, 0.95,        
     5           0.55, 1.49, 0.38, 0.41, 1.08, 0.93, 0.49, 1.04/        
      DATA TEMP2/0.65, 1.57, 0.53, 0.57, 1.16, 1.00, 0.55, 1.13,        
     7           0.74, 1.63, 0.71, 0.75, 1.23, 1.07, 0.63, 1.24,        
     8           0.83, 1.70, 0.90, 0.95, 1.30, 1.15, 0.72, 1.40,        
     9           0.92, 1.77, 1.07, 1.14, 1.38, 1.25, 0.83, 1.50,        
     A           1.01, 1.86, 1.23, 1.28, 1.47, 1.35, 0.95, 1.62/        
      DATA TEMP3/1.11, 1.96, 1.35, 1.40, 1.57, 1.46, 1.08, 1.73,        
     C           1.20, 2.05, 1.45, 1.49, 1.67, 1.56, 1.19, 1.84,        
     D           1.30, 2.14, 1.55, 1.58, 1.77, 1.66, 1.30, 1.94,        
     E           1.39, 2.24, 1.65, 1.67, 1.86, 1.76, 1.40, 2.04,        
     F           1.47, 2.33, 1.74, 1.76, 1.95, 1.85, 1.50, 2.14/        
      DATA TEMP4/1.53, 2.41, 1.81, 1.83, 2.03, 1.93, 1.58, 2.24,        
     H           1.56, 2.45, 1.85, 1.87, 2.07, 1.97, 1.62, 2.31,        
     I           1.53, 2.44, 1.84, 1.86, 2.06, 1.96, 1.61, 2.31,        
     J           1.43, 2.36, 1.76, 1.78, 1.98, 1.88, 1.53, 1.92,        
     K           1.25, 2.18, 1.59, 1.61, 1.82, 1.72, 1.37, 1.49/        
C-----------------------------------------------------------------------
      DO  I=1,8                                                        
         DO  J=1,5                                                     
            RSPA(I,J) = TEMP1(I,J)                                      
            RSPA(I,J+5) = TEMP2(I,J)                                    
            RSPA(I,J+10) = TEMP3(I,J)                                   
            RSPA(I,J+15) = TEMP4(I,J)                                   
         ENDDO
      ENDDO
C                                                                       
      NM=0                                                              
      AVXM=0.                                                           
      SDXM=0.                                                           
      NF=0                                                              
      AVFM=0.                                                           
      SDFM=0.                                                           
      DO 40 I=1,NRP                                                     
      XMAG(I)=99.9                                                      
      RAD2=DELTA(I)**2+ZSQ                                              
      JI=KDX(I)                                                         
      K=KLAS(JI)                                                        
      AMXI=ABS(AMX(I))
      IF ((RAD2.LT.1.).OR.(RAD2.GT.360000.)) GO TO 30                   
c      JI=KDX(I)                                                         
c      K=KLAS(JI)                                                        
c      AMXI=ABS(AMX(I))                                                  
      CAL(I)=CALX(I)                                                    
      IF ((CAL(I).LT.0.01).OR.(ICAL(JI).EQ.1)) CAL(I)=CALR(JI)          
      IF ((AMXI.LT.0.01).OR.(CAL(I).LT.0.01)) GO TO 30                  
      IF ((K.LT.0).OR.(K.GT.8)) GO TO 30                                
      XLMR=0.                                                           
      IF (K .EQ. 0) GO TO 20                                            
      PRXI=PRX(I)                                                       
      IF (PRXI .LT. 0.01) PRXI=PRR(JI)                                  
      IF (IR .EQ. 0) GO TO 10                                           
      IF ((PRXI.GT.20.).OR.(PRXI.LT.0.033)) GO TO 30                    
      FQ=10.*ALOG10(1./PRXI)+20.                                        
      IFQ=FQ                                                            
      XLMR=QSPA(K,IFQ)+(FQ-IFQ)*(QSPA(K,IFQ+1)-QSPA(K,IFQ))             
      GO TO 20                                                          
   10 IF ((PRXI.GT.3.).OR.(PRXI.LT.0.05)) GO TO 30                      
      FQ=10.*ALOG10(1./PRXI)+6.                                         
      IFQ=FQ                                                            
      XLMR=RSPA(K,IFQ)+(FQ-IFQ)*(RSPA(K,IFQ+1)-RSPA(K,IFQ))             
   20 BLAC=ALOG10(AMXI/(2.*CAL(I)))-XLMR                                
      RLD2=ALOG10(RAD2)                                                 
      BLNT=ZMC1-PWC1*RLD2                                               
      IF (RAD2 .GE. 40000.) BLNT=ZMC2-PWC2*RLD2                         
      XMAG(I)=BLAC-BLNT+XMGC(JI)                                        
      NM=NM+1                                                           
      AVXM=AVXM+XMAG(I)                                                 
      SDXM=SDXM+XMAG(I)**2                                              
   30 FMAG(I)=99.9                                                      
      IF (FMP(I) .EQ. 0.) GO TO 40                                      
      FMAG(I)=TEST(7)+TEST(8)*ALOG10(FMP(I))+TEST(9)*DELTA(I)+FMGC(JI)  
      NF=NF+1                                                           
      AVFM=AVFM+FMAG(I)                                                 
      SDFM=SDFM+FMAG(I)**2                                              
   40 CONTINUE                                                          
      IF (NM .EQ. 0) GO TO 50                                           
      AVXM=AVXM/NM                                                      
      SDXM=SQRT(SDXM/NM-AVXM**2+1.e-7)                                  
   50 IF (NF .EQ. 0) GO TO 60                                           
      AVFM=AVFM/NF                                                      
      SDFM=SQRT(SDFM/NF-AVFM**2+1.e-7)                                  
   60 IF (NM .EQ. 0) AVXM=99.9                                          
      IF (NF .EQ. 0) AVFM=99.9                                          
      IF (IMAG-1) 70,80,90                                              
   70 MAG=AVXM                                                          
      RETURN                                                            
   80 MAG=AVFM                                                          
      RETURN                                                            
   90 MAG=0.5*(AVXM+AVFM)                                               
      IF (AVXM .EQ. 99.9) GO TO 80                                      
      IF (AVFM .EQ. 99.9) GO TO 70                                      
      RETURN                                                            
      END                                                               
