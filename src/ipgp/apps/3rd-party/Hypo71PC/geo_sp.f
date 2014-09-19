      FUNCTION ArcMe(Latitude)
c,<901226.1130>
C     ==============
C
C     Calcul de la longueur de l'Arc de Meridien
C     Latitude en Radians , Resultat en metres
C
      Implicit none
C
      REAL*8 Alpha,ArcMe,Beta,Constante,Delta,Gamma,J2,J4,J6,J8,
     *    Latitude,U,V
C
      DATA Alpha/0.0050761277D0/,Beta/0.0000429451D0/,
     *    Constante/6399936.608D0/,Delta/0.0000000013D0/,
     *    Gamma/0.0000001696D0/
C
      V=DCOS(Latitude)
      U=V*DSIN(Latitude)
      V=V**2
      J2=Latitude+U
      J4=(3.*J2+2.*U*V)/4.
      J6=(5.*J4+2.*U*V**2)/3.
      J8=(7.*J6+4.*U*V**3)/8.
C
      ArcMe=Constante*(Latitude-Alpha*J2+Beta*J4-Gamma*J6+Delta*J8)
C
      END
      FUNCTION ArcMer(Ellips,Latitude1,Latitude2)
c,<901226.1130>
C     ===============
C
C     Calcul de l'arc de meridien entre 2 latitudes donnees
C
C->   Ellips(2) - a0,e2 parametres de l'ellipsoide
C->   ArcMer= arc de Latitude1 a Latitude2
C
      Implicit None
C
      INTEGER No
C
      REAL*8 ArcMer,b(8),Beta0,Beta2,Beta4,Beta6,Beta8,e1,Ellips(2),
     *    Latitude1,Latitude2,p1,p2,p3,p4,s1
C
      s1=1.
      e1=1.-Ellips(2)
      b(1)=Ellips(1)*e1
      DO No=1,4
          s1=s1*Ellips(2)*(2*No+1)/(2*No)
          b(2*No)=Ellips(1)*e1*s1
      ENDDO
C
      Beta0=b(1)+.5*b(2)+3./8.*b(4)+5./16.*b(6)+35./128.*b(8)
      Beta2=.5*(b(2)+b(4))+15./32.*b(6)+28./64.*b(8)
      Beta4=1./8.*b(4)+3./16.*b(6)+28./128.*b(8)
      Beta6=1./32.*b(6)+4./64.*b(8)
      Beta8=1./128.*b(8)
C
      p1=Beta0*(Latitude2-Latitude1)
      p2=.5*Beta2*(DSIN(2.*Latitude2)-DSIN(2.*Latitude1))
      p3=.25*Beta4*(DSIN(4.*Latitude2)-DSIN(4.*Latitude1))
      p4=1./6.*Beta6*(DSIN(6.*Latitude2)-DSIN(6.*Latitude1))
C
      ArcMer=p1-p2+p3-p4
C
      END
      FUNCTION DdDmd(Degres_Decimaux)
c,<901226.1130>
C     ==============
C
C     Conversion Deg. decimaux --> Deg. Min. Decimales (DDDMM.MM)
C
      Implicit None
C
      INTEGER Nb_Degres
C
      REAL*8 DdDmd,Degres_Decimaux
C
      Nb_Degres=Degres_Decimaux
C
      DdDmd=Nb_Degres*100.+(Degres_Decimaux-Nb_Degres)*60.
C
      END
      FUNCTION DdDms(Degres_Decimaux)
c,<901226.1130>
C     ==============
C
C     Conversion des Degres Decimaux en Degres Minutes Secondes sous la
C         forme DDDMM.SEC
C
      Implicit None
C
      INTEGER Minute,Nb_Degres
C
      REAL*8 DdDms,Degres_Decimaux,Reste,Seconde
C
      Nb_Degres=Degres_Decimaux
C
      Reste=(Degres_Decimaux-Nb_Degres)*60.
      Minute=Reste
C
      Seconde=Reste-Minute
C
      DdDms=Nb_Degres*100.+Minute+Seconde/100.*60.
C
      END
      FUNCTION DdRad(Degres_Decimaux)
c,<901226.1130>
C     ==============
C
C     Conversion de Degres_Decimaux en Radian
C
      Implicit none
C
      REAL*8 DdRad,Degres_Decimaux
C
      REAL*8 Conv_Deg_Rad,Pi,Conv_Grad_Rad
C
      COMMON /Constante/Conv_Deg_Rad,Pi,Conv_Grad_Rad
C
C     Conv_Deg_Rad=DATAN(1.D0)/45.
C
      DdRad=Conv_Deg_Rad*Degres_Decimaux
C
      END
      FUNCTION DmdDd(Deg_Mn_Decimale)
c,<901226.1130>
C     =============
C
C     Conversion Deg. Min. Decimales (DDDMM.MM) --> Deg. Decimaux
C
      Implicit None
C
      INTEGER Nb_Degres
C
      REAL*8 Deg_Mn_Decimale,DmdDd,Mn_Decimale
C
      Nb_Degres=Deg_Mn_Decimale/100.
      DmdDd=Nb_Degres
C
      Mn_Decimale=Deg_Mn_Decimale-Nb_Degres*100.
      DmdDd=DmdDd+Mn_Decimale/60.
C
      END
      FUNCTION DmdRad(Deg_Mn_Decimale)
c,<901226.1130>
C     ===============
C
C     Conversion Deg. Min. Decimales (DDDMM.MM) --> Radian
C
      Implicit None
C
      REAL*8 Deg_Mn_Decimale,DmdDd,DmdRad
C
      REAL*8 Conv_Deg_Rad,Pi,Conv_Grad_Rad
C
      COMMON /Constante/Conv_Deg_Rad,Pi,Conv_Grad_Rad
C
C     Conv_Deg_Rad=DATAN(1.D0)/45.
C
      DmdRad=DmdDd(Deg_Mn_Decimale)*Conv_Deg_Rad
C
      END
      FUNCTION DmsDd(Deg_Mn_Sec)
c,<901226.1130>
C     ==============
C
C     Conversion Deg Min Sec (DDDNb_Minutes.Seconde) --> Deg Decimaux
C
      Implicit None
C
      INTEGER Nb_Minutes,Nb_Degres
C
      REAL*8 Deg_Mn_Sec,DmsDd,Mn_Decimale,Seconde
C
      Nb_Degres=Deg_Mn_Sec/100.
      DmsDd=Nb_Degres
C
      Mn_Decimale=Deg_Mn_Sec-Nb_Degres*100.
      Nb_Minutes=Mn_Decimale
C
      Seconde=Mn_Decimale-Nb_Minutes
C
      Mn_Decimale=Nb_Minutes+Seconde*100./60.
      DmsDd=DmsDd+Mn_Decimale/60.
C
      END
      SUBROUTINE GeLam(Longitude,Latitude,X_Lambert,Y_Lambert,
C     ================
     *    No_Systeme)
c,<901226.1130>
C
C     Conversion Geographique (Radians)  -  Lambert (Metres)
C
C     CALL InitG(No_Systeme) initialise le systeme
C
      Implicit None
C
      INTEGER No_Systeme,No_Systeme_Ref
C
      REAL*8 Constante_Lamb,Delta_Long_Ref,Lat_Croissante,
     *    Lat_Croiss_Ref,Lat_Conforme,Latitude,Latitude_Ref,Longitude,
     *    Longitude_Ref,R,R0,Sinus_Lat_Ref,X_Lambert_Ref,X_Lambert,
     *    Y_Lambert_Ref,Y_Lambert
C
      COMMON /GLAM/Sinus_Lat_Ref,X_Lambert_Ref,Y_Lambert_Ref,R0,
     *    Longitude_Ref,Lat_Croiss_Ref,Delta_Long_Ref,No_Systeme_Ref
C
      IF(No_Systeme.NE.No_Systeme_Ref)CALL InitG(No_Systeme)
C
      Constante_Lamb=(Longitude-Longitude_Ref-Delta_Long_Ref)*
     *    Sinus_Lat_Ref
C
      CALL LACRE(Latitude,Lat_Conforme,Lat_Croissante)
C
      R=R0*DEXP((Lat_Croiss_Ref-Lat_Croissante)*Sinus_Lat_Ref)
C
      X_Lambert=X_Lambert_Ref+R*DSIN(Constante_Lamb)
      Y_Lambert=Y_Lambert_Ref+R0-R*DCOS(Constante_Lamb)
C
      END
      SUBROUTINE geotm(Ellips,Param_Projection,Longitude_Rad,
C     ================
     *    Latitude_Rad,X_Cart,Y_Cart)
c,<901226.1130>
C
C->   geographiques -> transverse Mercator
C->   Ellips(2) - a0,e2   : parametres de l'Ellipsoide
C->   Param_Projection(5) : cml,fi0,rk0 : long,lat et facteur d'echelle
C                                         point central
C->                         x0,y0       : constantes point central
C->   Longitude1,Latitude_Rad --> X_Cart,Y_Cart
C->   source j. pagarete
C
      Implicit None
C->
      REAL*8 a0,ArcMer,c,c2,c4,c6,e2,Ellips(2),Latitude_Rad,Longitude1,
     *    Longitude2,Longitude_Rad,Param_Projection(5),qr,qr2,qr3,rk1,
     *    rk2,rk3,rk4,rk5,rk6,rn,rp,s,sigma,t,t2,t4,t6,w,X_Cart,Y_Cart
C
      a0=Ellips(1)
      e2=Ellips(2)
      Longitude1=Longitude_Rad-Param_Projection(1)
      sigma=ArcMer(Ellips,Param_Projection(2),Latitude_Rad)
      s=DSIN(Latitude_Rad)
      w=DSQRT(1.-e2*s*s)
      rn=a0/w
      qr=(w*w)/(1.-e2)
      qr2=qr*qr
      qr3=qr2*qr
      c=DCOS(Latitude_Rad)
      rp=rn*c
      t=s/c
      Longitude2=Longitude1*Longitude1
      c2=c*c*Longitude2
      c4=c2*c2
      c6=c4*c2
      t2=t*t
      t4=t2*t2
      t6=t4*t2
      rk1=qr-t2
      rk2=rk1+4.*qr2
      rk3=qr*(14.-58.*t2)+40.*t2+t4-9.
      rk4=(8.*qr3*(11.-24.*t2)-28.*qr2*(1.-6.*t2)+qr*(1.-32.*t2)-2.*t2)*
     *    qr+t4
      rk5=61.-479.*t2+179.*t4-t6
      rk6=1385.-3111.*t2+543.*t4-t6
      Y_Cart=sigma+.5*rp*Longitude2*s*(1.+c2*rk2/12.+c4*rk4/360.+c6*rk6/
     *    20160.)
      Y_Cart=Param_Projection(3)*Y_Cart+Param_Projection(5)
      X_Cart=Param_Projection(3)*rp*Longitude1*(1.+c2*rk1/6.+c4*rk3/120.
     *    +c6*rk5/5040.)+Param_Projection(4)
C
      end
      SUBROUTINE Geo_Glr(Longitude_Rad,Latitude_Rad,X_Glr,Y_Glr)
c,
C     ==================
c    *    <901226.1130>
C
C-    geographiques (Radians) -> Gauss Laborde Reunion (Metres)
C
      Implicit None
C
      LOGICAL Init
C
      REAL*8 DmdRad,Ellips(2),Latitude_Rad,Longitude_Rad,
     *    Param_Projection(5),RadDmd,X_Glr,Y_Glr
C
C-    Ellipsoide international a0,e2
C
      DATA Ellips/6378200.d0,0.00669342162d0/
C
C-    Projection Gauss laborde Reunion
C     long - lat du centre de Projection facteur d'echelle + x0,y0
C
      DATA Param_Projection/5532.d0,-2107.d0,1.d0,1.6d5,5.d4/
C
      Param_Projection(1)=DmdRad(Param_Projection(1))
      Param_Projection(2)=DmdRad(Param_Projection(2))
C
      CALL geotm(Ellips,Param_Projection,Longitude_Rad,Latitude_Rad,
     *    X_Glr,Y_Glr)
C
      Param_Projection(1)=RadDmd(Param_Projection(1))
      Param_Projection(2)=RadDmd(Param_Projection(2))
C
      END
      SUBROUTINE Glr_Geo(X_Glr,Y_Glr,Longitude_Rad,Latitude_Rad)
c,
C     ==================
c    *    <901226.1130>
C
C-    Gauss Laborde Reunion (Metres) -> geographiques (Radians)
C
      Implicit None
C
      REAL*8 DmdRad,Ellips(2),Latitude_Rad,Longitude_Rad,
     *    Param_Projection(5),RadDmd,X_Glr,Y_Glr
C
C-    Ellipsoide international a0,e2
C
      DATA Ellips/6378200.d0,0.00669342162d0/
C
C-    Projection Gauss laborde Reunion
C     long - lat du centre de Projection facteur d'echelle + x0,y0
C
      DATA Param_Projection/5532.d0,-2107.d0,1.d0,1.6d5,5.d4/
C
      Param_Projection(1)=DmdRad(Param_Projection(1))
      Param_Projection(2)=DmdRad(Param_Projection(2))
C
      CALL tmgeo(Ellips,Param_Projection,X_Glr,Y_Glr,Longitude_Rad,
     *    Latitude_Rad)
C
      Param_Projection(1)=RadDmd(Param_Projection(1))
      Param_Projection(2)=RadDmd(Param_Projection(2))
C
      END
      FUNCTION GradRad(Grade)
c,<901226.1130>
C     ================
C
C     Conversion de Grades en Radian
C
      Implicit none
C
      REAL*8 Grade,GradRad
C
      REAL*8 Conv_Deg_Rad,Pi,Conv_Grad_Rad
C
      COMMON /Constante/Conv_Deg_Rad,Pi,Conv_Grad_Rad
C
C     Conv_Deg_Rad=DATAN(1.D0)/50.
C
      GradRad=Conv_Grad_Rad*Grade
C
      END
      SUBROUTINE GUTM(Longitude,Latitude,X_UTM,Y_UTM,No_Fuseau_UTM)
c,
C     ===============
c    *    <901226.1130>
C
C     Conversion Geographique (Radian)  -  UTM (metres)
C
C     Longitude et Latitude en Radians.
C     No_Fuseau_UTM=(Longitude+186.)/6.
C     Utilise le Sspg ArcMe
C
      Implicit None
C
      INTEGER No_Fuseau_UTM
C
      REAL*8 ArcMe,Constante_UTM,Cosinus,DdRad,ECOXI,Eta,Eprim,GN,
     *    Latitude,Longitude,Longitud_Relativ,X_UTM,XI,Y_UTM
C
      DATA Constante_UTM/6399936.608D0/,Eprim/0.0822688896073373D0/
C
      Longitud_Relativ=Longitude-DdRad(No_Fuseau_UTM*6.D0-183.D0)
      Cosinus=DCOS(Latitude)
C
      XI=Cosinus*DSIN(Longitud_Relativ)
      XI=0.5*DLOG((1.+XI)/(1.-XI))
C
      Eta=DATAN(DSIN(Latitude)/(Cosinus*DCOS(Longitud_Relativ)))-
     *    Latitude
      GN=Constante_UTM/DSQRT(1.+(Eprim*Cosinus)**2)
      ECOXI=(Eprim*Cosinus*XI)**2
C
      X_UTM=500000.+0.9996*GN*XI*(1.+ECOXI/6.)
C
      Y_UTM=0.9996*(GN*Eta*(1.+ECOXI/2.)+ArcMe(Latitude))
C
      END
      SUBROUTINE InitG(No_Systeme)
c,<901226.1130>
C     ================
C
C     Initialise les constantes de conversion Geographiques - Lambert
C         No_Systeme de 1 a 10
C
      Implicit None
C
      INTEGER No_Systeme,No_Systeme_Ref
  
      REAL*8 DdRad,Delta_Long_Ref,GradRad,Lat_Croiss_Ref,
     *    Lat_Conforme_Ref,Latitude_Ref,Longitude_Ref,Sinus_Lat_Ref,R0,
     *    Param_Projection(6,10),X_Lambert_Ref,Y_Lambert_Ref
C
      COMMON /GLAM/Sinus_Lat_Ref,X_Lambert_Ref,Y_Lambert_Ref,R0,
     *    Longitude_Ref,Lat_Croiss_Ref,Delta_Long_Ref,No_Systeme_Ref
C
      DATA Param_Projection /
C Syst  Latitude_Ref  R0              Longitude_Ref   X    Y   DELT
C                                                   Lambert_Ref
C
C    1                  France   Nord
C
     *  55.D0, 5457616.68D0, 0.D0,     0.6D6,        0.2D6, 2.3372292D0,
C
C    2                  France   Centre
C
     *  52.D0, 5999695.77D0, 0.D0,     0.6D6,        0.2D6, 2.3372292D0,
C
C    3                  France   Sud
C
     *  49.D0, 6591905.08D0, 0.D0,     0.6D6,        0.2D6, 2.3372292D0,
C
C    4                  France   Corse
C
     *46.85D0, 7053300.18D0, 0.D0, 234.538D0, 185861.369D0, 2.3372292D0,
  
C
C    5                  Nord     Maroc
C
     *  37.D0,9716290.594D0,-6.D0,     0.5D6,        0.3D6,        0.D0,
C
C    6                  Sud      Maroc
C
     *  33.D0,11187308.682D0,-6.D0,    0.5D6,        0.3D6,        0.D0,
C
C    7                  Nord     Algerie
C
     *  40.D0,8785951.541D0, 3.D0,      5.D5,         3.D5,        0.D0,
C
C    8                  Sud      Algerie
C
     *  37.D0,9716290.594D0, 3.D0,      5.D5,         3.D5,        0.D0,
C
C    9                  Nord     Tunisie
C
     *  40.D0,8785951.541D0,11.D0,      5.D5,         3.D5,        0.D0,
C
C   10                  Sud      Tunisie
C
     *  37.D0,9716290.594D0,11.D0,      5.D5,         3.D5,        0.D0/
C
      No_Systeme_Ref=No_Systeme
      X_Lambert_Ref=Param_Projection(4,No_Systeme)
      Y_Lambert_Ref=Param_Projection(5,No_Systeme)
      R0=Param_Projection(2,No_Systeme)
C
      Latitude_Ref=GradRad(Param_Projection(1,No_Systeme))
      Longitude_Ref=GradRad(Param_Projection(3,No_Systeme))
      Delta_Long_Ref=DdRad(Param_Projection(6,No_Systeme))
      Sinus_Lat_Ref=DSIN(Latitude_Ref)
C
      CALL LACRE(Latitude_Ref,Lat_Conforme_Ref,Lat_Croiss_Ref)
C
      END
      SUBROUTINE LACRE(Latitude,Lat_Conforme,Lat_Croissante)
c,
C     ================
c     *    <901226.1130>
C
C     Sspg de calcul de Latitude Conforme et de Latitude Croissante
C
C     Calcul de la Latitude conforme Lat_Conforme
C     Calcul de la Latitude Croissante Ellipsoidique Lat_Croissante
C     Donnees et Resultats en Radians
C
      Implicit None
C
      REAL*8 Constante,Lat_Conforme,Lat_Croissante,Latitude,Teta,Valeur
C
      DATA Constante/0.0824832568D0/
C
      Valeur=0.5*Latitude
      Teta=DTAN(Valeur)
      Valeur=2.*Constante*Teta/(1.+Teta**2)
      Valeur=0.5*Constante*DLOG((1.+Valeur)/(1.-Valeur))
      Lat_Croissante=DLOG((1.+Teta)/(1.-Teta))-Valeur
      Valeur=DTANH(0.5*Valeur)
      Lat_Conforme=2.*DATAN((Teta-Valeur)/(1.-Valeur*Teta))
C
      END
      SUBROUTINE LamGe(X_Lambert,Y_Lambert,Longitude,Latitude,
C     ================
     *    No_Systeme)
c,<901226.1130>
C
C     Conversion Lambert (Metres)  -  Geographiques (Radians)
C     CALL InitG(No_Systeme)  Initialise le systeme
C     Ellipsoide de CLARKE IGN
C
      Implicit None
C
      INTEGER No_Systeme,No_Systeme_Ref
C
      REAL*8 Delta_Long_Ref,Latitude,Lat_Crois_Ref,Lat_Croissante,
     *    Longitude,Longitude_Ref,R,R0,RLAC,Sinus_Lat_Ref,X_Lambert,
     *    X_Lambert_Ref,Y_Lambert,Y_Lambert_Ref
C
      COMMON /GLAM/Sinus_Lat_Ref,X_Lambert_Ref,Y_Lambert_Ref,R0,
     *    Longitude_Ref,Lat_Crois_Ref,Delta_Long_Ref,No_Systeme_Ref
C
      IF(No_Systeme.NE.No_Systeme_Ref)CALL InitG(No_Systeme)
C
      Longitude=X_Lambert-X_Lambert_Ref
      Latitude=R0-(Y_Lambert-Y_Lambert_Ref)
      R=Latitude*DSQRT(1.+(Longitude*Longitude)/(Latitude*Latitude))
      Longitude=Longitude_Ref+DATAN(Longitude/Latitude)/Sinus_Lat_Ref
      Lat_Croissante=Lat_Crois_Ref+DLOG(R0/R)/Sinus_Lat_Ref
      Lat_Croissante=DTANH(0.5D0*Lat_Croissante)
C
      Latitude=RLAC(Lat_Croissante)
      Longitude=Longitude+Delta_Long_Ref
C
      END
      FUNCTION RadDd(Radian)
c,<901226.1130>
C     ==============
C
C     Conversion de radians en degres decimaux
C
      Implicit None
C
      REAL*8 RadDd,Radian
C
      REAL*8 Conv_Deg_Rad,Pi,Conv_Grad_Rad
C
      COMMON /Constante/Conv_Deg_Rad,Pi,Conv_Grad_Rad
C
C     Conv_Deg_Rad=DATAN(1.D0)/45.
C
      RadDd=Radian/Conv_Deg_Rad
C
      END
      FUNCTION RadDmd(Radian)
c,<901226.1130>
C     ===============
C
C     Conversion de Radians en Degres Minutes decimales
C
      Implicit None
C
      REAL*8 DdDmd,RadDmd,Radian
C
      REAL*8 Conv_Deg_Rad,Pi,Conv_Grad_Rad
C
      COMMON /Constante/Conv_Deg_Rad,Pi,Conv_Grad_Rad
C
C     Conv_Deg_Rad=datan(1.d0)/45.
C
      RadDmd=DdDmd(Radian/Conv_Deg_Rad)
C
      END
      FUNCTION RLAC(Tangente_Lat_xxx)
c,<901226.1130>
C     =============
C
C     Fonction inverse du sspg LACRE
C     Calcul retour de latitude croissante ou de latitude conforme
C
C         Tangente_Lat_xxx=TAN(Lat_Conforme/2.)=TANH(Lat_Croissante/2.)
C
C     Resultat : Latitude en Radians
C
      Implicit None
C
      INTEGER Nb
C
      REAL*8 Constante,Q,RLAC,Tangente_Lat_xxx,Teta
C
      DATA Constante/0.0824832568D0/
C
      Teta=Tangente_Lat_xxx
      DO Nb=1,3
          Q=2.*Constante*Teta/(1.+Teta**2)
          Q=DTANH(0.25*Constante*DLOG((1.+Q)/(1.-Q)))
          Teta=(Tangente_Lat_xxx+Q)/(1.+Q*Tangente_Lat_xxx)
      ENDDO
C
      RLAC=2.*DATAN(Teta)
C
      END
      SUBROUTINE tmgeo(Ellips,Param_Projection,X_Cart,Y_Cart,
C     ================
     *    Longitude_Rad,Latitude_Rad)
c,<901226.1130>
C
C->   transverse Mercator -> geogrphiques
C->   Ellips - a0,e2 - parametres de l'Ellipsoide
C->   Param_Projection - cm0,fi0,rk0,x0,y0
C-    coordonnes du point central et facteur d'echelle
C->   X_Cart,Y_Cart --> Longitude_Rad,Latitude_Rad
C->          metres --> radians
C
      Implicit None
C
      INTEGER No
C     INTEGER LU_1
C
      REAL*8 a0,c,dLatitude,dLongitude,dist,dist2,dx,dy,e2,Ellips(2),
     *    Latitude_Rad,Longitude_Rad,prec2,Param_Projection(5),qr,rn,rp,
     *    s,w,X_Cart,xe,Y_Cart,ye
C
C     COMMON /LUecran/LU_1
C
      a0=Ellips(1)
      e2=Ellips(2)
      prec2=0.0001
      No=0
C
C->   on obtient une approximation de Longitude_Rad et Latitude_Rad par
C         une formule standard
C
      CALL tmgeo1(Ellips,Param_Projection,X_Cart,Y_Cart,Longitude_Rad,
     *    Latitude_Rad)
   10 CALL geotm(Ellips,Param_Projection,Longitude_Rad,Latitude_Rad,xe,
     *    ye)
      dx=X_Cart-xe
      dy=Y_Cart-ye
C     IF(No.EQ.0)WRITE(LU_1,*)'dx dy --> ',dx,dy
      s=DSIN(Latitude_Rad)
      w=DSQRT(1.-e2*s*s)
      rn=a0/w
      qr=(w*w)/(1.-e2)
      c=DCOS(Latitude_Rad)
      rp=rn*c
      dLatitude=dy*qr/rn
      dLongitude=dx/rp
      Latitude_Rad=Latitude_Rad+dLatitude
      Longitude_Rad=Longitude_Rad+dLongitude
      No=No+1
      IF(No.LE.20)then
          dist2=dx*dx+dy*dy
          if(dist2.GT.prec2)GO TO 10
C     ELSE
C         WRITE(LU_1,*)' tmgeo : pas de convergence '
      ENDIF
C
      END
      SUBROUTINE tmgeo1(Ellips,Param_Projection,X_Cart,Y_Cart,
C     =================
     *    Longitude_Rad,Latitude_Rad)
c,<901226.1130>
C
C->   transverse Mercator --> geographiques
C->   formule directe approchee (utilisee dans tmgeo)
C->   l'erreur n'excede pas 2 metres dans un fuseau de +- 10 deg
C->   adapte de utmgr pour des Ellipsoides variables
C->   a0,e2 - parametres de l'Ellipsoide
C->   cm0,fi0,rk0 - long,lat point central et facteur d'echelle
C->   X_Cart,Y_Cart --> Longitude_Rad,Latitude_Rad
C
      Implicit None
C
      real*8 a0,a1,a2,ArcMer,cofip,cophi,e2,ecofi,ecxn2,Ellips(2),eprim,
     *    eprim2,eta,expxi,gn,gphi,Latitude_Rad,Longitude_Rad,phi,phipr,
     *    Param_Projection(5),siphi,vm2,x,X_Cart,xi,y,Y_Cart
c
      EQUIVALENCE (cofip,ecxn2),(gphi,expxi),(vm2,ecofi)
C
C-    gain de place ?
C
      a0=Ellips(1)
      e2=Ellips(2)
      eprim2=e2/(1.-e2)
      eprim=DSQRT(eprim2)
      a1=a0/DSQRT(1.-e2)
      a2=a0*DSQRT(1.+eprim2)
      y=Y_Cart-Param_Projection(5)
      y=y/Param_Projection(3)
      x=(X_Cart-Param_Projection(4))/Param_Projection(3)
      phi=y/a2+Param_Projection(2)
      siphi=DSIN(phi)
      cophi=DCOS(phi)
      ecofi=eprim*cophi
      gn=a1/DSQRT(1.+ecofi**2)
      y=y-ArcMer(Ellips,Param_Projection(2),phi)
      xi=x/gn
      ecxn2=(ecofi*xi)**2
      xi=xi*(1.-ecxn2/6.)
      eta=(y/gn)*(1.-ecxn2/2.)
      phipr=phi+eta
      cofip=DCOS(phipr)
      expxi=DEXP(xi)
      Longitude_Rad=DATAN((expxi**2-1.)/(2.*expxi*cofip))
      gphi=DATAN(DCOS(Longitude_Rad)*DSIN(phipr)/cofip)
      vm2=1.+ecofi**2-1.5*eprim*siphi*ecofi*(gphi-phi)
      Longitude_Rad=Longitude_Rad+Param_Projection(1)
      Latitude_Rad=phi+vm2*(gphi-phi)
C
      END
      SUBROUTINE TRANSF(X_Init,Y_Init,X_Final,Y_Final,No_Systeme_Init,
C     =================
     *    No_Systeme_Final,No_Ref_Init,No_Ref_Final)
c,<1.1>
C
C     Conversion de coordonnees
C              X_Init,Y_Init      en       X_Final,Y_Final
C         No_Systeme_Init  -  No_Systeme_Final
C              1      2       3       4       5       6
C             UTM    DMS     DD      DMD     LAM     GLR
C     No_Ref_Init,No_Ref_Final Parametres supplementaires (No Lambert
C         ou Fuseau UTM)
C     Dans le cas des Geographiques : Deplacement du meridien origine
C              1      2       3       4       5
C             GRNW   PARIS   ROME    .....    ......
C
      IMPLICIT None
C
      INTEGER LU_1,No_Ref_Final,No_Ref_Init,No_Reference,
     *    No_Systeme_Final,No_Systeme_Init
C
      REAL*8 DdDmd,DdDms,DdRad,DmdDd,DmsDd,Longitude_Ref(5),RadDd,
     *    X_Init,X_Final,X_Inter,Y_Init,Y_Final,Y_Inter
C
      REAL*8 Conv_Deg_Rad,Pi,Conv_Grad_Rad
C
      COMMON /Constante/Conv_Deg_Rad,Pi,Conv_Grad_Rad
C
      DATA No_Reference/0/,Longitude_Ref/0.D0,2.3372292D0,12.452222D0,
     *    0.D0,0.D0/
C
C     Initialisation unique via le Common Constante sinon la mettre dans
C         TRANSG ou TRANSF suivant le programme :
C             GEODIS et PARNU (GEO) : TRANSG
C             COORD  et HYPO        : TRANSF
C
      Pi=4.*DATAN(1.D0)
      Conv_Deg_Rad=Pi/180.
      Conv_Grad_Rad=Pi/200.
C
      IF(No_Systeme_Init.EQ.No_Systeme_Final)GO TO 90
   10 GO TO (20,30,40,50,60,70),No_Systeme_Init
C
      WRITE(*,'("Transf : Systeme initial indefini")')
      RETURN
C
   20 CALL UTMGR(X_Init*1000.,Y_Init*1000.,X_Inter,Y_Inter,No_Ref_Init)
C
      GO TO 80
C
   30 X_Final=DmsDd(X_Init)+Longitude_Ref(No_Ref_Init)
      Y_Final=DmsDd(Y_Init)
      GO TO 100
C
   40 X_Final=X_Init+Longitude_Ref(No_Ref_Init)
      Y_Final=Y_Init
      GO TO 100
C
   50 X_Final=DmdDd(X_Init)+Longitude_Ref(No_Ref_Init)
      Y_Final=DmdDd(Y_Init)
      GO TO 100
C
   60 CALL LamGe(1000.*X_Init,1000.*Y_Init,X_Inter,Y_Inter,No_Ref_Init)
      GO TO 80
C
   70 CALL Glr_Geo(1000.*X_Init,1000.*Y_Init,X_Inter,Y_Inter)
C
   80 X_Final=RadDd(X_Inter)
      Y_Final=RadDd(Y_Inter)
      GO TO 100
C
   90 IF(No_Ref_Init.NE.No_Ref_Final)GO TO 10
      X_Final=X_Init
      Y_Final=Y_Init
      RETURN
C
  100 GO TO (110,120,130,140,150,160),No_Systeme_Final
C
      WRITE(*,'("Transf :  Systeme final indefini")')
      RETURN
C
  110 IF(No_Ref_Final.NE.0)No_Reference=No_Ref_Final
      IF(No_Reference.EQ.0)No_Reference=(X_Final+186.)/6.
      No_Ref_Final=No_Reference
C
      CALL GUTM(DdRad(X_Final),DdRad(Y_Final),X_Inter,Y_Inter,
     *    No_Reference)
      GO TO 170
C
  120 X_Final=DdDms(X_Final-Longitude_Ref(No_Ref_Final))
      Y_Final=DdDms(Y_Final)
      RETURN
C
  130 X_Final=X_Final-Longitude_Ref(No_Ref_Final)
      RETURN
C
  140 X_Final=DdDmd(X_Final-Longitude_Ref(No_Ref_Final))
      Y_Final=DdDmd(Y_Final)
      RETURN
C
  150 CALL GeLam(DdRad(X_Final),DdRad(Y_Final),X_Inter,Y_Inter,
     *    No_Ref_Final)
      GO TO 170
C
  160 CALL Geo_Glr(DdRad(X_Final),DdRad(Y_Final),X_Inter,Y_Inter)
C
  170 X_Final=X_Inter/1000.
      Y_Final=Y_Inter/1000.
C
      END
      SUBROUTINE UTMGR(X_UTM,Y_UTM,Longitude,Latitude,No_Fuseau_Ref)
c,
C     ================
c    *    <901226.1130>
C
C     Conversion Coordonnees UTM en Geographiques
C
C     Donnees en Metres , Resultats en Radians
C     Origine des longitudes : le meridien international
C     Definition normale de la projection mais Y_UTM est negatif dans
C         l'hemisphere sud , prendre ABS(Y_UTM) pour avoir le NORTHING
C         habituel
C
C     Longitude_Ref est la longitude du meridien central du fuseau
C
      Implicit None
C
      INTEGER No_Fuseau_Ref
C
      REAL*8 ArcMe,C,COFIP,CoPhi,DdRad,ECOFI,ECXN2,Eprim,Eta,EXPXI,GN,
     *    GPhi,Latitude,Longitude,Longitude_Ref,Phi,PhiPr,SiPhi,VM2,X,
     *    X_UTM,XI,Y,Y_UTM
C
      EQUIVALENCE(COFIP,ECXN2),(GPhi,EXPXI),(VM2,ECOFI)
C
      DATA C/6399936.608D0/,Eprim/0.0822688896073373D0/
C
      Longitude_Ref=DdRad(No_Fuseau_Ref*6.D0-183.)
      Y=DABS(Y_UTM)
C
C     IF(Y_UTM.LT.0.0)Y=-Y_UTM-10000000.
C
      Y=Y/0.9996
      Phi=Y/6366197.724
      SIPhi=DSIN(Phi)
      COPhi=DCOS(Phi)
      ECOFI=Eprim*COPhi
      GN=C/DSQRT(1.+ECOFI**2)
      X=(X_UTM-500000.)/0.9996
      Y=Y-ArcMe(Phi)
      XI=X/GN
      ECXN2=(ECOFI*XI)**2
      XI=XI*(1.-ECXN2/6.)
      Eta=(Y/GN)*(1.-ECXN2/2.)
      PhiPR=Phi+Eta
      COFIP=DCOS(PhiPR)
      EXPXI=DEXP(XI)
      Longitude=DATAN((EXPXI**2-1.)/(2.*EXPXI*COFIP))
      GPhi=DATAN(DCOS(Longitude)*DSIN(PhiPR)/COFIP)
      VM2=1.+ECOFI**2-1.5*Eprim*SIPhi*ECOFI*(GPhi-Phi)
      Longitude=Longitude+Longitude_Ref
      Latitude=Phi+VM2*(GPhi-Phi)
      IF(Y_UTM.LT.0.)Latitude=-Latitude
C
      END
