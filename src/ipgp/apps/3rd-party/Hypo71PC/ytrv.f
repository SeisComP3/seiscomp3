      SUBROUTINE TRVDRV(ISW,V,D,DEPTH,VSQ,NL,THK,H,G,F,TID,DID,FLT,
     &  DELTA,DX,DY,NR,KDX,KNO,FLTEP,z,ZSQ,X,T,ANIN)
C------- COMPUTE TRAVEL TIME AND DERIVATIVES FROM CRUSTAL MODEL --------
        include 'H_param.f'
      CHARACTER*4 ISW
      INTEGER*4 KDX(N_dat)
      REAL*4 DELTA(N_dat),DX(N_dat),DY(N_dat),T(N_dat),ANIN(N_dat)
      REAL*4 V(21),D(21),VSQ(21),THK(21),H(21),DEPTH(21)
        real*4 TID(21,21),DID(21,21),F(21,21)
      REAL*4 G(4,21),FLT(2,N_sta),X(4,N_dat)
        common /txcal/ dxd(21,21),txd(21,21)
        common /gradi/nlecp

      REAL*4 TINs(21),DIDs(21),TR(21),tinf(21),didf(21)
        real*8 wx,wy,wz,wf,wv,wg,wr,wh,wgx,wgy,wgz,w1,w2,w3,w4
        real*8 timz3
      common/provi/epsob,iflag,elev(N_sta)
        data indr/0/
c----------------- modele a gradient ----------------------------------
        if(nl.eq.0) then
                is=0
                wh=v(1)-depth(1)
                wv=v(2+is)
                wg=v(3+is)
                wr=v(4+is)
                wf=z-depth(1)
        do k=1,nlecp
                ji = kdx(k)
                       wz=elev(ji)-depth(1)
                wx=dx(k)
                wy=dy(k)
                t(k)=timz3(wx,wy,wz,wf,wv,wg,wr,wh,wgx,wgy,wgz,
     +                w1,w2,w3,w4)
                X(1,k)=-wgx
                      X(2,k)=-wgy
                      X(3,k)= wgz
        enddo
        if(nr.gt.nlecp) then
                is=3
                wh=v(1)
                wv=v(2+is)
                wg=v(3+is)
                wr=v(4+is)
                wf=z
                                        do k=nlecp+1,nr
                ji = kdx(k)
                       wz=elev(ji)-depth(1)
                wx=dx(k)
                wy=dy(k)
                t(k)=timz3(wx,wy,wz,wf,wv,wg,wr,wh,wgx,wgy,wgz,
     +                w1,w2,w3,w4)
                X(1,k)=-wgx
                      X(2,k)=-wgy
                      X(3,k)= wgz
                                        enddo
        endif
                return
        endif
c---------------------------------------------------------------
         IF (ISW .EQ. '1   ') then
                print *,'the variable first layer not implemented'
                stop
        endif
C-----------------------------------------------------------------------
        if(indr.eq.0) then
        do lr=2,nl
                do ls=1,nl
                        dxd(ls,lr)=0
                        txd(ls,lr)=0
                enddo
                do ls=1,nl
                        do ll=ls,lr-1
                              sqt=sqrt(vsq(lr)-vsq(ll))
                              tim=thk(ll)/v(ll)*v(lr)/sqt
                              dim=thk(ll)*v(ll)/sqt
                dxd(ls,lr)= dxd(ls,lr) + dim
                txd(ls,lr)= txd(ls,lr) + tim
c                print *,lr,ls,ll,txd(ls,lr),dxd(ls,lr),tim,dim
                        enddo
                enddo
        enddo
        indr=1
c        print *, 'init done'
        endif
        thk0=thk(1)
C-----------------------------------------------------------------------
                                IF (ISW .ne. '1   ') then
C-----INITIALIZATION FOR FIXED LAYER MODEL --once for all stations------
        call prep1(nl,z,d,v,vsq,txd,dxd,tinf,didf,xomaxf,jf,tkjf)
                                      endif
c--------------------------------------------------------------------
c---------------  loop over stations --------------------------------
      DO 300 I=1,NR
        ji = kdx(i)
        zstat=elev(ji)
        delti=delta(i)
        if(delti.lt.0.01) delti=0.01
c        thk(1)=thk0-zstat +d(1)
c                                     IF (ISW .eq. '1   ') goto 701
c------ find station layer
        call prep1(nl,zstat,d,v,vsq,txd,dxd,tins,dids,xomaxs,js,tkjs)
c        print *,'---> ',z,zstat,delti,js,jf,xomaxs,xomaxf
        tmin=9999
        jm=21
        tr(21)=tmin
              IF (Jf .EQ. NL) GO TO 90
         if(delti.lt.xomaxs+xomaxf) goto 90
C-----TRAVEL TIME & DERIVATIVES FOR REFRACTED WAVE
80         continue
        call refwav(nl,jf,tinf,didf,js,tins,dids,v,delti,tr,tmin,jm)
        goto 90
82    T(I)=TR(jm)
      DTDD=1.0/V(jm)
      DTDH=-SQRT(VSQ(jm)-VSQ(jf))/(V(jm)*V(jf))
      ANIN(I)=-V(jf)/V(jm)
      GO TO 260
90         continue
        if(z.gt.zstat) then
        call dirwav(z,jf,tkjf,zstat,js,tkjs,tins,dids,delti,tmin,1.,thk
     1 ,v,vsq,dtdd,dtdh,anin(i),t(i))
        else
        call dirwav(zstat,js,tkjs,z,jf,tkjf,tinf,didf,delti,tmin,-1.,thk
     1 ,v,vsq,dtdd,dtdh,anin(i),t(i))
        endif
        if(t(i).gt.tmin) goto 82

C-----SET UP PARTIAL DERIVATIVES FOR REGRESSION ANALYSIS ---------------
c-------------- c'est bien -px,-py,-pz       d'apres les formules
  260         continue
                X(1,I)=-DTDD*DX(I)/DELTI
                      X(2,I)=-DTDD*DY(I)/DELTI
      X(3,I)=DTDH
  300 CONTINUE
        thk(1)=thk0
      RETURN






c--- fossile
C-----                    INITIALIZATION FOR VARIABLE LAYER MODEL ---
c701   js=1
c      ff=1
c      DEPTH(2)=FLT(KNO,JI)
c      IF (Z .LT. FLTEP) DEPTH(2)=0.5*(FLT(KNO,JI)+FLTEP)
c      THK(1)=DEPTH(2)
c      THK(2)=D(3)-DEPTH(2)
c      DH1=THK(1)-H(1)
c      DH2=THK(2)-H(2)
c      DO L=1,NL
c         IF (DEPTH(L) .GT. Z) then
c            JJ=L
c            JL=L-1
c            GO TO 30
c         endif
c      enddo
c      JL=NL
c   30 TKJ=Z-DEPTH(JL)
c      TKJSQ=TKJ**2+0.000001
c      IF(JL.EQ.NL) GO TO 100
cC-----                    CALCULATION FOR REFRACTED WAVES -----------
c      DO L=JJ,NL
c         SQT=SQRT(VSQ(L)-VSQ(JL))
c         TIX=F(1,JL)*DH1*G(1,L)+F(2,JL)*DH2*G(2,L)+TID(JL,L)
c         DIX=F(1,JL)*DH1*G(3,L)+F(2,JL)*DH2*G(4,L)+DID(JL,L)
c         TINJ(L)=TIX-TKJ*SQT/(V(L)*V(JL))
c         DIDJ(L)=DIX-TKJ*V(JL)/SQT
c      enddo
c      TIX=F(1,JL)*DH1*G(1,JL)+F(2,JL)*DH2*G(2,JL)+TID(JL,JL)
c      XOVMAX=V(JJ)*V(JL)*(TINJ(JJ)-TIX)/(V(JJ)-V(JL))
c------ fin fossile

c        goto 80




      END

c----------------calcul des refractees
c
c80        call refwav(nl,jf,tinf,didf,js,tins,dids,v,delta(i),tr,tmin,jm)
      subroutine refwav(nl,jf,tinf,didf,js,tins,dids,v,delta,tr,tmin,jm)
        dimension tinf(1),tins(1),didf(1),dids(1),v(1)
        dimension tr(1)
                                        dimension fdx(21)

        jj=max(jf,js)+1
      DO M=JJ,NL
        fdx(m)=didf(m)+dids(m)
        tr(m)=tmin
        if(fdx(m).le.delta) TR(M)=TINs(M)+tinf(m)+(DELTA-fdx(m))/V(M)
      enddo
      DO M=JJ,NL
         IF (TR(M) .lt. TMIN) then
               jm=M
               TMIN=TR(M)
         endif
      enddo
        return
        end


        subroutine dirwav(z,jl,tkj,zstat,js,tkjs,tins,dids,delta,tmin,ff
     1    ,thk,v,vsq,dtdd,dtdh,anin,t)
        dimension v(1),vsq(1),thk(1),tins(1),dids(1)
        character*4 isw
C
C-----CALCULATION FOR DIRECT WAVE --focus and station in the same layer
        thk0=thk(js)
        ddz=z-zstat
        if(ddz.lt.0.01) ddz=0.01
c         print *,'dirwav ',delta,ddz,jl,js,tkj,tkjs
      IF (JL .eq.js) then
         SQT=SQRT((ddz)**2+DELTA**2)
         TDJ1=SQT/V(js)
C-----TRAVEL TIME & DERIVATIVES FOR DIRECT WAVE IN FIRST LAYER
         T=TDJ1
c        IF (TDJ1 .GE. TMIN) goto 800
         DTDD=DELTA/(V(js)*SQT)
         DTDH=(ddz) /(V(js)*SQT) *ff
         ANIN=DELTA/SQT
                goto 800
      endif
        thk(js)=thk0-tkjs

C-----FIND A DIRECT WAVE THAT WILL EMERGE AT THE STATION
  100 XBIG=DELTA
        TKJSQ=TKJ**2+0.000001
              XLIT=DELTA*TKJ/ddz
        if(xlit.gt.delta) then
                xlit=0.
                print *,'biz ',tkj,delta,ddz
        endif
      UB=XBIG/SQRT(XBIG**2+TKJSQ)
      UL=XLIT/SQRT(XLIT**2+TKJSQ)
      UBSQ=UB**2
      ULSQ=UL**2
      DELBIG=TKJ*UB/SQRT(1.000001-UBSQ)
      DELLIT=TKJ*UL/SQRT(1.000001-ULSQ)
cn        print *,'< ',delbig,' - ',dellit,'=',delbig-dellit
      J1=JL-1
      DO L=js,J1
cn        print *,l,ub,ul,ubsq,ulsq,thk(l),vsq(jl),vsq(l)
         DELBIG=DELBIG+(THK(L)*UB)/SQRT(VSQ(JL)/VSQ(L)-UBSQ)
         DELLIT=DELLIT+(THK(L)*UL)/SQRT(VSQ(JL)/VSQ(L)-ULSQ)
cn        print *,'< ',delbig,' - ',dellit,'=',delbig-dellit
      enddo
      DO LL=1,25
cn        print *,'< ',delbig,' - ',dellit,'=',delbig-dellit
         if (DELBIG-DELLIT .LT. 0.02) then
            XTR=0.5*(XBIG+XLIT)
            U=XTR/SQRT(XTR**2+TKJSQ)
            USQ=U**2
        else
            XTR=XLIT+(DELTA-DELLIT)*(XBIG-XLIT)/(DELBIG-DELLIT)
            U=XTR/SQRT(XTR**2+TKJSQ)
            USQ=U**2
         endif
         DELXTR=TKJ*U/SQRT(1.000001-USQ)
         do L=js,J1
            DELXTR=DELXTR+(THK(L)*U)/SQRT(VSQ(JL)/VSQ(L)-USQ)
         enddo
         XTEST=DELTA-DELXTR
         IF (ABS(XTEST) .LE. 0.02) GO TO 190
         IF (XTEST.lt.0.) then
            XBIG=XTR
            DELBIG=DELXTR
         else
            XLIT=XTR
            DELLIT=DELXTR
         endif
        if(ll.gt.10) then
            IF (1.0-U .LT. 0.0002) GO TO 190
        endif
      enddo

  190 IF (1.0-U .le. 0.0002) then
C-----IF U IS TOO NEAR 1, COMPUTE TDIR AS WAVE ALONG THE TOP OF LAYER JL
cn        print *,u
c        IF (ISW .EQ. '1   ') then
c--- fossile
c           TIX=F(1,JL)*DH1*G(1,JL)+F(2,JL)*DH2*G(2,JL)+TID(JL,JL)
c           TDC=TIX+DELTA/V(JL)
c--- fin fossile
c        else
            TDC=tins(JL)+(DELTA-dids(jl))/V(JL)
c        endif
            T=TDC
            DTDD=1.0/V(JL)
            DTDH=0.0
cn        print *,'fine ',delta,t,dtdd,dtdh
            ANIN=0.9999999
                goto 800
      endif

C-----TRAVEL TIME & DERIVATIVES FOR DIRECT WAVE BELOW FIRST LAYER
c--------------- travel time ---------------
      TDIR=TKJ/(V(JL)*SQRT(1.0-USQ))
cn        print *,tdir
      DO L=js,J1
         TDIR=TDIR+(THK(L)*V(JL))/(VSQ(L)*SQRT(VSQ(JL)/VSQ(L)-USQ))
c        print *,delta,tdir
      enddo
      T=TDIR
        if(tdir.gt.tmin) then
                dtdd=1.
                dtdh=1.
                goto 800
        endif
c---------------- derivatives --------------
      SRR=SQRT(1.-USQ)
      SRT=SRR**3
        if(srr.lt.0.001) then
                dtdd=0.
                dtdh=0.
                anin=0.
                goto 800
        endif

      ALFA=TKJ/SRT
      BETA=TKJ*U/(V(JL)*SRT)
      DO L=js,J1
         STK=(SQRT(VSQ(JL)/VSQ(L)-USQ))**3
         VTK=THK(L)/(VSQ(L)*STK)
         ALFA=ALFA+VTK*VSQ(JL)
         BETA=BETA+VTK*V(JL)*U
      enddo
      DTDD=BETA/ALFA
        if(ff.gt.0.) then
                jfo=jl
                else
                jfo=js
        endif
      DTDH=(1.0-V(Jfo)*U*DTDD)/(V(Jfo)*SRR)*ff
      ANIN=U
800        continue
        thk(js)=thk0
c        print *,'der ',t,dtdh,dtdd
c        print *,'dir*-',(thk(k),k=1,5)
        return
        end





      subroutine prep1(nl,z,d,v,vsq,tid,did,tinj,didj,xomax,jl,tkj)
      REAL*4 V(21),D(21),VSQ(21),TID(21,21),DID(21,21)
      REAL*4 TINJ(21),DIDJ(21)
c-------------- determine layer -------------------------
         DO   L=2,NL
            IF (D(L) .GT. Z) then
               jj=l
               jl=l-1
               goto 3
            endif
         enddo
         JL=NL
    3    TKJ=Z-D(JL)
c-------------- downgoing rays time and distance --------
        xomax=9999.
         IF (JL .ne. NL) then
            DO  L=JJ,NL
               SQT=SQRT(VSQ(L)-VSQ(JL))
               TINJ(L)=TID(JL,L)-TKJ/SQT*V(L)/V(JL)
               DIDJ(L)=DID(JL,L)-TKJ*V(JL)/SQT
               if(didj(l).lt.xomax) xomax=didj(l)
            enddo
c           XOMAX=V(JJ)*V(JL)*(TINJ(JJ)-TID(JL,JL))/(V(JJ)-V(JL))
         endif
        return
        end
