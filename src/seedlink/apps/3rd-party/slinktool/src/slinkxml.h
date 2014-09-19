
#ifndef SLINKXML_H
#define SLINKXML_H

#include <libxml/xmlerror.h>
#include <libxml/parser.h>

#ifdef __cplusplus
extern "C"
{
#endif

extern void prtinfo_identification(xmlDocPtr doc);
extern void prtinfo_stations(xmlDocPtr doc);
extern void prtinfo_streams(xmlDocPtr doc);
extern void prtinfo_gaps(xmlDocPtr doc);
extern void prtinfo_connections(xmlDocPtr doc);

/* Interface functions for libxml2 */
extern  xmlParserCtxtPtr xml_begin ();
extern  int xml_parse_chunk (xmlParserCtxtPtr ctxt, const char *chunk, int size,
			     int terminate);
extern  xmlDocPtr xml_end (xmlParserCtxtPtr ctxt);
extern  void xml_free_doc (xmlDocPtr doc);
extern  const char *xml_get_prop (xmlNodePtr node, const char *name);

#ifdef __cplusplus
}
#endif

#endif				/* SLINKXML_H */
