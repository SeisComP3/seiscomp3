      SUBROUTINE SINGLE(TEST,KNO,IW,NSTA,INS,IEW,DLY,FMGC,
     &  XMGC,KLAS,PRR,CALR,ICAL,LAT,LON,V,D,DEPTH,VSQ,THK,H,G,F,
     &  TID,DID,FLT,QSPA,MSTA,PRMK,W,JMIN,P,S,SRMK,WS,AMX,PRX,CALX,RMK,
     &  DT,FMP,AZRES,SYM,QRMK,KDX,LDX,JDX,TP,WRK,KSMP,TS,TIME1,TIME2,
     &  AVXM,AVFM,XMAG,FMAG,NRES,SR,SRSQ,SRWT,QNO,MNO)
c******** besoin de iheter pour refroidir le calcul de trvhet si iheter=
	include 'H_param.f'
      common/heter2/iheter,xorgh,yorgh,cphih,sphih,xlatc,xlonc
      common/provi/epsob,iflag,elev(N_s)
      common/gradi/nlecp
C-------- SOLUTION FOR A SINGLE EARTHQUAKE ----------------------------
      CHARACTER*1 SYM(N_d),QRMK(N_d)
      CHARACTER*1 IW(N_s),INS(N_s),IEW(N_s)
      CHARACTER*3 RMK(N_d)
      CHARACTER*4 ISW,IPRO,CHECK,NSTA(N_s)
      CHARACTER*4 MSTA(N_d),PRMK(N_d),SRMK(N_d),WRK(N_d),AZRES(N_d)
      CHARACTER*48 AHEAD
      CHARACTER*80 SUCARD
      INTEGER*4 ISKP(4),LA(10),LO(10)
      INTEGER*4 KDX(N_d),KEY(N_d),JMIN(N_d),LDX(N_d)
      INTEGER*4 ICAL(N_s),KLAS(N_s),MNO(N_s),KSMP(N_s),JDX(N_s)
      INTEGER*4 NRES(2,N_s)
      REAL*4 LATRT,LONRT,LATSV,LONSV
      REAL*4 LAT2,LON2,LATEP,LONEP,MAG,LATR,LONR
      REAL*4 QNO(4),XMEAN(4),SUM(5),WF(41),ALZ(10)
      REAL*4 V(21),D(21),VSQ(21),THK(21),H(21),DEPTH(21)
      REAL*4 TID(21,21),DID(21,21),F(21,21),G(4,21)
      REAL*4 SR(2,N_s),SRSQ(2,N_s),SRWT(2,N_s)
      REAL*4 AF(3),B(4),Y(4),SE(4),TEST(15),X(4,N_d),QSPA(9,40)
      REAL*4 FLT(2,N_s),DLY(2,N_s)
      REAL*4 AMX(N_d),PRX(N_d),CALX(N_d),XMAG(N_d)
      REAL*4 FMP(N_d),FMAG(N_d),TEMP(N_d),W(N_d),WS(N_d)
      REAL*4 DELTA(N_d),DX(N_d),DY(N_d),ANIN(N_d),AIN(N_d),AZ(N_d)
      REAL*4 WT(N_d),T(N_d),P(N_d),TP(N_d),DT(N_d),S(N_d),TS(N_d)
      REAL*4 FMGC(N_s),XMGC(N_s),PRR(N_s),CALR(N_s),LAT(N_s),LON(N_s)
      REAL*8 TIME1,TIME2
      COMMON/C1/IQ,KMS,KFM,IPUN,IMAG,IR,IPRN,KPAPER,KTEST,KAZ,KSORT,KSEL
      COMMON/C2/ ZTR,XNEAR,XFAR,POS,LATR,LONR,ONF,FLIM
      COMMON/C3/ AHEAD,IPRO,ISW
      COMMON/C4/ NL,NS,KDATE,KHR,NEAR,IEXIT,IDXS
      COMMON/C5/ PMIN,XFN
      COMMON/O1/ NI,INST,KNST,IPH,JPH,NDEC,JMAX,JAV,NR,NRP,KF,KP,KZ,KKF
      COMMON/O2/ AVRPS,DMIN,RMSSQ,ADJSQ,LATEP,LONEP,Z,ZSQ,AVR,AAR,ORG
      COMMON/O3/ SUCARD
      COMMON/O4/ delta
      DATA WF/.95,0.95,0.95,0.95,0.95,0.95,0.94,0.94,0.94,0.93,
     1       0.92,0.92,0.91,0.90,0.88,0.87,0.85,0.83,0.80,0.77,
     2       0.73,0.69,0.64,0.59,0.53,0.47,0.41,0.34,0.28,0.23,
     3       0.18,0.14,0.11,0.08,0.06,0.04,0.03,0.02,0.01,0.01,0./
      DATA LA/1,1,1,1,0,0,-1,-1,-1,-1/,
     1     LO/+1,-1,+1,-1,0,0,+1,-1,+1,-1/,
     2     ALZ/-1.0,-1.0,+1.0,+1.0,-1.732,+1.732,-1.0,-1.0,+1.0,+1.0/
c************** on modifie iheter pour la premiere passe iflag=0 durant
c     ihetol=iheter
c     iheter=0
C-----------------------------------------------------------------------
      LATRT=0.
      LONRT=0.
      LATSV=0.
      LONSV=0.
  778 AVRPS = 0.0
      IEXIT=0
      ZRES=P(NR+1)
      KNST=JMIN(NR+1)/10
      INST=JMIN(NR+1)-KNST*10
      NRP=NR
	nlecp=nrp
   30 IF (IDXS .EQ. 0) GO TO 80
C------- TREAT S DATA BY AUGMENTING P DATA -----------------------------
      NOS=0
      DO 65 I=1,NRP
      IF (LDX(I) .EQ. 0) GO TO 65
      NOS=NOS+1
      NRS=NRP+NOS
      TP(NRS)=TS(I)
      W(NRS)=WS(I)
      KSMP(NRS)=0
      IF ((KNST.NE.1).AND.(KNST.NE.6)) W(NRS)=0.
      KDX(NRS)=KDX(I)
      LDX(I)=NRS
      WRK(NRS)='    '
   65 CONTINUE
      NR=NRP+NOS
	nlecp=nrp
C------- INITIALIZE TRIAL HYPOCENTER -----------------------------------
   80 K=KDX(NEAR)
      SVY1 = 0.0
      SVY2 = 0.0
      SVY3 = 0.0
      ERLMT = 0.
      DO 25 I = 1,3
      ISKP(I)=0
   25 CONTINUE
      IF (INST .NE. 9) GO TO 90
      READ(12,85) ORG1,ORG2,LAT1,LAT2,LON1,LON2,Z
   85 FORMAT(F5.0,F5.2,I5,F5.2,I5,2F5.2)
      ORG=60.*ORG1+ORG2
      LATEP=60.*LAT1+LAT2
      LONEP=60.*LON1+LON2
      GO TO 105
   90 IF (NR .GE. 4) GO TO 100
   96 WRITE(8,97)
   97 FORMAT(' ***** INSUFFICIENT DATA FOR LOCATING THIS QUAKE:')
      KKF = 1
      IF( NRP .EQ. 0 ) NRP = 1
      DO 98 L=1,NRP
c---- volcniques gua
	delta(i)=3
   	WRITE(*,599) KDATE,KHR,JMIN(L),P(L),S(L),fmp(l),msta(l),prmk(l)
   98 WRITE(8,99) MSTA(L),PRMK(L),KDATE,KHR,JMIN(L),P(L),S(L)
   99 FORMAT(5X,2A4,1X,I6,2I2,F5.2,7X,F5.2,f5.2)
  599 FORMAT(I6,1x,I2,1x,i2,1x,F5.2,7X,F5.2,2x,f5.2,1x,2a4)
      IEXIT=1
c     IF (NRP .EQ. 1) then
c       iheter=ihetol
c       RETURN
c     endif
      GO TO 575
  100 Z=ZTR
	zsv=ztr
      IF (AZRES(NRP+1).NE. '    ') Z=ZRES
      ORG=PMIN-Z/5.-1.
      IF(LATRT.EQ.0.) GO TO 102
      LATEP=LATRT
      LONEP=LONRT
      GO TO 105
  102 IF (LATR .EQ. 0.) GO TO 104
      LATEP=LATR
      LONEP=LONR
      GO TO 105
  104 LATEP=LAT(K)+0.1
      LONEP=LON(K)+0.1
  105 ADJSQ=0.
      IPH=0
      NDEC=0
      PRMSSQ=100000.
      IF (ISW .EQ. '1   ') KNO=MNO(K)
      IF(ISW .EQ. '1   ') FLTEP=FLT(KNO,K)
      NIMAX=TEST(11)+.0001
C------- GEIGER'S ITERATION TO FIND HYPOCENTRAL ADJUSTMENTS ------------
  777 NI = 1
      IF (INST .EQ. 9) NI=NIMAX
  111 IF(ERLMT .EQ. 0.) GO TO 110
      LATEP = LATSV + LA(NA)*DELAT
      LONEP = LONSV + LO(NA)*DELON
c	print *,'GI',lonep/60.,latep/60.
      Z = ZSV + ALZ(NA)*DEZ
      IF(Z .LT.test(15)) Z=test(15)
  110 FMO=0.
      FNO=0.
      DO 112 I=1,5
  112 SUM(I)=0.
c	print *,'GIi',lonep/60.,latep/60.
C------- CALCULATE EPICENTRAL DISTANCE BY RICHTER'S METHOD -------------
      DO 120 I=1,NR
      JI=KDX(I)
      PHI = 0.0174532 * ((LAT(JI)+LATEP)/120.)
      SINPHI = SIN(PHI)
      SINP2  = SINPHI**2
      SINP4  = SINP2**2
      CA = 1.8553654 + 0.0062792*SINP2 + 0.0000319*SINP4
      CB = 1.8428071 + 0.0187098*SINP2 + 0.0001583*SINP4
      DX(I) = (LON(JI)-LONEP) * CA * COS(PHI)
      DY(I) = (LAT(JI)-LATEP) * CB
      DELTA(I)=SQRT(DX(I)**2+DY(I)**2)+0.000001
      WT(I)=W(I)
      IF (NI .LE. 1) GO TO 115
C------- DISTANCE WEIGHTING --------------------------------------------
      IF (DELTA(I) .LE. XNEAR) GO TO 115
      WT(I)=W(I)*(XFAR-DELTA(I))/XFN
      IF (WT(I) .LT. 0.005) WT(I)=0.
  115 IF (WT(I) .EQ. 0.) GO TO 120
      IF (KSMP(I) .EQ. 1) FMO=FMO+1.
      FNO=FNO+1.
      SUM(4)=SUM(4)+WT(I)
  120 CONTINUE
      IF (FNO .LT. 4.) GO TO 96
      AVWT=SUM(4)/FNO
C------- NORMALIZE DISTANCE WEIGHTS ------------------------------------
      SUM(4)=0.0
      DO 122 I=1,NR
  122 WT(I)=WT(I)/AVWT
      IF ((NI.LE.2).OR.(KAZ.EQ.0)) GO TO 130
C------- AZIMUTHAL WEIGHTING -------------------------------------------
      CALL AZWTOS(DX,DY,NR,WT,KDX,AZ,TEMP,KEY,INS,IEW)
C------- COMPUTE TRAVEL TIMES & DERIVATIVES ----------------------------
  130 ZSQ=Z**2
c************** on entre dans trvdrv quand iflag=0 ( premiere passe ou
c               premier passage de la deuxieme passe )
      if(iflag.eq.0) CALL TRVDRV(ISW,V,D,DEPTH,VSQ,NL,THK,H,G,F,
     &  TID,DID,FLT,DELTA,DX,DY,NR,KDX,KNO,FLTEP,Z,ZSQ,X,T,ANIN)
  131 FDLY=1.
      IF (ISW .EQ. '1   ') FDLY=0.
C------- CALCULATE TRAVEL TIME RESIDUALS X(4,I) & MODIFY THE DERIV'S ---
      DO 150 I=1,NR
      JI=KDX(I)
      IF (I .LE. NRP) GO TO 145
C------- S PHASE DATA --------------------------------------------------
      T(I)=POS*T(I)
      X(1,I)=POS*X(1,I)
      X(2,I)=POS*X(2,I)
      X(3,I)=POS*X(3,I)
      X(4,I)=TP(I)-T(I)-ORG-POS*DLY(KNO,JI)*FDLY
      GO TO 150
  145 IF (KSMP(I) .EQ. 0) GO TO 146
C------- S-P DATA ------------------------------------------------------
      X(1,I)=(POS-1.)*X(1,I)
      X(2,I)=(POS-1.)*X(2,I)
      X(3,I)=(POS-1.)*X(3,I)
      X(4,I)=TS(I)-TP(I)-(POS-1.)*(DLY(KNO,JI)*FDLY+T(I))
      GO TO 150
C------- P TRAVEL TIME RESIDUAL ----------------------------------------
  146 X(4,I)=TP(I)-T(I)-ORG-DLY(KNO,JI)*FDLY
  150 CONTINUE
C------- COMPUTE AVR, AAR, RMSSQ, & SDR --------------------------------
      ONF=0.0
      DO 152 I=1,NR
      ONF = ONF + WT(I)*(1-KSMP(I))
      XWT = X(4,I)*WT(I)
      SUM(1)=SUM(1)+XWT
      SUM(2)=SUM(2)+ABS(XWT)
      SUM(3)=SUM(3)+X(4,I)*XWT
      SUM(5)=SUM(5)+XWT*(1-KSMP(I))
  152 CONTINUE
      IF(FNO .GT. FMO) AVRPS=SUM(5)/(ONF)
      AVR=SUM(1)/FNO
      AAR=SUM(2)/FNO
      RMSSQ=SUM(3)/FNO
      SDR=SQRT(ABS(RMSSQ-AVR**2))
      DO 153 I=1,5
      SUM(I)= 0.0
  153 CONTINUE
      IF (RMSSQ .GE. TEST(1)) GO TO 154
      IF(ERLMT .EQ. 1.) GO TO 167
      IF(INST.EQ.9) GO TO 501
      IF(NI .GE. 2) GO TO 167
      GO TO 165
C------- JEFFREYS' WEIGHTING -------------------------------------------
  154 FMO=0.
      FNO=0.
      DO 160 I=1,NR
      WRK(I)='    '
      IF (WT(I) .EQ. 0.) GO TO 160
      K=10.*ABS(X(4,I)-AVR)/SDR+1.5
      IF (K .GT. 41) K=41
      WT(I)=WT(I)*WF(K)
      IF (K .GT. 30) WRK(I)='****'
      IF (WT(I) .LT. 0.005) WT(I)=0.
      IF (WT(I) .EQ. 0.) GO TO 160
      IF (KSMP(I) .EQ. 1) FMO=FMO+1.
      FNO=FNO+1.
      SUM(4)=SUM(4)+WT(I)
  160 CONTINUE
      IF (FNO .LT. 4.) GO TO 96
      AVWT=SUM(4)/FNO
      SUM(4)=0.0
      ONF=0.0
      DO 164 I=1,NR
      WT(I)=WT(I)/AVWT
      ONF = ONF + WT(I)*(1-KSMP(I))
      XWT=X(4,I)*WT(I)
      SUM(5)=SUM(5)+XWT*(1-KSMP(I))
  164 CONTINUE
C------- RECALCULATE AVRPS ---------------------------------------------
      IF(ERLMT .EQ. 1.) GO TO 163
      IF(INST .NE. 9) GO TO 163
      AVRPS = 0.0
      IF(FNO .NE. FMO) AVRPS = SUM(5)/ONF
      GO TO 501
  163 IF(FNO.EQ.FMO) AVRPS=0.0
      IF(FNO.EQ.FMO) GO TO 167
      AVRPS=SUM(5)/(ONF)
      SUM(5)=0.0
      IF(ERLMT .EQ. 1.) GO TO 167
C------- RESET FIRST ORIGIN TIME ---------------------------------------
      IF(NI.GE. 2) GO TO 167
  165 ORG=ORG+AVRPS
      DO 166 I=1,NR
      IF(KSMP(I) .EQ. 0) X(4,I)=X(4,I)-AVRPS
      XWT=WT(I)*X(4,I)
      SUM(5)=SUM(5)+XWT*(1 - KSMP(I))
      SUM(2)=SUM(2)+ABS(XWT)
      SUM(3)=SUM(3)+X(4,I)*XWT
  166 CONTINUE
      IF(FNO .GT. FMO) AVRPS=SUM(5)/(ONF)
      AAR=SUM(2)/FNO
      RMSSQ = SUM(3)/FNO
      GO TO 169
C------- FOR NI>1, COMPUTE AAR, & RMSSQ AS IF AVRPS=0. -----------------
  167 DO 168 I=1,NR
      XWT=WT(I)*(X(4,I)-AVRPS*(1-KSMP(I)))
      SUM(2)=SUM(2)+ABS(XWT)
      SUM(3)=SUM(3)+(X(4,I)-AVRPS*(1-KSMP(I)))*XWT
  168 CONTINUE
      AAR=SUM(2)/FNO
      RMSSQ=SUM(3)/FNO
      IF(ERLMT .EQ. 0.) GO TO 169
C------- OUTPUT RMS ERROR OF AUXILIARY POINTS --------------------------
      L = LATEP/60.
      ALA = LATEP - 60.*L
      L = LONEP/60.
      ALO = LONEP - 60.*L
      RMSX= SQRT(RMSSQ)
      DRMS = RMSX - RMSSV
      GO TO (1,2,3,4,5,6,1,2,3,4), NA
    1 WRITE(8,801) ALA,ALO,Z,AVRPS,RMSX,DRMS
  801 FORMAT(5F10.2,10X,F6.2)
      GO TO 174
    2 WRITE(8,802) ALA,ALO,Z,AVRPS,RMSX,DRMS
  802 FORMAT(5F10.2,28X,F6.2)
      GO TO 174
    3 WRITE(8,803) ALA,ALO,Z,AVRPS,RMSX,DRMS
  803 FORMAT(5F10.2,13X,'(',F6.2,')')
      GO TO 174
    4 WRITE(8,804) ALA,ALO,Z,AVRPS,RMSX,DRMS
  804 FORMAT(5F10.2,31X,'(',F6.2,')')
      IF(NA .EQ. 10) GO TO 550
      GO TO 174
    5 WRITE(8,805) ALA,ALO,Z,AVRPS,RMSX,DRMS
  805 FORMAT(/5F10.2,19X,F6.2)
      WRITE(8,807) RMSSV
  807 FORMAT(40X,F10.2,23X,'0.00')
      GO TO 174
    6 WRITE(8,806) ALA,ALO,Z,AVRPS,RMSX,DRMS
  806 FORMAT(5F10.2,22X,'(',F6.2,')'/)
  174 NA = NA + 1
      GO TO 111
C------- CHECK IF SOLUTION IS BETTER THAN PREVIOUS ONE -----------------
  169 IF((NI .EQ. 1) .AND. (NDEC .EQ. 0)) GO TO 170
      IF(PRMSSQ.GE.RMSSQ) GO TO 170
      NDEC = NDEC +1
      IF(NDEC .GT. 1) GO TO 175
      DO 177 I= 1,3
      B(I) = 0.0
      AF(I)=-1.0
      SE(I) = 0.0
  177 CONTINUE
      NI = NI -1
      BM1=Y(1)
      BM2=Y(2)
      BM3=Y(3)
      BMAX = ABS(Y(1))
      IIMAX = 1
      DO 176 I = 2,3
      IF(ABS(Y(I)).LE.BMAX) GO TO 176
      BMAX = ABS(Y(I))
      IIMAX = I
  176 CONTINUE
      ISKP(IIMAX)=1
      Y(1)=-BM1/5.
      Y(2)=-BM2/5.
      Y(3)=-BM3/5.
      Y(4)=-Y(1)*XMEAN(1)-Y(2)*XMEAN(2)-Y(3)*XMEAN(3)
      XADJSQ=Y(1)**2+Y(2)**2+Y(3)**2
      KP=0
      IF(XADJSQ .LT. 4.*TEST(4)/25.) GO TO 170
  175 IF(NDEC .EQ. 5) GO TO 170
      GO TO 325
C------- STEPWISE MULTIPLE REGRESSION ANALYSIS OF TRAVEL TIME RESIDUALS-
  170 IF(NDEC .GE. 1) NI = NI + 1
      IF (INST.EQ.1) GO TO 250
      IF(ISKP(3) .EQ. 1) GO TO 250
      IF (INST .EQ. 9) GO TO 501
      IF ((FNO.EQ.4) .AND. (FMO.LT.4)) GO TO 250
C---- FREE SOLUTION
      KZ=0
      KF=0
      CALL SWMREG(TEST,IPRN,NR,KSMP,FNO,X,WT,ISKP,KF,KZ,XMEAN,
     &  B,Y,SE,AF,ONF,FLIM)
C------- AVOID CORRECTING DEPTH IF HORIZONTAL CHANGE IS LARGE ----------
      IF (Y(1)**2+Y(2)**2 .LT. TEST(2)) GO TO 300
C---- FIXED DEPTH SOLUTION
  250 KZ=1
      KF=0
      CALL SWMREG(TEST,IPRN,NR,KSMP,FNO,X,WT,ISKP,KF,KZ,XMEAN,
     &  B,Y,SE,AF,ONF,FLIM)
C------- LIMIT FOCAL DEPTH CHANGE & AVOID HYPOCENTER IN THE AIR --------
  300 DO 275 I= 1,3
      ISKP(I)=0
  275 CONTINUE
      OLDY1=Y(1)
      OLDY2=Y(2)
      OLDY3=Y(3)
      ABSY1=ABS(Y(1))
      ABSY2=ABS(Y(2))
      ABSY3=ABS(Y(3))
      IF(ABSY1.GT.ABSY2) GO TO 305
      ABSGR=ABSY2
      GO TO 308
  305 ABSGR=ABSY1
  308 IF(ABSY3.LE.TEST(5)) GO TO 310
      I=ABSY3/TEST(5)
      Y(3)=Y(3)/(I+1)
  310   continue
c	print *,'------>',z,y(3),z+y(3),test(15)
	 IF((Z+Y(3)).GT.test(15)) GO TO 315
      Y(3)=-Z*TEST(12)+.000001
      ISKP(3) = 1
C------- LIMIT HORIZONTAL ADJUSTMENT OF EPICENTER ----------------------
  315 	continue
c	print *,z,y(3)
	IF(ABSGR.LE.TEST(10)) GO TO 320
      I=ABSGR/TEST(10)
      Y(1)=Y(1)/(I+1)
      Y(2)=Y(2)/(I+1)
  320 Y(4)=Y(4)-(Y(3)-OLDY3)*XMEAN(3)-(Y(1)-OLDY1)*XMEAN(1)
     1 -(Y(2)-OLDY2)*XMEAN(2)
      XADJSQ=Y(1)**2+Y(2)**2+Y(3)**2
      KP=0
      NDEC=0
      JPH=0
c************** on sort le resultat si iprn > 1 dans le cas ou
c               deuxieme passe alors iflag # 0
c 325 IF (IPRN .GE. 1.and.(iflag.ne.0.or.ihetol.eq.0)) CALL OUTPUT
  325 IF (IPRN .GE. 1) CALL OUTPUT
     &  (TEST,KNO,IW,INS,IEW,DLY,FMGC,XMGC,
     &  KLAS,PRR,CALR,ICAL,FLT,QSPA,MSTA,
     &  PRMK,JMIN,P,S,SRMK,AMX,PRX,CALX,RMK,DT,FMP,AZRES,QRMK,KDX,LDX,
     &  WT,TP,T,WRK,KSMP,TS,TIME1,TIME2,DELTA,DX,DY,AVXM,
     &  XMAG,AVFM,FMAG,MAG,FNO,X,B,Y,SE,AF,AZ,AIN,ANIN,TEMP,KEY)
      IF(NDEC .GE. 1) GO TO 330
C------- TERMINATE ITERATION IF HYPOCENTER ADJUSTMENT < TEST(4) --------
      IF (XADJSQ .LT. TEST(4)) GO TO 500
  330 IF(NI .EQ. NIMAX) GO TO 500
C------- ADJUST HYPOCENTER ---------------------------------------------
      PHI = 0.0174532 * (LATEP/60.)
      SINPHI = SIN(PHI)
      SINP2  = SINPHI**2
      SINP4  = SINP2**2
      CA = 1.8553654 + 0.0062792*SINP2 + 0.0000319*SINP4
      CB = 1.8428071 + 0.0187098*SINP2 + 0.0001583*SINP4
      LATEP = LATEP + (Y(2)/CB)
      LONEP = LONEP + (Y(1)/(CA*COS(PHI)))
      Z=Z+Y(3)
      ORG=ORG+Y(4)
      SVY1 = Y(1)
      SVY2 = Y(2)
      SVY3 = Y(3)
      ADJSQ=XADJSQ
      IF(NDEC .EQ. 0) PRMSSQ=RMSSQ
      IF(NDEC.GE.1) GO TO 110
      NI = NI + 1
      IF(NI .LE. NIMAX) GO TO 111
C------- RESET ORIGIN TIME ---------------------------------------------
  500 ORG=ORG+XMEAN(4)
      GO TO 502
  501 XMEAN(4)=0.0
  502 DO 505 I=1,5
  505 SUM(I)=0.0
      SUMM = 0.0
      DO 510 I=1,NR
      IF (KSMP(I) .EQ. 0) X(4,I)=X(4,I)-XMEAN(4)
      IF (WT(I) .EQ. 0.) GO TO 510
      IF(INST .NE. 9) GO TO 509
      XWTS=WT(I)*(X(4,I)**2)
      IF(KSMP(I) .EQ. 0) XWTS=WT(I)*((X(4,I)-AVRPS)**2)
      SUMM = SUMM + XWTS
  509 XWT=X(4,I)*WT(I)
      SUM(1)=SUM(1)+XWT
      SUM(2)=SUM(2)+ABS(XWT)
      SUM(3)=SUM(3)+X(4,I)*XWT
      SUM(5)=SUM(5)+XWT*(1-KSMP(I))
  510 CONTINUE
      RM9SV = SUMM/FNO
      AVR=SUM(1)/FNO
      AVRPS = 0.0
      IF(FNO .GT. FMO) AVRPS=SUM(5)/ONF
      AAR=SUM(2)/FNO
      RMSSQ=SUM(3)/FNO
C------- COMPUTE ERROR ESTIMATES BY SOLVING FULL NORMAL EQUATION -------
      KF=2
      KP=1
      KZ=0
      CALL SWMREG(TEST,IPRN,NR,KSMP,FNO,X,WT,ISKP,KF,KZ,XMEAN,
     &  B,Y,SE,AF,ONF,FLIM)
      DO 521 I =1,3
  521 Y(I)=0.0
      IF(INST.EQ.1) KZ = 1
c***************  on donne si ihetol=0 ou iflag # 0  voir plus haut
c     if(iflag.ne.0.or.ihetol.eq.0)
       CALL OUTPUT(TEST,KNO,IW,INS,IEW,DLY,FMGC,XMGC,
     &  KLAS,PRR,CALR,ICAL,FLT,QSPA,MSTA,
     &  PRMK,JMIN,P,S,SRMK,AMX,PRX,CALX,RMK,DT,FMP,AZRES,QRMK,KDX,LDX,
     &  WT,TP,T,WRK,KSMP,TS,TIME1,TIME2,DELTA,DX,DY,AVXM,
     &  XMAG,AVFM,FMAG,MAG,FNO,X,B,Y,SE,AF,AZ,AIN,ANIN,TEMP,KEY)
      IF (KMS .EQ. 1) CALL MISING
     &  (NSTA,LAT,LON,NS,MAG,TEMP,DMIN,JDX,
     &  JMAX,LATEP,LONEP,INS,IEW)
      IF ((KNST.GE.5) .OR. (KFM.GE.1)) CALL FMPLOT
     &                   (KPAPER,KFM,FNO,NRP,AZ,AIN,SYM,SUCARD)
      QNO(JAV)=QNO(JAV)+1.
      IF (JAV .GT. IQ) GO TO 523
C------- COMPUTE SUMMARY OF TRAVEL TIME RESIDUALS ----------------------
      DO 522 I=1,NRP
      IF ((WT(I).EQ.0.) .OR. (KSMP(I).EQ.1))  GO TO 522
      JI=KDX(I)
      NRES(KNO,JI)=NRES(KNO,JI)+1
      SR(KNO,JI)=SR(KNO,JI)+X(4,I)*WT(I)
      SRSQ(KNO,JI)=SRSQ(KNO,JI)+X(4,I)**2*WT(I)
      SRWT(KNO,JI)=SRWT(KNO,JI)+WT(I)
  522 CONTINUE
  523 IF (KTEST .NE. 1.) goto 550
C------- COMPUTE RMS AT AUXILIARY POINTS -------------------------------
      RMSSV = SQRT(RMSSQ)
      IF(INST.EQ.9) RMSSV = SQRT(RM9SV)
      ERLMT = 1.
      LATSV = LATEP
      LONSV = LONEP
      ZSV = Z
      PHI = 0.0174532 * (LATEP/60.)
      SINPHI = SIN(PHI)
      SINP2  = SINPHI**2
      SINP4  = SINP2**2
      CA = 1.8553654 + 0.0062792*SINP2 + 0.0000319*SINP4
      CB = 1.8428071 + 0.0187098*SINP2 + 0.0001583*SINP4
      DELAT = TEST(13)/CB
      DELON = TEST(13)/(CA*COS(PHI))
      DEZ = TEST(13)
      WRITE (8,525)
  525 FORMAT (/'       LAT       LON         Z     AVRPS       RMS
     1                DRMS'/)
      NA=1
      GO TO 111
  550 TIME1=TIME2
  575 CONTINUE
C------- CHECK FOR MULTIPLE SOLUTIONS OF THE SAME EARTHQUAKE -----------
      IF(IPRO.NE.' ** ') RETURN
      NR=NRP
      NRP1=NR +1
      READ(12,600)  CHECK,IPRO,KNST,INST,ZRES,LAT1,LAT2,LON1,LON2,
     1 AZRES(NRP1)
      WRITE(8,601) CHECK,IPRO,KNST,INST,ZRES,LAT1,LAT2,LON1,LON2
  601 FORMAT(//2A4,9X,2I1,F5.2,1X,2(I4,F6.2),'--- RUN AGAIN ---')
  600 FORMAT(2A4,9X,2I1,F5.2,1X,2(I4,F6.2),T21,A4)
      LATRT=60.*LAT1+LAT2
      LONRT=60.*LON1+LON2
      IF(CHECK.EQ.'    ') GO TO 30
      WRITE(8,610) CHECK
  610 FORMAT(/' ERROR ',A4,' SKIPPED.   INST. CARD DID NOT FOLLOW ***')
      RETURN
      END
