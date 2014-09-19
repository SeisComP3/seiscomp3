#pragma ident "$Id: ReleaseNotes.c,v 1.31 2008/01/10 16:26:14 dechavez Exp $"

char *VersionIdentString = "Release 1.11.0";

/* Release notes

1.11.0 01/10/2008
       Added -q option, include raw header display in raw feeds if -q not specified

1.10.2 06/28/2007 
       Made help even more verbose.

1.10.1 06/01/2007 
       Fixed secs to samples bug in reporting time tears, report time tears as
       days, hours, minutes, seconds as well as total seconds

1.10.0 05/17/2007
       Added -t option to track time tears

1.9.1  03/26/2007
       Print headers on vanilla "data" request (got lost in 1.9.0?)

1.9.0  02/19/2007
       Added conditional dl option, fixed up verbosity handling

1.8.1  02/09/2007
       Fixed problem of aborting on illegal wfdisc records in wd requests

1.8.0  01/25/2007
       Added datdir=- option for standard output data feeds

1.7.1  08/18/2006
       10.3 packet support

1.7.0  08/14/2006
       Added -l option for including latency reports in data requests

1.6.4  06/20/2006
       Link with libisi2.4.1

1.6.3  06/07/2006
       Link with libisi2.3.3 to understand QDPLUS packet types

1.6.2  09/10/2005
       Hex dump raw headers when -v option is given, improve reporting on
       reason for quitting when server sends an alert

1.6.1  07/26/2005
       changed behavior of raw option to mimic that of data (ie, only
       print header by default and output data only if datdir is given)

1.6.0  06/29/2005
       raw (seqno) support tested

1.5.2  06/24/2005
       Checkpoint support for raw (seqno) requests, not tested since
       server isn't ready yet

1.5.1  06/10/2005
       Rename isiSetLogging() to isiStartLogging()

1.5.0  01/27/2005 (cvs rtag isi_1_5_0 isi)
       Added dbgpath option for debugging traffic

1.4.8  09/29/2004 (cvs rtag isi_1_4_8 isi)
       Fixed typo in improved(?) help message

1.4.7  04/26/2004 (cvs rtag isi_1_4_7 isi)
       Improved(?) help message, default to uncompressed data 

1.4.6  01/29/2004 (cvs rtag isi_1_4_6 isi)
       Relink with purified libraries, use new isiFree[Soh,Cnf,Wfdisc]

1.4.5  12/21/2003 (cvs rtag isi_1_4_5 isi)
       Print default server name as part of help.
       Relink with liblogio 1.3.0, libiacp 1.3.4

1.4.4  12/11/2003 (cvs rtag isi_1_4_4 isi)
       Changed data() to include building and freeing data request
       Relink with libisi 1.4.7

1.4.3  12/09/2003 (cvs rtag isi_1_4_3 isi)
       Relink with another batch of updated libraries

1.4.2  12/04/2003 (cvs rtag isi_1_4_2 isi)
       Relink with libisi 1.4.5, libutil 2.2.4, and libiacp 1.3.2

1.4.1  11/21/2003 (cvs rtag isi_1_4_1 isi)
       Changed default server to idahub (bad idea?) and corrected
       test for isiReadGenericTS() failure cause

1.4.0  11/19/2003 (cvs rtag isi_1_4_0 isi)
       Rework to use simplified ISI API (libisi 1.4.0),
       and to include header in generic data disk dumps

1.3.0  11/13/2003 (cvs rtag isi_1_3_0 isi)
       Rework to use simplified ISI API (libisi 1.3.0)

1.2.0  11/04/2003 (cvs rtag isi_1_2_0 isi)
       Relink with libisi 1.2.0, libiacp 1.3.0

1.1.0  11/02/2003 (cvs rtag isi_1_1_0 isi)
       Rework to use ISI handle and related functions from version 1.1.0 of
       the library

1.0.0  10/16/2003 (cvs rtag isi_1_0_0 isi)
       Initial release

*/
