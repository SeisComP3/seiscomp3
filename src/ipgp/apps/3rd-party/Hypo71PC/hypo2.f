      SUBROUTINE ANSWER(A,S,XMEAN,SIGMA,IDX,PHI,L,M,MM,PF,NDX,ADX)      
C------- PRINT INTERMEDIATE RESULTS OF REGRESSION ANALYSIS (SWMREG) --- 
      CHARACTER*8 ADX                                                   
      INTEGER*4 IDX(4)                                                  
      REAL*4 A(7,7),S(4,4),XMEAN(4),SIGMA(4),B(4),BSE(4),PF(3)          
C-----------------------------------------------------------------------
      DO 410 I=1,MM                                                     
      WRITE(8,400) (A(I,J),J=1,MM)                                      
  400 FORMAT(7E18.8)                                                    
  410 CONTINUE                                                          
      FVE=1.-A(M,M)                                                     
      B0=XMEAN(M)                                                       
      YSE=77.7                                                          
      IF (PHI .GE. 1) YSE=SIGMA(M)*SQRT(ABS(A(M,M)/PHI))                
      DO  5 I=1,L                                                       
      IF (IDX(I).EQ.0) GO TO  5                                         
      B(I)=A(I,M)* SQRT(ABS(S(M,M)/S(I,I)))                             
      BSE(I)=YSE* SQRT(ABS(A(I+M,I+M)/S(I,I)))                          
      B0=B0-B(I)*XMEAN(I)                                               
    5 CONTINUE                                                          
      WRITE(8,10) ADX,NDX,FVE,YSE,B0                                    
   10 FORMAT(/,' VARIABLE ', A8, '................',I5                  
     2,      /,' FRACTION OF VARIATION EXPLAINED..',E18.8               
     3,      /,' STANDARD ERROR OF Y..............',E18.8               
     4,      /,' CONSTANT IN REGRESSION EQUATION..',E18.8)              
      WRITE(8,20)                                                       
   20 FORMAT(/,' VARIABLE     COEFFICIENT      STANDARD ERROR'          
     1,'     PARTIAL F-VALUE')                                          
      DO 40 I=1,L                                                       
      IF (IDX(I).EQ.0) GO TO 40                                         
      WRITE(8,30) I,B(I),BSE(I),PF(I)                                   
   30 FORMAT(I5,3E20.6)                                                 
   40 CONTINUE                                                          
      RETURN                                                            
      END                                                               
      SUBROUTINE AZWTOS(DX,DY,NR,WT,KDX,AZ,TEMP,KEY,INS,IEW)            
C------- AZIMUTHAL WEIGHTING OF STATIONS BY QUADRANTS ------------------
	include 'H_param.f'
      CHARACTER*1 INS(N_s),IEW(N_s)                                     
      INTEGER*4 KTX(4),KEMP(N_d),KEY(N_d),KDX(N_d)                      
      REAL*4 TX(4),TXN(4),DX(N_d),DY(N_d),WT(N_d),AZ(N_d),TEMP(N_d)     
C-----------------------------------------------------------------------
      J=0                                                               
      DO 10 I=1,NR                                                      
      IF (WT(I) .EQ. 0.) GO TO 10                                       
      DXI=DX(I)                                                         
      DYI=DY(I)                                                         
      IF ((DXI.EQ.0.).AND.(DYI.EQ.0.)) GO TO 6                          
      JI=KDX(I)                                                         
      IF (INS(JI) .EQ. 'S') DYI=-DYI                                    
      IF (IEW(JI) .EQ. 'W') DXI=-DXI                                    
      AZ(I)=AMOD(ATAN2(DXI,DYI)*57.29578 + 360., 360.)                  
      GO TO 7                                                           
    6 AZ(I)=999.                                                        
    7 J=J+1                                                             
      TEMP(J)=AZ(I)                                                     
   10 CONTINUE                                                          
      CALL SORT(TEMP,KEY,J)                                             
      GAP=TEMP(1)+360.-TEMP(J)                                          
      IG=1                                                              
      DO 20 I=2,J                                                       
      DTEMP=TEMP(I)-TEMP(I-1)                                           
      IF (DTEMP .LE. GAP) GO TO 20                                      
      GAP=DTEMP                                                         
      IG=I                                                              
   20 CONTINUE                                                          
      TX(1)=TEMP(IG)-0.5*GAP                                            
      TX(2)=TX(1)+90.                                                   
      TX(3)=TX(1)+180.                                                  
      TX(4)=TX(1)+270.                                                  
      DO 124 I=1,4                                                      
      TXN(I)=0.                                                         
      IF (TX(I) .LT. 0.) TX(I)=TX(I)+360.                               
      IF (TX(I).GT.360.) TX(I)=TX(I)-360.                               
  124 CONTINUE                                                          
      CALL SORT(TX,KTX,4)                                               
      DO 130 I=1,NR                                                     
      IF (WT(I) .EQ. 0.) GO TO 130                                      
      IF (AZ(I) .GT. TX(1)) GO TO 126                                   
  125 TXN(1)=TXN(1)+1.                                                  
      KEMP(I)=1                                                         
      GO TO 130                                                         
  126 IF (AZ(I) .GT. TX(2)) GO TO 127                                   
      TXN(2)=TXN(2)+1.                                                  
      KEMP(I)=2                                                         
      GO TO 130                                                         
  127 IF (AZ(I) .GT. TX(3)) GO TO 128                                   
      TXN(3)=TXN(3)+1.                                                  
      KEMP(I)=3                                                         
      GO TO 130                                                         
  128 IF (AZ(I) .GT. TX(4)) GO TO 125                                   
      TXN(4)=TXN(4)+1.                                                  
      KEMP(I)=4                                                         
  130 CONTINUE                                                          
      XN=4                                                              
      IF (TXN(1).EQ.0.) XN=XN-1                                         
      IF (TXN(2).EQ.0.) XN=XN-1                                         
      IF (TXN(3).EQ.0.) XN=XN-1                                         
      IF (TXN(4).EQ.0.) XN=XN-1                                         
      FJ=J/XN                                                           
      DO 150 I=1,NR                                                     
      IF (WT(I) .EQ. 0.) GO TO 150                                      
      KI=KEMP(I)                                                        
      WT(I)=WT(I)*FJ/TXN(KI)                                            
  150 CONTINUE                                                          
      RETURN                                                            
      END                                                               
