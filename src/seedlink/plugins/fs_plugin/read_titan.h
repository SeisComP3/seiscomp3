/*======================================================================
    read_titan.h

    Library for Titan to binary data converter

    Author: J.-F. Fels, OMP, Toulouse

    Modified for SeedLink: A. Heinloo, GFZ Potsdam

*======================================================================*/

#ifndef READ_TITAN_H
#define READ_TITAN_H

#ifndef ANSI_C
#define ANSI_C
#include "hack_titan.h"
#include "hack_proto.h"
#undef ANSI_C
#else
#include "hack_titan.h"
#include "hack_proto.h"
#endif

// extern FILE    *Fp_err;        /* file ptr to output SEED log file */ 
extern FILE    *Fp_log;
extern int     inp_data[NCHAN][NCOMP][NINP]; /* Input data array */
extern struct  Channel Channel[NCHAN];
extern outData OutData[NCHAN];
extern struct  option opt;
extern Paths   db_paths;
extern Event   evn;
extern struct  acq acq;
extern int     totOutSamples;
extern char    Station[8];
extern char    Network[3];
extern int     byteswap;
extern char    tqf;            /* Time quality factor */
extern int     nodata;
extern double  SystTime;
extern double  Observ_dt;
extern double  ExtraTcorr;
extern char    netname[];
extern char    seed_scalefactor[];

#endif // READ_TITAN_H

