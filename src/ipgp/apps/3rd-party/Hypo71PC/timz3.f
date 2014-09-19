      function timz3
     + (xx,y,z1,z2,v0,g,vr,h,gx,gy,dtdz,dtdv,dtdg,dtdr,dtdh)
      implicit real*8 (a-h,o-z)
      dasinh(x)=dlog(x+dsqrt(x*x+1.))
c--- z1 doit etre dans la 1ere couche
      x=dsqrt(xx**2+y**2)
	if(x.eq.0.d0) x=0.005
	gx=xx/x
	gy=y/x
      gmin=0.005
      v1=v0+g*z1
      v2=v0+g*z2
      vh=v0+g*h
      tir=0.
      if(z2.gt.h) then
c---       recherche iterative de p
      sph=1/vh**2
      v12=v1**2
      vh2=vh**2
            pa=0
            pb=1/vr
            p=(pa+pb)/2
            xx1=x-(h-z1)*(vh/vr)/dsqrt(1-(vh/vr)**2)
            if(xx1.lt.0.) xx1=x*0.8
            p=xx1/dsqrt(xx1**2+(z2-h)**2)/vr
1     sth=p*vh
      st1=p*v1
      cth=dsqrt(1-sth**2)
      ct1=dsqrt(1-st1**2)
      a1=(sth+st1)/(cth+ct1)
      str=p*vr
      ctr=dsqrt(1-str**2)
      xx1=(h-z1)*a1
            xd=str*(z2-h)/ctr+xx1
            if(dabs(xd-x).lt.0.001d0) goto 10
            if(x.lt.xd) then
                              pb=p
                              p=(p+pa)/2
                              else
                              pa=p
                              p=(pb+p)/2
            endif
            goto 1
10    continue
c---<   t   >
      tu2=(xx1**2+(h-z1)**2)/v1/vh
      tu=dsqrt(tu2)
      u=g/2*tu
      suup=dsqrt(u**2+1)
      tir=(x-xx1)/str/vr
      if(dabs(g).gt.gmin) then
            tir1=2/g*dasinh(u)
            tir=tir+tir1
            else
            tir=tir+tu
      endif
      timz3=tir
      tg=str/ctr
      cpc=cth+ct1
      asap=a1/(a1**2+1)
      saap=1/dsqrt(a1**2+1)
c---<   dtdx   >
      gx=p*gx
      gy=p*gy
c---<   dtdz   >
      dtdz=ctr/vr
c---<   dtdv   >
      dadv=p/cpc*(2+a1*(sth/cth+st1/ct1))
      dtdv=tu*(-(1/vh+1/v1)/2+asap*dadv)/suup -p*(h-z1)*dadv
c---<   dtdg   >
      dadg=p*(z1+h+a1*(z1*st1/ct1+h*sth/cth))/cpc
      dtdg=0
      if(dabs(g).gt.gmin) dtdg=(tu/suup-tir1)/g
      dtdg=dtdg+tu*(-(h/vh+z1/v1)/2+asap*dadg)/suup-p*(h-z1)*dadg
c---<   dtdr   >
      dtdr=-(z2-h)/vr**2/ctr
c---<   dtdh   >
      dadh=p*g/cpc*(1+a1*sth/cth)
      dtdh=-ctr/vr
     +       + tu*(-g/2/vh+asap*dadh)/suup -p*(a1+(h-z1)*dadh)
     +         +dsqrt((a1**2+1)/v1/vh)/suup-p*a1
      goto 119
      endif
      x1=(h-z1)*(v1+vh)/(dsqrt(vr**2-vh**2)+dsqrt(vr**2-v1**2))
      x2=(h-z2)*(v2+vh)/(dsqrt(vr**2-vh**2)+dsqrt(vr**2-v2**2))
c--- refractee dans la 1ere couche
            d2=x**2+(z2-z1)**2
            tu2=d2/v1/v2
            tu=dsqrt(tu2)
            u=g/2*tu
            a=dsqrt(u**2+1)
            tusa=tu/a
            dtdx=x/d2*tusa
      gx=gx*dtdx
      gy=gy*dtdx
            dtdz=((z2-z1)/d2-g/2/v2)*tusa
            dtdv=-(v1+v2)/v1/v2*tusa/2.
            if(dabs(g).gt.0.001d0) then
                              tir=2*dasinh(u)/g
                              dtdg=-tir/g+(1/g-(z1/v1+z2/v2)/2.)*tusa
            else
                              tir=tu
                              dtdg=-(z1/v1+z2/v2)/2.*tusa
            endif
            dtdr=0
            dtdh=0
      timz3=tir
      if(x1+x2.gt.x) goto 119
c--- refractee dans la 2eme couche
      p=1/vr
      st1=p*v1
      st2=p*v2
      sth=p*vh
      ct1=dsqrt(1.-st1*st1)
      ct2=dsqrt(1.-st2*st2)
      cth=dsqrt(1.-sth*sth)
c     if(g.ne.0.) then
c---<   t   >
c     tti1=dlog((1-cth)**2/(1-ct1)/(1-ct2)*v1*v2/vh**2)/g
c     tti2=-(ct1+ct2-2*cth)/g
c     ttii=tti1+tti2+x/vr
c---<   dt/dv0   >
c     if(g.ne.0.)dtdv1=(2*cth/vh-ct1/v1-ct2/v2)/g
c---<   dt/dg   >
c     dtdg=(-(tti1+tti2)+2*h/vh/cth-z1/v1/ct1-z2/v2/ct2
c    +  +(z1*v1/ct1+z2*v2/ct2-2*vh*h/cth)/vr**2)/g
c---<   dt/dvr   >
c     dtdr=
c    + (-2*vh**2/(1-cth)+v1**2/(1-ct1)+v2**2/(1-ct2))/vr**3/g
c    +     -x/vr**2
c     else
c     endif
c---<   t   >
      vv1=dsqrt(vh*v1)
      vv2=dsqrt(vh*v2)
      vhm1=(1/vh+1/v1)/2
      vhm2=(1/vh+1/v2)/2
      a1=(sth+st1)/(cth+ct1)
      a2=(sth+st2)/(cth+ct2)
      aap1=a1*a1+1
      aap2=a2*a2+1
      asap1=a1/aap1
      asap2=a2/aap2
      saap1=dsqrt(aap1)
      saap2=dsqrt(aap2)
      tu1=(h-z1)*saap1/vv1
      tu2=(h-z2)*saap2/vv2
      u1=g*tu1/2.
      u2=g*tu2/2.
      suu1=dsqrt(u1*u1+1)
      suu2=dsqrt(u2*u2+1)
      cpc1=cth+ct1
      cpc2=cth+ct2
      if(g.ne.0.) then
            t1=dasinh(u1)*2/g
            t2=dasinh(u2)*2/g
      else
            t1=tu1
            t2=tu2
      endif
      tir1a=t1+t2
      tir1b=-(x1+x2)/vr
      tir=tir1a+tir1b+x/vr
      if(tir.gt.timz3) goto 119
c---<   dt/dx   >
      dtdx=p
      gx=p*gx
      gy=p*gy
c---<   dt/dz   >
      dtdz=-ct2/v2
c---< dt/dh   >
      dtdh=2*cth/vh
c---<   dt/dv0   >
      dadv1= p * (2 + (sth/cth + st1/ct1 )*a1 )/cpc1
      dadv2= p * (2 + (sth/cth + st2/ct2 )*a2 )/cpc2
      dtdv=-tu1*(vhm1-asap1*dadv1)/suu1 -tu2*(vhm2-asap2*dadv2)/suu2
     +      - p*dadv1*(h-z1) - p*dadv2*(h-z2)
c---<   dt/dvr   >
      dadr1= -a1*p*(1+(sth**2/cth+st1**2/ct1)/cpc1)
      dadr2= -a2*p*(1+(sth**2/cth+st2**2/ct2)/cpc2)
      dtdr=tu1*asap1*dadr1/suu1+tu2*asap2*dadr2/suu2
     +     -p*dadr1*(h-z1) -p*dadr2*(h-z2) - (x-x1-x2)*p**2
c---<   dt/dg    >
      dadg1= p*(h+z1+a1*(h*sth/cth+z1*st1/ct1))/cpc1
      dadg2= p*(h+z2+a2*(h*sth/cth+z2*st2/ct2))/cpc2
      dtdg= - tu1*((h/vh+z1/v1)/2-asap1*dadg1)/suu1
     +      - tu2*((h/vh+z2/v2)/2-asap2*dadg2)/suu2
     +     -p*dadg1*(h-z1) - p*dadg2*(h-z2)
      if(g.ne.0.) then
            dtdg1=(tu1/suu1-t1)/g
            dtdg=dtdg+dtdg1
      endif
      timz3=tir
119	continue
      return
      end

