/*

  Original Author: Manfred Baer, Swiss Seismological Service

  picker_ppick.f -- translated by f2c (version 20061008).

  Manually modified in 2010 by Stefan Heimers,
  will work without libf2c.


*/

#define SEISCOMP_COMPONENT Picker

#include <stdlib.h>
#include <math.h>
#include <iostream>

#include <seiscomp3/logging/log.h>
#include <seiscomp3/processing/picker/bk.h>


int preset(double *rbuf, int *n, double *old, double *y2,
	   double *yt, double *sumx, double *sumx2, double *sdev, int *nsum, int
	   *itar, int *ptime, int *preptime, char *pfm, int *ipkflg,
	   double *samplespersec){

  static int i;
  static double yy2, ysv, yyt;

  ysv = rbuf[0];
  *old = ysv;
  *sumx = ysv;
  *y2 = ysv * ysv;
  *yt = 0.f;
  for (i = 1; i < *n; ++i) {
    yy2 = rbuf[i];
    yyt = (yy2 - ysv) * *samplespersec;
    ysv = yy2;
    *sumx += ysv;
    *y2 += yy2 * yy2;
    *yt += yyt * yyt;
  }
  *sdev = sqrt(*n * *y2 - *sumx * *sumx) / (*n * *n);
  *sumx /= *n;
  *sumx2 = 0.f;
  *nsum = 0;
  *itar = 0;
  *ptime = 0;
  *preptime = 0;
  pfm[0]=' ';
  *ipkflg = 0;
  return 0;
} /* preset */





int ppick(double *reltrc, double *trace, int npts, double *
	thrshl1, double *thrshl2, int *tdownmax, int *tupevent, int
	*ipkflg, int *uptime, int *ptime, int *pamp, int *
	pamptime, int *preptime, int *prepamp, int *prepamptime,
	int *ifrst, int *noise, int *noisetime, int *signal,
	int *signaltime, int *test, int *nanf, int *nend,
	int *skip, int *prset, int *pickduration,
	double *samplespersec, char *pfm){


  //SEISCOMP_DEBUG("ppick() started");

  static int realdown, i__;
  static double y2, yt;
  static int picklength, amp, num;
  static double sum, ssx, rda2, rdi2, ssx2, edat, mean, edev, rdif;
  static int iamp;
  static double rdat;
  static int itar;
  static double sdev;
  static int itrm;
  static double omega;
  static int dtime, p_dur__, realup;
  static double rawold;

  static int nstart, end_dur__, amptime;

  /* ==================================================================== */

  /* p-picker routine by m. baer, schweizer. erdbebendienst */
  /*                              institut fur geophysik */
  /*                              eth-zurich */
  /*                              8093 zurich */
  /*                              tel. 01-377 26 27 */
  /*                              telex: 823480 eheb ch */

  /* see paper by m. baer and u. kradolfer: an automatic phase picker */
  /*                              for local and teleseismic events */
  /*                              bssa vol. 77,4 pp1437-1445 */
  /* ==================================================================== */

  /* this subroutine can be call successively for subsequent portions */
  /* of a seismogram. all necessary values are passed back to the calling */
  /* program. */
  /* for the initial call, the following variables have to be set: */

  /*      y2                    (e.g. = 1.0, or from any estimate) */
  /*      yt                    (e.g. = 1.0, or from any estimate) */
  /*      sdev                  (e.g. = 1.0) */
  /*      rawold                (e.g. = 0.0) */
  /*      tdownmax              (c.f. paper for possible values) */
  /*      tupevent              (c.f. paper for possible values) */
  /*      thrshl1               (e.g. = 10.0) */
  /*      thrshl2               (e.g. = 20.0) */
  /*      ptime                 must be 0 */
  /*      num                   must be 0 */

  /* subroutine parameters: */
  /* reltrc      : timeseries as floating data, possibly filtered */
  /* trace	      : timeseries as integer data, unfiltered, used to */
  /* 		determine the amplitudes */
  /* npts        : number of datapoints in the timeseries */
  /* rawold      : last datapoint of timeseries when leaving the subroutine */
  /* ssx         : sum of characteristic function (CF) */
  /* ssx2        : sum of squares of CF */
  /* mean        : mean of CF */
  /* sdev        : sigma of CF */
  /* num         : number of datapoints of the CF used for sdev & mean */
  /* omega       : weighting factor for the derivative of reltrc */
  /* y2          : last value of amplitude**2 */
  /* yt          : last value of derivative**2 */
  /* ipkflg      : pick flag */
  /* itar        : first triggered sample */
  /* amp         : maximum value of amplitude */
  /* ptime       : sample number of parrival */
  /* pamp        : maximum amplitude within trigger periode */
  /* pamptime    : sample number at which the amplitude was determined */
  /* preptime    : a possible earlier p-pick */
  /* prepamp     : maximum amplitude of earlier p-pick */
  /* prepamptime : sample number of prepamp */
  /* ifrst       : direction of first motion (1 or -1) */
  /* pfm         :     "          "     "    (U or D) */
  /* dtime       : counts the duration where CF dropped below thrshl1 */
  /* noise       : maximum noise amplitude */
  /* noisetime   : sample number of maximum noise amplitude */
  /* signal      : maximum signal amplitude */
  /* signaltime  : sample number of signal amplitude */
  /* uptime      : counter of samples where ipkflg>0 but no p-pick */
  /* test        : variable used for debugging, i.e. printed output */
  /* samplespersec: no of samples per second */
  /* realup      : number of up samples */
  /* realdown    : number of down samples in trigger window */

  /*      common /pckpar/ tdownmax,tupevent,thrshl1,thrshl2 */

  /* tdownmax    : if dtime exceeds tdownmax, the trigger is examined for */
  /*               validity */
  /* tupevent    : min nr of samples for itrm to be accepted as a pick */
  /* thrshl1     : threshold to trigger for pick (c.f. paper) */
  /* thrshl2     : threshold for updating sigma  (c.f. paper) */


  /* unfiltered */
  /* nanf & nend : write test output only for nanf<=nsmp<=nend */
  /*      data nanf,nend/9000,10000/ */
  /*      if(ptime.ne.0) then    !do only amplitude update after a valid */
  /*                             !pick was found */
  /*         call signalamplitude(reltrc,npts,amp) */
  /*         return */
  /*      endif */
  /* Parameter adjustments */
  --trace;
  --reltrc;

  /* Function Body */
  amptime = 0;
  realup = 0;
  if (*skip == 0) {
    nstart = *samplespersec * 3;
    /* skip first 3 seconds: bb-signals may */
  }
  else{
    nstart = *skip;
  }
  if (*prset == 0) {
    *prset = *samplespersec * 2;
    /* skip first 3 seconds: bb-signals may */
  }
  preset(&reltrc[nstart], prset, &rawold, &y2, &yt, &ssx, &ssx2, &sdev, &
         num, &itar, ptime, preptime, pfm, ipkflg, samplespersec);
  omega = y2 / yt;
  /* set weighting factor */
  amp = 0;
  p_dur__ = *samplespersec * 6.f;
  picklength = *pickduration * *samplespersec;
  end_dur__ = 0;
  *noise = 0;
  *ipkflg = 0;
  *ptime = 0;
  *pamp = 0;
  *pamptime = 0;
  *preptime = 0;
  *prepamp = 0;
  *prepamptime = 0;
  *ifrst = 0;
  pfm[0]=' ';
  *noisetime = 0;
  *signal = 0;
  *signaltime = 0;
  mean = ssx;
  ssx = 0.f;
  i__ = 0;
  /* cc      nstart= 3*samplespersec		!skip first 3 seconds: bb-signals may */
  i__ = nstart;
  /* need time for the filter to become effective */
  if (*test != 0) {
    //s_wsle(&io___19);
    //SEISCOMP_DEBUG("Samplerate: %f",*samplespersec);
    //	e_wsle();
    // s_wsle(&io___20);
    // do_lio(&c__9, &c__1, "  i       edat        sdev        edev        ",
    // 	 (size_t)46);
    // do_lio(&c__9, &c__1, "rdat       ipkflg itar  ptime  prept  dtime", (
    // 	size_t)43);
    // e_wsle();
    /*         write(7,'(i6,4e12.4,5i7)') i,edat,sdev,edev,rdat, */
    /*     +      ipkflg,itar,ptime,preptime,dtime */
  }


 L160:

  /* loop for npts samples */
  ++i__;
  if (i__ > npts) {
    //SEISCOMP_DEBUG("ppick() loop L160: i__(%d) > npts(%d) samples",i__,npts);
    *signal = amp;
    *signaltime = amptime;
    if (*ptime == 0 && itar != 0) {
      itrm = i__ - itar - dtime + *ipkflg;
      if (itrm >= *tupevent) {
	//SEISCOMP_DEBUG("ppick() triggered for more than tupevent = %d", *tupevent);
	/* triggered for more than tupevent */
	if (*ptime == 0) {
	  *ptime = itar;
	  if (*samplespersec < 100.f) {
	    if (*ifrst < 0) {
	      pfm[0]='U';
	    }
	    /* ad-wandler invertiert */
	    if (*ifrst > 0) {
	      pfm[0]='D';
	    }
	    /* die signale */
	  } else {
	    if (*ifrst < 0) {
	      pfm[0]='D';
	    }
	    if (*ifrst > 0) {
	      pfm[0]='U';
	    }
	  }
	  if (*test != 0) {
	    std::cout <<  "pick at sample: " << *ptime << std::endl;
	    std::cout <<   " uptime= " << itrm << std::endl;
	    //itar = 0; // first triggered sample TODO: is this correct
	  }
	}
      }
    }
    if (*test != 0) {
      std::cout << "pick at sample: " << *ptime    << std::endl;
      std::cout << " uptime=      "   <<  itrm     << std::endl; // TODO: is this correct???
      std::cout << " tupevent=    "   << *tupevent << std::endl;
      std::cout << " dire=        "   << *ifrst    << std::endl;
      std::cout << " Firstmotion= "   <<  *pfm     << std::endl;
    }

    //SEISCOMP_DEBUG("ppick() about to return 0");
    return 0;
  }
  if (i__ > picklength) {
    iamp = abs(trace[i__]) + 0.5;
    if (iamp > amp) {
      amp = iamp;
      amptime = i__;
    }
    goto L160;
  }

  rdat = reltrc[i__];
  rdif = (rdat - rawold) * *samplespersec;
  rawold = rdat;
  rda2 = rdat * rdat;
  rdi2 = rdif * rdif;
  y2 += rda2;
  yt += rdi2;
  edat = rda2 + omega * rdi2;
  edat *= edat;
  if (ssx == 0.f) {
    mean = edat;
  }
  omega = y2 / yt;
  edev = (edat - mean) / sdev;

  if (*test != 0 && i__ >= *nanf && i__ <= *nend) {
    // s_wsfe(&io___33);
    // do_fio(&c__1, (char *)&i__, (size_t)sizeof(int));
    // do_fio(&c__1, (char *)&edat, (size_t)sizeof(double));
    // do_fio(&c__1, (char *)&sdev, (size_t)sizeof(double));
    // do_fio(&c__1, (char *)&edev, (size_t)sizeof(double));
    // do_fio(&c__1, (char *)&rdat, (size_t)sizeof(double));
    // do_fio(&c__1, (char *)&(*ipkflg), (size_t)sizeof(int));
    // do_fio(&c__1, (char *)&itar, (size_t)sizeof(int));
    // do_fio(&c__1, (char *)&(*ptime), (size_t)sizeof(int));
    // do_fio(&c__1, (char *)&(*preptime), (size_t)sizeof(int));
    // do_fio(&c__1, (char *)&dtime, (size_t)sizeof(int));
    // e_wsfe();
  }
  iamp = abs(trace[i__]) + 0.5;
  if (iamp > amp) {
    amp = iamp;
    amptime = i__;
  }
  if (i__ <= end_dur__) {
    *pamp = amp;
    *pamptime = amptime;
  }

  if (edev > *thrshl1 && i__ > nstart << 1) {
    if (*ipkflg == 0) {
      /* save current parameters */
      itar = i__; /* itar is the first triggered sample */
      *ipkflg = 1;
      if (*ptime == 0) {
	end_dur__ = itar + p_dur__;
	if (*noise == 0) {
	  *noise = amp;
	  *noisetime = amptime;
	}
	if (rdif < 0.f) {
	  *ifrst = -1;
	}
	if (rdif > 0.f) {
	  *ifrst = 1;
	}
      }
      if (*preptime == 0) {
	/* save this onset as pre-p */
	*preptime = itar;
	*prepamp = amp;
	*prepamptime = amptime;
      }
      *uptime = 1;
    } else if (*ptime == 0) {
      if (edev > 40.f && dtime == 0) {
	*ipkflg += 2;
      }
      ++(*uptime);
    }
    ++realup;

    dtime = 0;

  } else {
    if (*ipkflg != 0) {
      ++dtime;
      ++realdown;
      if (*ptime == 0) {
	++(*uptime);
      }
      if (dtime > *tdownmax) {
	itrm = i__ - itar - dtime + *ipkflg;
	if (realup >= *tupevent) {
	  /* triggered for more than tupev */
	  if (*ptime == 0) {
	    *ptime = itar;
	    if (*samplespersec < 100.f) {  // Hack for old AD converter at the SED, no longer in use
	      if (*ifrst < 0) {
		pfm[0]='U';
	      }
	      /* ad-wandler invertiert */
	      if (*ifrst > 0) {
		pfm[0] = 'D';
	      }
	      /* die signale */
	    } else {
	      if (*ifrst < 0) {
		pfm[0] = 'D';
	      }
	      if (*ifrst > 0) {
		pfm[0]='U';
	      }
	    }
	    itar = 0;
	  }
	  if (amp < *noise << 1) {
	    //SEISCOMP_DEBUG("ppick()amp < *noise, continue to find better pick");
	    /* continue to find better pick */
	    *ptime = 0;
	    *pamp = 0;
	    itar = 0;
	  }
	} else {
	  *prepamp = amp;
	  *prepamptime = amptime;
	  itar = 0;
	}
	*ipkflg = 0;
	*uptime = 0;
	realdown = 0;
	realup = 0;
      }
    }
  }

  /*  update standard deviation */
  if (edev < *thrshl2 || i__ <= nstart + 256) {
    ssx += edat;
    ssx2 += edat * edat;
    sum = (double) (num + 1);
    sdev = sqrt((sum * ssx2 - ssx * ssx) / (sum * sum));
    if (sdev <= 0.f) {
      sdev = 1.f;
    }
    mean = ssx / sum;
    num = sum + .5f;
  }
  goto L160;

} /* ppick */
