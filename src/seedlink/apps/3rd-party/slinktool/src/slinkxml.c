/***************************************************************************
 * slinkxml.c
 * INFO message handling routines and interface functions for libxml2
 *
 * Written by:
 *   Andres Heinloo, GFZ Potsdam GEOFON Project
 *   Chad Trabant, ORFEUS Data Center/MEREDIAN Project, IRIS/DMC
 *
 * modified: 2009.240
 ***************************************************************************/

#include <stdio.h>
#include <string.h>

#include <libslink.h>

#include "slinkxml.h"


/***************************************************************************
 * prtinfo_identification():
 * Format the specified XML document into an identification summary.
 ***************************************************************************/
void
prtinfo_identification (xmlDocPtr doc)
{
  xmlNodePtr root;

  if (doc == NULL || (root = xmlDocGetRootElement (doc)) == NULL)
    return;

  if (strcmp ((char *) root->name, "seedlink"))
    {
      sl_log (1, 0, "XML INFO root tag is not <seedlink>, invalid data\n");
      return;
    }

  printf ("SeedLink server: %s\n"
	  "Organization   : %s\n"
	  "Start time     : %s\n",
	  xml_get_prop (root, "software"),
	  xml_get_prop (root, "organization"),
	  xml_get_prop (root, "started"));
  
}  /* End of prtinfo_identification() */


/***************************************************************************
 * prtinfo_stations():
 * Format the specified XML document into a station list.
 ***************************************************************************/
void
prtinfo_stations (xmlDocPtr doc)
{
  xmlNodePtr root, node;

  if (doc == NULL || (root = xmlDocGetRootElement (doc)) == NULL)
    return;

  if (strcmp ((char *) root->name, "seedlink"))
    {
      sl_log (1, 0, "XML INFO root tag is not <seedlink>, invalid data\n");
      return;
    }

  for (node = root->children; node; node = node->next)
    {
      if (strcmp ((char *) node->name, "station"))
	continue;
      
      printf ("%-2s %-5s %s\n",
	      xml_get_prop (node, "network"),
	      xml_get_prop (node, "name"),
	      xml_get_prop (node, "description"));
    }
}  /* End of prtinfo_stations() */


/***************************************************************************
 * prtinfo_streams():
 * Format the specified XML document into a stream list.
 ***************************************************************************/
void
prtinfo_streams (xmlDocPtr doc)
{
  xmlNodePtr root, node0;

  if (doc == NULL || (root = xmlDocGetRootElement (doc)) == NULL)
    return;

  if (strcmp ((char *) root->name, "seedlink"))
    {
      sl_log (1, 0, "XML INFO root tag is not <seedlink>, invalid data\n");
      return;
    }

  for (node0 = root->children; node0; node0 = node0->next)
    {
      xmlNodePtr node1;
      const char *name, *network, *stream_check;
      
      if (strcmp ((char *) node0->name, "station"))
	continue;
      
      name = xml_get_prop (node0, "name");
      network = xml_get_prop (node0, "network");
      stream_check = xml_get_prop (node0, "stream_check");
      
      if ( !strcmp (stream_check, "enabled") )
	{
	  for (node1 = node0->children; node1; node1 = node1->next)
	    {
	      if (strcmp ((char *) node1->name, "stream"))
		continue;
	      
	      printf ("%-2s %-5s %-2s %-3s %s %s  -  %s\n", network, name,
		      xml_get_prop (node1, "location"),
		      xml_get_prop (node1, "seedname"),
		      xml_get_prop (node1, "type"),
		      xml_get_prop (node1, "begin_time"),
		      xml_get_prop (node1, "end_time"));
	    }
	}
      else
	{
	  sl_log (0, 1, "%-2s %-5s: No stream information, stream check disabled\n",
		  network, name);
	}
    }
}  /* End of prtinfo_streams() */


/***************************************************************************
 * prtinfo_gaps():
 * Format the specified XML document into a gap list.
 ***************************************************************************/
void
prtinfo_gaps (xmlDocPtr doc)
{
  xmlNodePtr root, node0;

  if (doc == NULL || (root = xmlDocGetRootElement (doc)) == NULL)
    return;

  if (strcmp ((char *) root->name, "seedlink"))
    {
      sl_log (1, 0, "XML INFO root tag is not <seedlink>, invalid data\n");
      return;
    }

  for (node0 = root->children; node0; node0 = node0->next)
    {
      xmlNodePtr node1;
      const char *name, *network, *stream_check;

      if (strcmp ((char *) node0->name, "station"))
	continue;

      name = xml_get_prop (node0, "name");
      network = xml_get_prop (node0, "network");
      stream_check = xml_get_prop (node0, "stream_check");
      
      if ( !strcmp (stream_check, "enabled") )
	{
	  for (node1 = node0->children; node1; node1 = node1->next)
	    {
	      xmlNodePtr node2;
	      const char *location, *seedname, *type;
	      
	      if (strcmp ((char *) node1->name, "stream"))
		continue;
	      
	      location = xml_get_prop (node1, "location");
	      seedname = xml_get_prop (node1, "seedname");
	      type = xml_get_prop (node1, "type");
	      
	      for (node2 = node1->children; node2; node2 = node2->next)
		{
		  if (strcmp ((char *) node2->name, "gap"))
		    continue;
		  
		  printf ("%-2s %-5s %-2s %-3s %s %s  -  %s\n", network, name,
			  location, seedname, type,
			  xml_get_prop (node2, "begin_time"),
			  xml_get_prop (node2, "end_time"));
		}
	    }
	}
      else
	{
	  sl_log (0, 1, "%-2s %-5s: No gap information, stream check disabled\n",
		  network, name);
	}
    }
}  /* End of prtinfo_gaps() */


/***************************************************************************
 * prtinfo_connections():
 * Format the specified XML document into a connection list.
 ***************************************************************************/
void
prtinfo_connections (xmlDocPtr doc)
{
  xmlNodePtr root, node0;

  if (doc == NULL || (root = xmlDocGetRootElement (doc)) == NULL)
    return;

  if (strcmp ((char *) root->name, "seedlink"))
    {
      sl_log (1, 0, "XML INFO root tag is not <seedlink>, invalid data\n");
      return;
    }

  printf
    ("STATION  REMOTE ADDRESS        CONNECTION ESTABLISHED   TX COUNT GAPS  QLEN FLG\n");
  printf
    ("-------------------------------------------------------------------------------\n");
  /* GE TRTE  255.255.255.255:65536 2002/08/01 11:00:00.0000 12345678 1234 12345 DSE */

  for (node0 = root->children; node0; node0 = node0->next)
    {
      xmlNodePtr node1;
      const char *network, *name, *end_seq;

      if (strcmp ((char *) node0->name, "station"))
	continue;

      network = xml_get_prop (node0, "network");
      name = xml_get_prop (node0, "name");
      end_seq = xml_get_prop (node0, "end_seq");

      for (node1 = node0->children; node1; node1 = node1->next)
	{
	  xmlNodePtr node2;
	  unsigned long qlen = 0;
	  int active = 0, window = 0, realtime = 0, selectors = 0, eod = 0;
	  const char *current_seq;
	  char address[25];
	  char flags[4] = { ' ', ' ', ' ', 0 };

	  if (strcmp ((char *) node1->name, "connection"))
	    continue;

	  for (node2 = node1->children; node2; node2 = node2->next)
	    {
	      if (!strcmp ((char *) node2->name, "window"))
		window = 1;

	      if (!strcmp ((char *) node2->name, "selector"))
		selectors = 1;
	    }

	  current_seq = xml_get_prop (node1, "current_seq");

	  if (strcmp (current_seq, "unset"))
	    {
	      qlen = (strtoul (xml_get_prop (node0, "end_seq"), NULL, 16) -
		      strtoul (xml_get_prop (node1, "current_seq"), NULL,
			       16)) & 0xffffff;
	      active = 1;
	    }

	  if (strcmp (xml_get_prop (node1, "realtime"), "no"))
	    realtime = 1;

	  if (strcmp (xml_get_prop (node1, "end_of_data"), "no"))
	    eod = 1;

	  if (!active)
	    flags[0] = 'O';	/* Connection opened, but not configured */
	  else if (window)
	    flags[0] = 'W';	/* Window extraction (TIME) mode */
	  else if (!realtime)
	    flags[0] = 'D';	/* Dial-up mode */
	  else
	    flags[0] = 'R';	/* Normal real-time mode */

	  if (selectors)
	    flags[1] = 'S';	/* Using selectors */

	  if (eod)
	    flags[2] = 'E';	/* Connection is waiting to be closed */

	  sprintf (address, "%.15s:%.5s",
		   xml_get_prop (node1, "host"),
		   xml_get_prop (node1, "port"));

	  printf ("%-2s %-5s %-21s %s %8s %4s ", network, name, address,
		  xml_get_prop (node1, "ctime"),
		  xml_get_prop (node1, "txcount"),
		  xml_get_prop (node1, "sequence_gaps"));

	  if (realtime && active)
	    printf ("%5lu ", qlen);
	  else
	    printf ("    - ");

	  printf ("%s\n", flags);
	}
    }
}  /* End of prtinfo_connections() */


xmlParserCtxtPtr
xml_begin ()
{
  return xmlCreatePushParserCtxt (NULL, NULL, NULL, 0, NULL);
}

int
xml_parse_chunk (xmlParserCtxtPtr ctxt, const char *chunk, int size,
		 int terminate)
{
  return xmlParseChunk (ctxt, chunk, size, terminate);
}

xmlDocPtr
xml_end (xmlParserCtxtPtr ctxt)
{
  xmlDocPtr ret = ctxt->myDoc;
  xmlFreeParserCtxt (ctxt);
  return ret;
}

void
xml_free_doc (xmlDocPtr doc)
{
  xmlFreeDoc (doc);
}

const char *
xml_get_prop (xmlNodePtr node, const char *name)
{
  const char *ret;
  if ((ret = (char *) xmlGetProp (node, (xmlChar *) name)) == NULL)
    ret = "";
  return ret;
}
