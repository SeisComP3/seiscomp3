/***************************************************************************
 * slarchive.c
 * A SeedLink client for data collection and archiving.
 *
 * Connects to a SeedLink server, collects and archives data.
 *
 * Written by Chad Trabant, ORFEUS/EC-Project MEREDIAN
 *
 * modified 2005.147
 * modified 2020.119, added signature verification
 ***************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <time.h>
#include <dirent.h>

#include <openssl/asn1.h>
#include <openssl/ecdsa.h>
#include <openssl/err.h>
#include <openssl/evp.h>
#include <openssl/pem.h>
#include <openssl/x509.h>

#include <libslink.h>

#include "archive.h"

#define PACKAGE   "slarchive"
#define VERSION   "1.8-sc3"

static void packet_handler(char *msrecord, int packet_type,
                           int seqnum, int packet_size);
static int  validate_record (SLMSrecord *msr);
static int  parameter_proc(int argcount, char **argvec);
static char *getoptval (int argcount, char **argvec, int argopt);
static int  add_dsarchive(const char *path, int archivetype);
static int  set_dsbuffersize(int size);
static void term_handler(int);
static void print_timelog(const char *msg);
static void usage(void);

/* A chain of archive definitions */
typedef struct DSArchive_s {
	DataStream  datastream;
	struct DSArchive_s *next;
}
DSArchive;

typedef struct CertCTX_s {
	const X509  *cert;
	char        *hash;
	int          nCerts;
	X509       **certs;
} CertCTX;

static const char *certStoreBaseDirectory = NULL;
static CertCTX certCtx;

static void cert_ctx_init (CertCTX *ctx);
static void cert_ctx_free (CertCTX *ctx);
static int  cert_ctx_load (CertCTX *ctx, const char *subject_hash);
static int  cert_ctx_update(const char *subject_hash);

static int  verify_signature(const X509 *cert, const ECDSA_SIG *signature,
                             const unsigned char *digest, int digest_len);

static short int verbose         = 0;   /* flag to control general verbosity */
static short int ppackets        = 0;   /* flag to control printing of data packets */
static int stateint              = 0;   /* packet interval to save statefile */
static char *statefile           = 0;	 /* state file for saving/restoring stream states */
static short int check_signature = 0;   /* flag to control authentication check */

static SLCD *slconn;	         /* connection parameters */
static DSArchive *dsarchive;


int main (int argc, char **argv) {
	SLpacket *slpack;
	int seqnum;
	int ptype;
	int packetcnt = 0;

	/* Signal handling, use POSIX calls with standardized semantics */
	  struct sigaction sa;

	sa.sa_flags   = SA_RESTART;
	sigemptyset (&sa.sa_mask);

	sa.sa_handler = term_handler;
	sigaction (SIGINT, &sa, NULL);
	sigaction (SIGQUIT, &sa, NULL);
	sigaction (SIGTERM, &sa, NULL);

	sa.sa_handler = SIG_IGN;
	sigaction (SIGHUP, &sa, NULL);
	sigaction (SIGPIPE, &sa, NULL);

	dsarchive = NULL;

	/* Allocate and initialize a new connection description */
	slconn = sl_newslcd();

	/* Process given parameters (command line and parameter file) */
	if ( parameter_proc (argc, argv) < 0 ) {
		fprintf (stderr, "Parameter processing failed\n");
		fprintf (stderr, "Try '-h' for detailed help\n");
		return -1;
	}

	/* Loop with the connection manager */
	while ( sl_collect (slconn, &slpack) ) {
		ptype  = sl_packettype (slpack);
		seqnum = sl_sequence (slpack);

		packet_handler ((char *) &slpack->msrecord, ptype, seqnum, SLRECSIZE);

		if ( statefile && stateint ) {
			if ( ++packetcnt >= stateint ) {
				if ( dsarchive ) {
					DSArchive *curdsa = dsarchive;

					while ( curdsa != NULL ) {
						if ( curdsa->datastream.grouphash ) {
							DataStreamGroup *curgroup, *tmp;
							HASH_ITER(hh, curdsa->datastream.grouphash, curgroup, tmp) {
								if ( curgroup->filed ) {
									if ( curgroup->bp ) {
										sl_log (1, 3, "Writing data to data stream file %s\n", curgroup->filename);
										if ( write (curgroup->filed, curgroup->buf, curgroup->bp) != curgroup->bp ) {
											sl_log (2, 1, "main: failed to write record\n");
											return -1;
										}
									}
									curgroup->bp = 0;
								}
							}
						}
						curdsa = curdsa->next;
					}
				}
				sl_savestate (slconn, statefile);
				packetcnt = 0;
			}
		}
	}

	/* Do all the necessary cleanup and exit */
	if (slconn->link != -1)
		sl_disconnect (slconn);

	if (dsarchive) {
		DSArchive *curdsa = dsarchive;

		while ( curdsa != NULL ) {
			archstream_proc (&curdsa->datastream, NULL, 0);
			curdsa = curdsa->next;
		}
	}

	if (statefile)
		sl_savestate (slconn, statefile);

	return 0;
}  /* End of main() */



#define AUTH_ERROR(msg)\
do {\
  if ( check_signature == 1 )\
    {\
      /* Warning mode */\
      sl_log (2, 0, "%s.%s.%s.%s: " msg ": authentication failed\n",\
              net, sta, loc, chan);\
    }\
  else\
    {\
      /* Skip mode */\
      sl_log (1, 1, "%s.%s.%s.%s: " msg ": skipping record\n",\
              net, sta, loc, chan);\
      return 1;\
    }\
} while(0)


/***************************************************************************
 * validate_record:
 * Check signature of a record and returns 0 in case of success or 1
 * otherwise.
 ***************************************************************************/
static int validate_record (SLMSrecord *msr) {
	static EVP_MD_CTX *mdctx = NULL;
	static ECDSA_SIG *static_signature = NULL;
	static unsigned char digest_buffer[EVP_MAX_MD_SIZE];
	static unsigned int digest_len;

	char          net[3], sta[6], loc[3], chan[4];
	const char   *opaque_headers, *subject_hash;
	char         *tptr;
	ECDSA_SIG    *signature;
	const unsigned char *pp;
	long          pp_len;
	int           i, retVal;
	const X509   *found_cert;

	sl_strncpclean (net, msr->fsdh.network, 2);
	sl_strncpclean (sta, msr->fsdh.station, 5);
	sl_strncpclean (loc, msr->fsdh.location, 2);
	sl_strncpclean (chan, msr->fsdh.channel, 3);

	if ( !msr->Blkt2000 ) {
		AUTH_ERROR("no blockette 2000 found");
	}
	else if ( msr->Blkt2000->header_cnt == 1 ) {
		/* Only one header expected with format: "SIGN/[subject-hash]~" */
		opaque_headers = msr->msrecord + msr->Blkt2000Ofs + sizeof(struct sl_blkt_2000_s);
		tptr = (char*)strchr(opaque_headers, '~');
		if ( tptr == NULL ) {
			AUTH_ERROR("no valid opaque data header");
		}
		else {
			*tptr = '\0';
			if ( strncmp(opaque_headers, "SIGN/", 5) == 0 ) {
				subject_hash = opaque_headers + 5;
				sl_log(1, 1, "%s.%s.%s.%s: Received record from authority '%s' \n",
				       net, sta, loc, chan, subject_hash);

				if ( !cert_ctx_update(subject_hash) ) {
					AUTH_ERROR("No matching certificate");
				}
				else {
					/* Extract signature and create digest */
					if ( mdctx == NULL )
						mdctx = EVP_MD_CTX_create();

					if ( static_signature == NULL )
						static_signature = ECDSA_SIG_new();

					EVP_DigestInit(mdctx, EVP_sha256());
					EVP_DigestUpdate(mdctx, msr->msrecord + offsetof(struct sl_fsdh_s, dhq_indicator), sizeof(msr->fsdh.dhq_indicator));
					EVP_DigestUpdate(mdctx, msr->msrecord + offsetof(struct sl_fsdh_s, station), offsetof(struct sl_fsdh_s, begin_data) - offsetof(struct sl_fsdh_s, station));
					EVP_DigestUpdate(mdctx, msr->msrecord + msr->fsdh.begin_data, (1 << msr->Blkt1000->rec_len) - msr->fsdh.begin_data);
					EVP_DigestFinal_ex(mdctx, (unsigned char*)digest_buffer, &digest_len);

					pp = (const unsigned char *)(msr->msrecord + msr->Blkt2000Ofs + ntohs(msr->Blkt2000->data_offset));
					pp_len = ntohs(msr->Blkt2000->total_len) - ntohs(msr->Blkt2000->data_offset);

					signature = d2i_ECDSA_SIG(&static_signature, &pp, pp_len);
					found_cert = NULL;

					/* First check the last matched certificate if available */
					if ( certCtx.cert ) {
						retVal = verify_signature(certCtx.cert, signature, digest_buffer, digest_len);
						if ( !retVal )
							found_cert = certCtx.cert;
					}

					if ( found_cert == NULL ) {
						for ( i = 0; i < certCtx.nCerts; ++i ) {
							const X509 *cert = certCtx.certs[i];
							if ( certCtx.cert == cert ) continue;

							retVal = verify_signature(cert, signature, digest_buffer, digest_len);
							if ( !retVal ) {
								found_cert = cert;
								break;
							}
						}
					}

					if ( found_cert == NULL ) {
						if ( check_signature == 1 ) {
							/* Warning mode */
							sl_log (2, 0, "%s.%s.%s.%s: record not authenticated: signature does not match\n",
							        net, sta, loc, chan);
						}
						else {
							/* Skip mode */
							sl_log (1, 1, "%s.%s.%s.%s: signature does not match: skipping record\n",
							        net, sta, loc, chan);
							return 1;
						}
					}
					else {
						sl_log (1, 1, "%s.%s.%s.%s: signature verified\n",
						        net, sta, loc, chan);
						certCtx.cert = found_cert;
					}
				}
			}
			else {
				if ( check_signature == 1 ) {
					/* Warning mode */
					sl_log (2, 0, "%s.%s.%s.%s: record not authenticated: no valid signature found\n",
					        net, sta, loc, chan);
				}
				else {
					/* Skip mode */
					sl_log (1, 1, "%s.%s.%s.%s: no signature found: skipping record\n",
					        net, sta, loc, chan);
					return 1;
				}
			}
		}
	}
	else {
		if ( check_signature == 1 ) {
			/* Warning mode */
			sl_log (2, 0, "%s.%s.%s.%s: record not authenticated: no valid signature found\n",
			        net, sta, loc, chan);
		}
		else {
			/* Skip mode */
			sl_log (1, 1, "%s.%s.%s.%s: no signature found: skipping record\n",
			        net, sta, loc, chan);
			return 1;
		}
	}

	return 0;
}


static void cert_ctx_init(CertCTX *ctx) {
	ctx->cert = NULL;
	ctx->hash = NULL;
	ctx->nCerts = 0;
	ctx->certs = NULL;
}

static void cert_ctx_free(CertCTX *ctx) {
	int i;
	if ( ctx->certs ) {
		for ( i = 0; i < ctx->nCerts; ++i )
			X509_free(ctx->certs[i]);
		free(ctx->certs);
	}
	if ( ctx->hash ) free(ctx->hash);
	cert_ctx_init(ctx);
}

static int cert_ctx_load(CertCTX *ctx, const char *subject_hash) {
	DIR *cert_dir;
	struct dirent *entry;
	const char *p;
	char *path;
	int i;
	BIO *bio;
	X509 *cert;
	X509 **certs;

	cert_ctx_free(ctx);

	sl_log(1, 1, "Certificate loading\n");
	sl_log(1, 1, "  Hash : %s\n", subject_hash);

	ctx->hash = strdup(subject_hash);
	if ( !ctx->hash || !certStoreBaseDirectory ) {
		sl_log(1, 1, "End\n");
		return 0;
	}

	cert_dir = opendir(certStoreBaseDirectory);
	if ( cert_dir == NULL ) {
		sl_log(2, 0, "%s: cannot open certificate directory\n",
		       certStoreBaseDirectory);
		return 0;
	}

	while ( (entry = readdir(cert_dir)) ) {
		/* Just interested in links und regular files */
		if ( entry->d_type != DT_LNK && entry->d_type != DT_REG ) continue;
		p = strrchr(entry->d_name, '.');
		/* Name must have an extension: .0, .1, .2 ... */
		if ( p == NULL ) continue;
		/* Name must be of length 8 (hexadecimal encoding of 32bit hash) */
		if ( p - entry->d_name != 8 ) continue;
		/* Extension must be a positive integer */
		i = atoi(p + 1);
		if ( i < 0 ) continue;

		path = (char*)malloc(strlen(certStoreBaseDirectory) + 1 + strlen(entry->d_name) + 1);
		*path = '\0';
		strcat(path, certStoreBaseDirectory);
		strcat(path, "/");
		strcat(path, entry->d_name);

		bio = BIO_new_file(path, "rb");

		if ( bio == NULL ) {
			free(path);
			sl_log(2, 0, "Failed to create bio for %s\n", entry->d_name);
			continue;
		}

		cert = NULL;
		PEM_read_bio_X509(bio, &cert, NULL, NULL);
		BIO_free(bio);
		free(path);

		certs = (X509**)realloc(ctx->certs, (ctx->nCerts + 1) * sizeof(X509*));
		if ( certs == NULL ) {
			sl_log(2, 0, "Memory allocation error in certificate store\n");
			return ctx->nCerts;
		}

		ctx->certs = certs;
		ctx->certs[ctx->nCerts] = cert;
		++ctx->nCerts;
	}

	closedir(cert_dir);

	sl_log(1, 1, "  N    : %d\n", ctx->nCerts);
	sl_log(1, 1, "End\n");
	return ctx->nCerts;
}


static int verify_signature(const X509 *cert, const ECDSA_SIG *signature,
                            const unsigned char *digest, int digest_len) {
	EVP_PKEY *pubkey;
	EC_KEY *ec_key;
	int retVal;

	pubkey = X509_get_pubkey((X509*)cert);
	if ( pubkey == NULL ) return 1;

	ec_key = EVP_PKEY_get1_EC_KEY(pubkey);
	if ( ec_key == NULL ) return 2;

	retVal = ECDSA_do_verify(digest, (int)digest_len, signature, ec_key);
	EC_KEY_free(ec_key);
	return retVal == 1 ? 0 : 3;
}


static int cert_ctx_update(const char *subject_hash) {
	if ( certCtx.hash && !strcmp(certCtx.hash, subject_hash) )
		return certCtx.nCerts;

	/* Force reload */
	return cert_ctx_load(&certCtx, subject_hash);
}


/***************************************************************************
 * packet_handler:
 * Process a received packet based on packet type.
 ***************************************************************************/
static void
packet_handler (char *msrecord, int packet_type, int seqnum, int packet_size)
{
  static SLMSrecord * msr = NULL;

  double      dtime;			/* Epoch time */
  double      secfrac;		/* Fractional part of epoch time */
  time_t      ttime;			/* Integer part of epoch time */
  char        timestamp[20];
  struct      tm *timep;
  int         archflag = 1;

  /* The following is dependent on the packet type values in libslink.h */
  const char *type[]  = { "Data", "Detection", "Calibration", "Timing",
                          "Message", "General", "Request", "Info",
                          "Info (terminated)", "KeepAlive" };

  if ( verbose >= 1 ) {
    /* Build a current local time string */
    dtime   = sl_dtime ();
    secfrac = (double) ((double)dtime - (int)dtime);
    ttime   = (time_t) dtime;
    timep   = localtime (&ttime);
    snprintf (timestamp, 20, "%04d.%03d.%02d:%02d:%02d.%01.0f",
	      timep->tm_year + 1900, timep->tm_yday + 1, timep->tm_hour,
	      timep->tm_min, timep->tm_sec, secfrac);
    
    sl_log (1, 1, "%s, seq %d, Received %s blockette\n",
	    timestamp, seqnum, type[packet_type]);
  }

  /* Parse data record and print requested detail if any */
  if ( sl_msr_parse (slconn->log, msrecord, &msr, 1, 0) == NULL )
    {
      sl_log (2, 0, "%s, seq %d, Received %s blockette, error: blockette discarded\n",
                          timestamp, seqnum, type[packet_type]);
      return;
    }
  
  if ( ppackets )
    sl_msr_print (slconn->log, msr, ppackets - 1);
  
  /* Process waveform data and send it on */
  if ( packet_type == SLDATA )
    {
      /* Test for a so-called end-of-detection record */
      if ( msr->fsdh.samprate_fact == 0 && msr->fsdh.num_samples == 0 )
	archflag = 0;
    }
  
  /* Write packet to all archives in archive definition chain */
  if ( dsarchive && archflag ) {
    if ( check_signature && validate_record(msr) )
      return;

    DSArchive *curdsa = dsarchive;

    while ( curdsa != NULL ) {
      curdsa->datastream.packettype = packet_type;

      /* Limit BUD archiving to waveform data */
      if ( curdsa->datastream.archivetype == BUD &&
	   packet_type != SLDATA ) {
	curdsa = curdsa->next;
	continue;
      }

      archstream_proc (&curdsa->datastream, msr, packet_size);
      
      curdsa = curdsa->next;
    }
  }
}  /* End of packet_handler() */


/***************************************************************************
 * parameter_proc:
 *
 * Process the command line parameters.
 *
 * Returns 0 on success, and -1 on failure.
 ***************************************************************************/
static int
parameter_proc (int argcount, char **argvec)
{
  int futurecontflag= 0;  /* continuous future check flag */
  int futurecont    = 2;  /* continuous future check overlap */
  int futureinitflag= 0;  /* initial future check (opening files) flag */
  int futureinit    = 2;  /* initial future check (opening files) overlap */
  int idletimeout   = 300; /* idle stream timeout */
  int error = 0;

  char *streamfile  = 0;   /* stream list file for configuring streams */
  char *multiselect = 0;
  char *selectors   = 0;
  char *timewin     = 0;
  char *sigmode     = 0;
  char *tptr;
  char *certsPath   = 0;

  SLstrlist *timelist;	   /* split the time window arg */

  if (argcount <= 1)
    error++;

  /* Process all command line arguments */
  for (optind = 1; optind < argcount; optind++)
    {
      if (strcmp (argvec[optind], "-V") == 0)
        {
          fprintf(stderr, "%s version: %s\n", PACKAGE, VERSION);
          exit (0);
        }
      else if (strcmp (argvec[optind], "-h") == 0)
        {
          usage();
          exit (0);
        }
      else if (strncmp (argvec[optind], "-v", 2) == 0)
	{
	  verbose += strspn (&argvec[optind][1], "v");
	}
      else if (strncmp (argvec[optind], "-p", 2) == 0)
        {
          ppackets += strspn (&argvec[optind][1], "p");
        }
      else if (strncmp (argvec[optind], "-Fc", 3) == 0)
        {
	  futurecontflag = 1;
	  if ( (tptr = strchr(argvec[optind], ':')) )
	    futurecont = atoi(tptr+1);
        }
      else if (strncmp (argvec[optind], "-Fi", 3) == 0)
        {
	  futureinitflag = 1;
	  if ( (tptr = strchr(argvec[optind], ':')) )
	    futureinit = atoi(tptr+1);
        }
      else if (strcmp (argvec[optind], "-d") == 0)
	{
	  slconn->dialup = 1;
	}
      else if (strcmp (argvec[optind], "-b") == 0)
	{
	  slconn->batchmode = 1;
	}
      else if (strcmp (argvec[optind], "-nt") == 0)
	{
	  slconn->netto = atoi (getoptval(argcount, argvec, optind++));
	}
      else if (strcmp (argvec[optind], "-nd") == 0)
	{
	  slconn->netdly = atoi (getoptval(argcount, argvec, optind++));
	}
      else if (strcmp (argvec[optind], "-k") == 0)
	{
	  slconn->keepalive = atoi (getoptval(argcount, argvec, optind++));
	}
      else if (strcmp (argvec[optind], "-i") == 0)
	{
	  idletimeout = atoi (getoptval(argcount, argvec, optind++));
	}
      else if (strcmp (argvec[optind], "-A") == 0)
	{
	  if ( add_dsarchive(getoptval(argcount, argvec, optind++), ARCH) == -1 )
	    return -1;
	}
	  else if (strcmp (argvec[optind], "-B") == 0)
	{
	  if ( set_dsbuffersize(atoi (getoptval(argcount, argvec, optind++))) == -1 )
	    return -1;
	}
      else if (strcmp (argvec[optind], "-SDS") == 0)
	{
	  if ( add_dsarchive(getoptval(argcount, argvec, optind++), SDS) == -1 )
	    return -1;
	}
      else if (strcmp (argvec[optind], "-BUD") == 0)
	{
	  if ( add_dsarchive(getoptval(argcount, argvec, optind++), BUD) == -1 )
	    return -1;
	}
      else if (strcmp (argvec[optind], "-DLOG") == 0)
	{
	  if ( add_dsarchive(getoptval(argcount, argvec, optind++), DLOG) == -1 )
	    return -1;
	}
      else if (strcmp (argvec[optind], "-l") == 0)
	{
	  streamfile = getoptval(argcount, argvec, optind++);
	}
      else if (strcmp (argvec[optind], "-s") == 0)
	{
	  selectors = getoptval(argcount, argvec, optind++);
	}
      else if (strcmp (argvec[optind], "-S") == 0)
	{
	  multiselect = getoptval(argcount, argvec, optind++);
	}
      else if (strcmp (argvec[optind], "-x") == 0)
	{
	  statefile = getoptval(argcount, argvec, optind++);
	}
      else if (strcmp (argvec[optind], "-tw") == 0)
	{
	  timewin = getoptval(argcount, argvec, optind++);
	}
      else if (strncmp (argvec[optind], "-Cs", 3) == 0)
        {
          sigmode = getoptval(argcount, argvec, optind++);
          if (strcmp(sigmode, "ignore") == 0)
            {
              check_signature = 0;
            }
          else if (strcmp(sigmode, "warning") == 0)
            {
              check_signature = 1;
            }
          else if (strcmp(sigmode, "skip") == 0)
            {
              check_signature = 2;
            }
          else
          {
            fprintf (stderr, "Unknown argument for option -Cs: %s\n", sigmode);
            exit (1);
          }
        }
      else if (strncmp (argvec[optind], "-certs", 5) == 0) {
          certsPath = getoptval(argcount, argvec, optind++);
          certStoreBaseDirectory = certsPath;
        }
      else if (strncmp (argvec[optind], "-", 1) == 0)
	{
	  fprintf (stderr, "Unknown option: %s\n", argvec[optind]);
	  exit (1);
	}
      else if (!slconn->sladdr)
        {
          slconn->sladdr = argvec[optind];
        }
      else
	{
	  fprintf (stderr, "Unknown option: %s\n", argvec[optind]);
	  exit (1);
	}
    }

  /* Make sure a server was specified */
  if ( ! slconn->sladdr )
    {
      fprintf(stderr, "No SeedLink server specified\n\n");
      fprintf(stderr, "%s version %s\n\n", PACKAGE, VERSION); 
      fprintf(stderr, "Usage: %s [options] [host][:][port]\n\n", PACKAGE);
      fprintf(stderr, "Try '-h' for detailed help\n");
      exit (1);
    }
  
  /* Initialize the verbosity for the sl_log function */
  sl_loginit (verbose, &print_timelog, NULL, &print_timelog, NULL);

  /* Set stdout (where logs go) to always flush after a newline */
  setvbuf(stdout, NULL, _IOLBF, 0);

  /* Report the program version */
  sl_log (1, 1, "%s version: %s\n", PACKAGE, VERSION);

  /* If errors then report the usage message and quit */
  if (error)
    {
      usage ();
      exit (1);
    }
  
  /* Load the stream list from a file if specified */
  if ( streamfile )
    sl_read_streamlist (slconn, streamfile, selectors);

  if ( check_signature ) {
      sl_log (1, 3, "Initialized X509 certificate store from directory '%s'\n", certsPath);
  }

  /* Split the time window argument */
  if ( timewin )
    {

      if (strchr (timewin, ':') == NULL)
	{
	  sl_log (2, 0, "time window not in begin:[end] format\n");
	  return -1;
	}

      if (sl_strparse (timewin, ":", &timelist) > 2)
	{
	  sl_log (2, 0, "time window not in begin:[end] format\n");
	  sl_strparse (NULL, NULL, &timelist);
	  return -1;
	}

      if (strlen (timelist->element) == 0)
	{
	  sl_log (2, 0, "time window must specify a begin time\n");
	  sl_strparse (NULL, NULL, &timelist);
	  return -1;
	}

      slconn->begin_time = strdup (timelist->element);

      timelist = timelist->next;

      if (timelist != 0)
	{
	  slconn->end_time = strdup (timelist->element);

	  if (timelist->next != 0)
	    {
	      sl_log (2, 0, "malformed time window specification\n");
	      sl_strparse (NULL, NULL, &timelist);
	      return -1;

	    }
	}

      /* Free the parsed list */
      sl_strparse (NULL, NULL, &timelist);
    }

  /* Parse the 'multiselect' string following '-S' */
  if ( multiselect )
    {
      if ( sl_parse_streamlist (slconn, multiselect, selectors) == -1 )
	return -1;
    }
  else if ( !streamfile )
    {		         /* No 'streams' array, assuming uni-station mode */
      sl_setuniparams (slconn, selectors, -1, 0);
    }

  /* Attempt to recover sequence numbers from state file */
  if (statefile)
    {
      /* Check if interval was specified for state saving */
      if ((tptr = strchr (statefile, ':')) != NULL)
	{
	  char *tail;
	  
	  *tptr++ = '\0';
	  
	  stateint = (unsigned int) strtoul (tptr, &tail, 0);
	  
	  if ( *tail || (stateint < 0 || stateint > 1e9) )
	    {
	      sl_log (2, 0, "state saving interval specified incorrectly\n");
	      return -1;
	    }
	}

      if (sl_recoverstate (slconn, statefile) < 0)
	{
	  sl_log (2, 0, "state recovery failed\n");
	}
    }
  
  /* If no archiving is specified print a warning */
  if ( !dsarchive ) {
    sl_log (1, 0, "WARNING: no archiving method was specified\n");
  }
  /* Otherwise fill in the global parameters for each entry */
  else {
    DSArchive *curdsa = dsarchive;
    while ( curdsa != NULL ) {
      curdsa->datastream.idletimeout = idletimeout;
      curdsa->datastream.futurecontflag = futurecontflag;
      curdsa->datastream.futurecont = futurecont;
      curdsa->datastream.futureinitflag = futureinitflag;
      curdsa->datastream.futureinit = futureinit;
      curdsa = curdsa->next;
    }
  }

  /** Free certificate context if allocated */
  cert_ctx_free(&certCtx);

  return 0;
}  /* End of parameter_proc() */


/***************************************************************************
 * getoptval:
 *
 * Return the value to a command line option; checking that the value is 
 * itself not an option (starting with '-') and is not past the end of
 * the argument list.
 *
 * argcount: total arguments in argvec
 * argvec: argument list
 * argopt: index of option to process, value is expected to be at argopt+1
 *
 * Returns value on success and exits with error message on failure
 ***************************************************************************/
static char *
getoptval (int argcount, char **argvec, int argopt)
{
  if ( argvec == NULL || argvec[argopt] == NULL ) {
    fprintf (stderr, "getoptval(): NULL option requested\n");
    exit (1);
  }

  if ( (argopt+1) < argcount && *argvec[argopt+1] != '-' )
    return argvec[argopt+1];

  fprintf (stderr, "Option %s requires a value\n", argvec[argopt]);
  exit (1);
}  /* End of getoptval() */


/***************************************************************************
 * add_dsarchive():
 * Add entry to the data stream archive chain.
 *
 * Returns 0 on success, and -1 on failure
 ***************************************************************************/
static int
add_dsarchive( const char *path, int archivetype )
{
  DSArchive *newdsa;

  newdsa = (DSArchive *) malloc (sizeof (DSArchive));
  
  if ( newdsa == NULL ) {
    sl_log (2, 0, "cannot allocate memory for new archive definition\n");
    return -1;
  }

  /* Setup new entry and add it to the front of the chain */
  newdsa->datastream.path = strdup(path);
  newdsa->datastream.archivetype = archivetype;
  newdsa->datastream.grouphash = NULL;

  if ( newdsa->datastream.path == NULL ) {
    sl_log (2, 0, "cannot allocate memory for new archive path\n");
    return -1;
  }

  newdsa->next = dsarchive;
  dsarchive = newdsa;

  return 0;
}  /* End of add_dsarchive() */


/***************************************************************************
 * set_dsbuffersize():
 * Sets the buffer size of the archive
 *
 * Returns 0 on success, and -1 on failure
 ***************************************************************************/
static int
set_dsbuffersize( int size )
{
  if ( size < 0 ) return -1;
  DS_BUFSIZE = size * 512;
  return 0;
}


/***************************************************************************
 * term_handler:
 * Signal handler routine to controll termination.
 ***************************************************************************/
static void
term_handler (int sig)
{
  sl_terminate (slconn);
}


/***************************************************************************
 * print_timelog:
 * Log message print handler used with sl_loginit() and sl_log().  Prefixes
 * a local time string to the message before printing.
 ***************************************************************************/
static void
print_timelog (const char *msg)
{
  char timestr[100];
  time_t loc_time;
  
  /* Build local time string and cut off the newline */
  time(&loc_time);
  strcpy(timestr, asctime(localtime(&loc_time)));
  timestr[strlen(timestr) - 1] = '\0';
  
  fprintf (stdout, "%s - %s", timestr, msg);
}


/***************************************************************************
 * usage:
 * Print the usage message and exit.
 ***************************************************************************/
static void
usage (void)
{
  fprintf (stderr, "%s version %s\n\n", PACKAGE, VERSION);
  fprintf (stderr, "Usage: %s [options] [host][:][port]\n\n", PACKAGE);
  fprintf (stderr,
       " ## General program options ##\n"
       " -V              report program version\n"
       " -h              show this usage message\n"
       " -v              be more verbose, multiple flags can be used\n"
       " -p              print details of data packets, multiple flags can be used\n"
       " -nd delay       network re-connect delay (seconds), default 30\n"
       " -nt timeout     network timeout (seconds), re-establish connection if no\n"
       "                   data/keepalives are received in this time, default 600\n"
       " -k interval     send keepalive (heartbeat) packets this often (seconds)\n"
       " -x sfile[:int]  save/restore stream state information to this file\n"
       " -i timeout      idle stream entries might be closed (seconds), default 300\n"
       " -d             configure the connection in dial-up mode\n"
       " -b             configure the connection in batch mode\n"
       " -Fi[:overlap]   Initially check (existing files) that data records are newer\n"
       " -Fc[:overlap]   Continuously check that data records are newer\n"
       "\n"
       " ## Data stream selection ##\n"
       " -s selectors    selectors for uni-station or default for multi-station mode\n"
       " -l listfile     read a stream list from this file for multi-station mode\n"
           " -S streams      define a stream list for multi-station mode\n"
       "   'streams' = 'stream1[:selectors1],stream2[:selectors2],...'\n"
       "        'stream' is in NET_STA format, for example:\n"
       "        -S \"IU_KONO:BHE BHN,GE_WLF,MN_AQU:HH?.D\"\n\n"
       " -tw begin:[end]  (requires SeedLink >= 3)\n"
       "        specify a time window in year,month,day,hour,min,sec format\n"
       "        example: -tw 2002,08,05,14,00,00:2002,08,05,14,15,00\n"
       "        the end time is optional, but the colon must be present\n"
       "\n"
       " ## Data archiving options ##\n"
       " -A format       save all received records is a custom file structure\n"
       " -B count        set record buffer size to count*512, default 1000\n"
       " -SDS  SDSdir    save all received records in a SDS file structure\n"
       " -BUD  BUDdir    save all received data records in a BUD file structure\n"
       " -DLOG DLOGdir   save all received data records in an old-style\n"
       "                   SeisComP/datalog file structure\n"
       "\n"
       " ## Data validation options ##\n"
       " -Cs mode        Signature checking mode: ignore, warn, skip\n"
       "                  Signatures are expected to be carried in blockette 2000\n"
       "                  as opaque data."
       "                  Modes:\n"
       "                   ignore : Signatures will be ignored and no further actions\n"
       "                            will be taken.\n"
       "                   warning: Signatures will be checked and all received records\n"
       "                            which do not carry a valid signature or no signature\n"
       "                            at all will be logged with at warning level.\n"
       "                   skip   : All received records without a valid signature\n"
       "                            will be ignored and will not be processed.\n"
       "                  Default is 'ignore'.\n"
       " -certs dir       Path to cerificate store where all certificates and CRLs are stored. Relative\n"
       "                  paths(as the default) are treated relative to the installation directory\n"
       "                  ($SEISCOMP_ROOT). If the signature check is enabled slarchive loads all files\n"
       "                  at start. The store uses the OpenSSl store format. From the offical OpenSSL\n"
       "                  documentation: \"The directory should contain one certificate or CRL per file\n"
       "                  in PEM format, with a file name of the form hash.N for a certificate, or\n"
       "                  hash.rN for a CRL. The .N or .rN suffix is a sequence number that starts at\n"
       "                  zero, and is incremented consecutively for each certificate or CRL with the\n"
       "                  same hash value. Gaps in the sequence numbers are not supported, it is\n"
       "                  assumed that there are no more objects with the same hash beyond the first\n"
       "                  missing number in the sequence.The .N or .rN suffix is a sequence number that\n"
       "                  starts at zero, and is incremented consecutively for each certificate or CRL\n"
       "                  with the same hash value. Gaps in the sequence numbers are not supported, it\n"
       "                  is assumed that there are no more objects with the same hash beyond the first\n"
       "                  missing number in the sequence.\" The hash value can be obtained as follows:\n"
       "                  openssl x509 -hash -noout -in <file>\n"
       "\n"
       " [host][:][port] Address of the SeedLink server in host:port format\n"
       "                  Default host is 'localhost' and default port is '18000'\n\n");

}  /* End of usage() */
