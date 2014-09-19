/************************************************************************/
/*  Field definitions used in QDA data record headers.			*/
/*									*/
/*	Douglas Neuhauser						*/
/*	Seismological Laboratory					*/
/*	University of California, Berkeley				*/
/*	doug@seismo.berkeley.edu					*/
/*									*/
/************************************************************************/

/*
 * Copyright (c) 1996-2000 The Regents of the University of California.
 * All Rights Reserved.
 * 
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for educational, research and non-profit purposes,
 * without fee, and without a written agreement is hereby granted,
 * provided that the above copyright notice, this paragraph and the
 * following three paragraphs appear in all copies.
 * 
 * Permission to incorporate this software into commercial products may
 * be obtained from the Office of Technology Licensing, 2150 Shattuck
 * Avenue, Suite 510, Berkeley, CA  94704.
 * 
 * IN NO EVENT SHALL THE UNIVERSITY OF CALIFORNIA BE LIABLE TO ANY PARTY
 * FOR DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES,
 * INCLUDING LOST PROFITS, ARISING OUT OF THE USE OF THIS SOFTWARE AND
 * ITS DOCUMENTATION, EVEN IF THE UNIVERSITY OF CALIFORNIA HAS BEEN
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 * THE UNIVERSITY OF CALIFORNIA SPECIFICALLY DISCLAIMS ANY WARRANTIES,
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.  THE SOFTWARE
 * PROVIDED HEREUNDER IS ON AN "AS IS" BASIS, AND THE UNIVERSITY OF
 * CALIFORNIA HAS NO OBLIGATIONS TO PROVIDE MAINTENANCE, SUPPORT,
 * UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
 */

/*	$Id: qda.h 2 2005-07-26 19:28:46Z andres $ 	*/

#ifndef	__qda_h
#define	__qda_h

#define	QDA_HDR_SIZE	64		    /* QDA file header size.	*/

#define	SOH_INACCURATE		    0x80    /* inaccurate time tagging, in SOH  */
#define	SOH_GAP			    0x40    /* time gap detected, in SOH	*/
#define	SOH_EVENT_IN_PROGRESS	    0x20    /* record contains event data	*/
#define	SOH_BEGINNING_OF_EVENT	    0x10    /* record is first of event sequence	*/
#define	SOH_CAL_IN_PROGRESS	    0x08    /* record contains calibration data	*/
#define	SOH_EXTRANEOUS_TIMEMARKS    0x04    /* too many time marks received during this record*/
#define	SOH_EXTERNAL_TIMEMARK_TAG   0x02    /* record time-tagged at a mark    */
#define	SOH_RECEPTION_GOOD	    0x01    /* time reception is adequate   */

typedef struct _qda_time {
    char	year;
    char	month;
    char	day;
    char	hour;
    char	minute;
    char	second;
} QDA_TIME;

/*  Fixed QDA header	*/
typedef struct qda_hdr {	    /*	byte offset  */
    int		header_flag;		    /*  0   */
    char	frame_type;		    /*  4   */
    char	component;		    /*  5   */
    char	stream;			    /*	6   */
    char	soh;			    /*	7   */
    char	station_id[4];		    /*	8   */
    short	millisecond;		    /*	12  */
    short	time_mark;		    /*	14  */
    int		samp_1;			    /*	16  */
    short	clock_corr;		    /*	20  */
    short	num_samples;		    /*	22  */
    char	sample_rate;		    /*	24  */
    char	reserved;		    /*	25  */
    QDA_TIME	time;			    /*	36  */
    int		seq_no;			    /*	32  */
} QDA_HDR;

#endif

