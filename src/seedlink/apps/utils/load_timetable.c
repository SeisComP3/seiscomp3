/***************************************************************************** 
 * load_timetable.c
 *
 * Prints the end time and record number of all streams for chain_plugin
 *
 * (c) 2003 Andres Heinloo, GFZ Potsdam
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2, or (at your option) any later
 * version. For more information, see http://www.gnu.org/
 *****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <time.h>
#include <errno.h>

#if defined(__GNU_LIBRARY__) || defined(__GLIBC__)
#include <getopt.h>
#endif

#include <libxml/xmlerror.h>
#include <libxml/parser.h>

#include "libslink.h"

#include "qtime.h"

#define MYVERSION "1.0 (2019.199)"

const char *const ident_str = "load_timetable v" MYVERSION;

#if defined(__GNU_LIBRARY__) || defined(__GLIBC__)
const char *const opterr_message = "Try `%s --help' for more information\n";
const char *const help_message = 
    "Usage: %s [options]\n"
    "\n"
    "-v                            Increase verbosity level\n"
    "    --verbosity=LEVEL         Set verbosity level\n"
    "-V, --version                 Show version information\n"
    "-h, --help                    Show this help message\n";
#else
const char *const opterr_message = "Try `%s -h' for more information\n";
const char *const help_message =
    "Usage: %s [options]\n"
    "\n"
    "-v             Increase verbosity level\n"
    "-V             Show version information\n"
    "-h             Show this help message\n";
#endif

/* XML parser context for processing INFO responses */
static xmlParserCtxtPtr xml_parser_ctxt = NULL;

static void log_print(const char *s);
static xmlParserCtxtPtr xml_begin(void);
static int xml_parse_chunk(xmlParserCtxtPtr ctxt, const char *chunk, int size,
  int terminate);
static xmlDocPtr xml_end(xmlParserCtxtPtr ctxt);
static void xml_free_doc(xmlDocPtr doc);
static const char *xml_get_prop(xmlNodePtr node, const char *name);
static void prtinfo_timetable(xmlDocPtr doc);
static int info_handler(SLMSrecord *msr, int terminate);

void log_print(const char *s)
  {
    time_t t = time(NULL);
    char* ts = asctime(localtime(&t));
    char* p = strchr(ts, '\n');

    if(p != NULL) *p = 0;
    
    fprintf(stderr, "%s - load_timetable: %s", ts, s);
    fflush(stderr);
  }

/***************************************************************************
 * Interface functions for libxml2
 ***************************************************************************/

xmlParserCtxtPtr xml_begin(void)
  {
    return xmlCreatePushParserCtxt(NULL, NULL, NULL, 0, NULL);
  }

int xml_parse_chunk(xmlParserCtxtPtr ctxt, const char *chunk, int size,
  int terminate)
  {
    return xmlParseChunk(ctxt, chunk, size, terminate);
  }

xmlDocPtr xml_end(xmlParserCtxtPtr ctxt)
  {
    xmlDocPtr ret = ctxt->myDoc;
    xmlFreeParserCtxt(ctxt);
    return ret;
  }

void xml_free_doc(xmlDocPtr doc)
  {
    xmlFreeDoc(doc);
  }

const char *xml_get_prop(xmlNodePtr node, const char *name)
  {
    const char *ret;
    if((ret = (char *) xmlGetProp(node, (xmlChar *) name)) == NULL)
      ret = "";
    return ret;
  }

/***************************************************************************
 * prtinfo_timetable():
 * Format the specified XML document into a chain_plugin-readable timetable.
 ***************************************************************************/

void prtinfo_timetable(xmlDocPtr doc)
  {
    xmlNodePtr root, node0;

    if(doc == NULL || (root = xmlDocGetRootElement(doc)) == NULL)
        return;

    if(strcmp((char *) root->name, "seedlink"))
      {
        sl_log(1, 0, "XML data has invalid structure\n");
        return;
      }

    for(node0 = root->children; node0; node0 = node0->next)
      {
        xmlNodePtr node1;
        const char *network, *name;

        if(strcmp((char *) node0->name, "station"))
          continue;

        network = xml_get_prop(node0, "network");
        name = xml_get_prop(node0, "name");

        for(node1 = node0->children; node1; node1 = node1->next)
          {
            int year, month, mday, yday, hour, minute, second, fract;

            if(strcmp((char *) node1->name, "stream"))
                continue;

            if(sscanf(xml_get_prop(node1, "end_time"),
              "%4d/%2d/%2d %2d:%2d:%2d.%4d", &year, &month, &mday,
              &hour, &minute, &second, &fract) != 7)
              {
                sl_log(1, 0, "cannot parse time\n");
                continue;
              }

            yday = mdy_to_doy(month, mday, year);
            
            printf("%s %s %s.%s.%s %s %d %d %d %d %d %d\n",
              network, name,
              xml_get_prop(node1, "location"),
              xml_get_prop(node1, "seedname"),
              xml_get_prop(node1, "type"),
              xml_get_prop(node1, "end_recno"),
              year, yday, hour, minute, second, fract * 100);
          }
      }
  }

/***************************************************************************
 * info_handler():
 * Process XML-based INFO packets.
 *
 * Returns:
 * -2 = Errors
 * -1 = XML is terminated
 *  0 = XML is not terminated
 ***************************************************************************/

int info_handler(SLMSrecord *msr, int terminate)
  {
    int preturn;

    char *xml_chunk = (char *)msr->msrecord + msr->fsdh.begin_data;
    int xml_size = msr->fsdh.num_samples;

    /* Init XML parser context if it has not been done yet */
    if(xml_parser_ctxt == NULL)
      {
        if((xml_parser_ctxt = xml_begin()) == NULL)
          {
            sl_log(1, 0, "xml_begin(): memory allocation error\n");
            return -2;
          }
      }

    /* Check for an error condition */
    if(!strncmp(msr->fsdh.channel, "ERR", 3) )
      {
        sl_log(1, 0, "INFO type requested is not enabled\n");
        return -2;
      }

    /* Parse the XML */
    preturn = xml_parse_chunk(xml_parser_ctxt, xml_chunk, xml_size, terminate);

    if(preturn != XML_ERR_OK)
      {
        sl_log(1, 0, "XML parse error %d\n", preturn);
        return -2;
      }

    if(terminate || preturn)
      {
        xmlDocPtr doc = xml_end(xml_parser_ctxt);
        xml_parser_ctxt = NULL;
        prtinfo_timetable(doc);
        xml_free_doc(doc);
        return -1;
      }

    return preturn;
  }                                /* End of info_handler() */

int main(int argc, char **argv)
  {
    SLCD* slcd;
    SLpacket* slpack;
    char* address;
    int verbosity = 0;

#if defined(__GNU_LIBRARY__) || defined(__GLIBC__)
    struct option ops[] = 
      {
        { "verbosity",      required_argument, NULL, 'X' },
        { "version",        no_argument,       NULL, 'V' },
        { "help",           no_argument,       NULL, 'h' },
        { NULL }
      };
#endif

    const char* p = strrchr(argv[0], '/');
    const char* progname = ((p == NULL)? argv[0]: (p + 1));

    int c;

#if defined(__GNU_LIBRARY__) || defined(__GLIBC__)
    while((c = getopt_long(argc, argv, "vVh", ops, NULL)) != EOF)
#else
    while((c = getopt(argc, argv, "vVh")) != EOF)
#endif
      {
        switch(c)
          {
          case 'v': ++verbosity; break;
          case 'X': verbosity = atoi(optarg); break;
          case 'V': fprintf(stdout, "%s\n", ident_str); exit(0);
          case 'h': fprintf(stdout, help_message, progname); exit(0);
          case '?': fprintf(stderr, opterr_message, progname); exit(0);
          }
      }
    
    if(optind != argc - 1)
      {
        fprintf(stderr, help_message, progname);
        exit(1);
      }

    address = argv[optind];

    sl_loginit(verbosity, log_print, NULL, log_print, NULL);
  
    if((slcd = sl_newslcd()) == NULL)
      {
        fprintf(stderr, "sl_newslcd: %s\n", strerror(errno));
        exit(1);
      }
  
    slcd->netto = 10;
    slcd->netdly = 1;
    slcd->keepalive = 0;
    slcd->sladdr = address;
  
    /* If no host is given for the SeedLink server, add 'localhost' */
    if(*slcd->sladdr == ':')
      {
        char* tptr = (char *) malloc(strlen(slcd->sladdr) + 10);
        sprintf(tptr, "localhost%s", slcd->sladdr);
        slcd->sladdr = tptr;
      }

    sl_request_info(slcd, "STREAMS");

    while(sl_collect(slcd, &slpack) == SLPACKET)
      {
        SLMSrecord* msr;
        int ptype;
        int r;
        
        ptype = sl_packettype(slpack);

        if(ptype != SLINF && ptype != SLINFT)
          {
            /* should never happen */
            sl_log(1, 0, "non-INFO packet received!\n");
            continue;
          }

        msr = sl_msr_new();
        sl_msr_parse(NULL, slpack->msrecord, &msr, 0, 0);
        r = info_handler(msr, (ptype == SLINFT));
        sl_msr_free(&msr);
        
        if(r == -1)
            break;

        if(r == -2)
          {
            sl_log(1, 0, "processing of INFO packet failed\n");
            exit(1);
          }
      }

    return 0;
  }      

