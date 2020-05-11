#include <stdio.h>
#include "project.h"
#include "config.h"
#include "map.h"
#include "gcf.h"

extern Map *rootmap;

int get_line (FILE *fd, char *line)
{
        char c;
        int i=0, n;

        do {
          n = fscanf ( fd, "%c", &c);
          if (n<0) return (-1);
          if ( c == '\n' ) break;
          *(line+i) = c;
          i++;
        } while (1);
        *(line+i) = '\0';
        return(i);
}

Map *init_map ()
{
        Map *map;
        map = malloc (sizeof(Map));
        return(map);
}

void add_map (Map *inmap)
{
   Map *newmap, *mp;

   if ( rootmap == NULL ) {
        rootmap = malloc (sizeof(Map));
        rootmap->sysid   = inmap->sysid != NULL ? strdup ( inmap->sysid ) : NULL; 
        rootmap->stream  = strdup ( inmap->stream ); 
        rootmap->network = strdup ( inmap->network ); 
        rootmap->station = strdup ( inmap->station ); 
        rootmap->channel = strdup ( inmap->channel ); 
        rootmap->id      = inmap->id;
        rootmap->next    = NULL;
   }
   else {
        for ( mp = rootmap; mp != NULL; mp=mp->next ) {
              if ( strncmp ( mp->stream, inmap->stream, strlen(inmap->stream)) == 0 ) {
                   if ( mp->sysid == NULL && inmap->sysid == NULL )
                       break;
                   if ( mp->sysid != NULL && inmap->sysid != NULL &&
                        strncmp ( mp->sysid, inmap->sysid, strlen(inmap->sysid)) == 0 )
                       break;
              }
        }

        if (mp==NULL) {
            newmap = init_map ();
            newmap->sysid   = inmap->sysid != NULL ? strdup ( inmap->sysid ) : NULL; 
            newmap->stream  = strdup ( inmap->stream ); 
            newmap->network = strdup ( inmap->network ); 
            newmap->station = strdup ( inmap->station ); 
            newmap->channel = strdup ( inmap->channel ); 
            newmap->id      =  inmap->id;
            newmap->next    = NULL;

            for ( mp = rootmap; mp != NULL; mp=mp->next ) {

              if ( mp->next == NULL ) {
                   mp->next = newmap;
                   break;
              }
            }
        }
   }
}

void parse_mapfile (char *mapfile)
{
   FILE *fd;
   Map *newmap;
   char lineid[200];
   char network[20];
   char code[20];
   char station[20];
   char channel[20];
   char line[1000];
   int  n;
   char *sep;
   int cntm=0;


   if ( (fd = fopen(mapfile,"r") ) == NULL ) {
         printf("Map file [%s] not opened\n", mapfile);  
         exit(0);
   }

   else {

        // parse lines like:
        //    ChanInfo PMSTZ4	IP  PMST	BHZ

        do {

           newmap = malloc (sizeof(Map));

           n = get_line (fd, line);
           if (n<0) break;

           sprintf ( lineid, " ");
           sscanf ( line, "%s", lineid );

           if ( strncmp ( lineid, "ChanInfo", 8) == 0 ) {
               n = sscanf ( line, "%s %s %s %s %s", 
                            lineid, code, network, station, channel);

                newmap->sysid = NULL;

                // Split code by '.' to separate sysid and streamid
                sep = strchr(code, '.');
                if (sep != NULL) {
                    *sep = '\0';
                    newmap->sysid = strdup ( code );
                    newmap->stream = strdup ( sep+1 );
                }
                else {
                    newmap->stream = strdup ( code ); 
                }

                newmap->network = strdup ( network ); 
                newmap->station = strdup ( station ); 
                newmap->channel = strdup ( channel ); 
                newmap->id =  cntm; cntm++;
                printf ( "newmap:  %s  %s %s %s %s %d  (%s %s)\n", 
                         lineid, newmap->stream, newmap->network, newmap->station, newmap->channel, newmap->id,
                         newmap->sysid, newmap->stream);

               if ( newmap->stream != NULL) {
                    add_map (newmap);
               }

               if (newmap != NULL) free(newmap);
           }

        } while (1);

   }

}
