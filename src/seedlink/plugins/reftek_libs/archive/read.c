#pragma ident "$Id: read.c 165 2005-12-23 12:34:58Z andres $"
/* --------------------------------------------------------------------
 Program  : Any
 Task     : Archive Library API functions.
 File     : read.c
 Purpose  : Cooked packet read functions.
 Host     : CC, GCC, Microsoft Visual C++ 5.x
 Target   : Solaris (Sparc and x86), Linux, Win32
 Author   : Robert Banfill (r.banfill@reftek.com)
 Company  : Refraction Technology, Inc.
            2626 Lombardy Lane, Suite 105
            Dallas, Texas  75220  USA
            (214) 353-0609 Voice, (214) 353-9659 Fax, info@reftek.com
 Copyright: (c) 1997-2005 Refraction Technology, Inc. - All Rights Reserved.
 Notes    :
 $Revision: 165 $
 $Logfile : R:/cpu68000/rt422/struct/version.h_v  $
 Revised  :
		05Oct05	(pld) change sample rate handling to float
					(pld) Add debug messages.
		2000-03-22 1.16 RLB Fixed rate determination bug.
		17 Aug 1998  ---- (RLB) First effort.

-----------------------------------------------------------------------*/

#include "archive.h"

#define debugf no_print
//#define debugf printf

/* Local prototypes ---------------------------------------------------*/
BOOL CreateEventHeader(H_STREAM hstream, RF_PACKET * packet);
BOOL GetNextDataPacket(H_STREAM hstream, RF_HEADER * hdr, RF_PACKET * packet);
H_STREAM GetNextStreamHandle(H_ARCHIVE harchive);
VOID FixupSequenceNumber(H_STREAM hstream, RF_PACKET * packet, UINT8 type);
VOID FixupHeaderTrailer(H_STREAM hstream, UINT8 type);
BOOL ReadPacket(H_FILE file, RF_HEADER * hdr, RF_PACKET * packet);
BOOL UnReadPacket(H_FILE file);

/****************************************************************************
	Purpose: used to turn off debug data
   Returns:	
   Notes  : 
   Revised:

===========================================================================*/
void no_print( const char *str, ... )
	{
	;
	}  /* end no_print() */

/*---------------------------------------------------------------------*/
H_STREAM ArchiveOpenStream(H_ARCHIVE harchive, STREAM * criteria, UINT32 options)
{
    H_STREAM hstream;

    ASSERT(criteria != NULL);

    if (!ValidateHandle(harchive))
        return (VOID_H_STREAM);

    _archive[harchive].last_error = ARC_NO_ERROR;

    /* Must specify unit and stream */
    if (criteria->unit == VOID_UNIT || criteria->stream == VOID_STREAM) {
        ArchiveLog(ARC_LOG_ERRORS, "%s", "ArchiveOpenStream: criteria must specify unit and stream!");
        _archive[harchive].last_error = ARC_BAD_CRITERIA;
        return (VOID_H_STREAM);
    }

    /* Get next available stream handle */
    if ((hstream = GetNextStreamHandle(harchive)) == VOID_H_STREAM)
        return (VOID_H_STREAM);

    /* Start clean */
    InitRead(hstream);

    /* Copy criteria and options */
    _reads[hstream].harchive = harchive;
    _reads[hstream].criteria.unit = criteria->unit;
    _reads[hstream].criteria.stream = criteria->stream;
    _reads[hstream].criteria.chn_mask = criteria->chn_mask;
    _reads[hstream].criteria.time.earliest = criteria->time.earliest;
    _reads[hstream].criteria.time.latest = criteria->time.latest;
    _reads[hstream].options = options;

    /* Find the first event file */
    if (!ArchiveFindFirstEvent(_reads[hstream].harchive, &_reads[hstream].criteria,
            &_reads[hstream].event))
        return (VOID_H_STREAM);

    /* If strict continuous is on, file must contain the criteria earliest time */
    if (_reads[hstream].options & OPT_NO_DISCONTINUOUS) {
        if (_reads[hstream].event.time.latest > _reads[hstream].criteria.time.earliest) {
            _archive[harchive].last_error = ARC_NOT_FOUND;
            return (VOID_H_STREAM);
        }
    }

    return (hstream);
}

/*---------------------------------------------------------------------*/
BOOL ArchiveCloseStream(H_STREAM hstream)
{

    if (!ValidateStreamHandle(hstream))
        return (FALSE);

    _archive[_reads[hstream].harchive].last_error = ARC_NO_ERROR;

    /* Close any open file */
    if (_reads[hstream].file != VOID_H_FILE) {
        if (!FileClose(_reads[hstream].file)) {
            _archive[_reads[hstream].harchive].last_error = ARC_FILE_IO_ERROR;
            return (FALSE);
        }
    }

    /* Clear any state */
    InitRead(hstream);

    return (TRUE);
}

/*---------------------------------------------------------------------*/
BOOL ArchiveReadPacket(H_STREAM hstream, RF_PACKET * packet)
{
    RF_HEADER hdr;

    ASSERT(packet != NULL);

debugf("ARP %u:%p...", hstream, packet);

    if (!ValidateStreamHandle(hstream))
        return (FALSE);

    _archive[_reads[hstream].harchive].last_error = ARC_NO_ERROR;

debugf("ARP state %u...", _reads[hstream].state);
    /* Act on state */
    if (_reads[hstream].state == STA_STARTING) {
        /* Get first data packet */
        if (!CreateEventHeader(hstream, packet))
            return (FALSE);
        /* Transistion */
        _reads[hstream].state = STA_IN_PROGRESS;
    }
    else if (_reads[hstream].state == STA_IN_PROGRESS) {
        /* Get the next packet */
        if (!GetNextDataPacket(hstream, &hdr, packet)) {
            /* End of event (due to break or end of data) */
            if (_archive[_reads[hstream].harchive].last_error == ARC_SEQUENCE_BREAK ||
                _archive[_reads[hstream].harchive].last_error == ARC_END_OF_DATA) {

                /* Fix up our saved EH as ET */
                FixupHeaderTrailer(hstream, ET);
                /* Copy to user buffer */
debugf("RD copy %p->%p...", &_reads[hstream].eh, packet);
                memcpy(packet, &_reads[hstream].eh, RF_PACKET_SIZE);

                /* If this is a break and not told not to, keep on trying */
                if (_archive[_reads[hstream].harchive].last_error == ARC_SEQUENCE_BREAK &&
                    !(_reads[hstream].options & OPT_NO_DISCONTINUOUS)) {
                    _reads[hstream].state = STA_STARTING;
debugf("RD done1");
                    return (TRUE);
                }
                /* Otherwise, end-of-data, we're done */
                else
                    _reads[hstream].state = STA_FINISHED;
debugf("RD done2");
            }
            /* Something bad happened */
            else
                return (FALSE);
        }
    }
    else if (_reads[hstream].state == STA_FINISHED) {
        _archive[_reads[hstream].harchive].last_error = ARC_END_OF_DATA;
        return (FALSE);
    }

    return (TRUE);
}  /* end ArchiveReadPacket() */

/*---------------------------------------------------------------------*/
BOOL ArchiveStreamTimeRange(H_STREAM hstream, TIME_RANGE * range)
{

    ASSERT(range != NULL);

    if (!ValidateStreamHandle(hstream))
        return (FALSE);

    range->earliest = _reads[hstream].time.earliest;
    range->latest = _reads[hstream].time.latest;

    return (TRUE);
}

/*---------------------------------------------------------------------*/
BOOL ArchiveReadError(H_STREAM hstream)
{

    if (ArchiveEndOfData(hstream))
        return (FALSE);

    return (TRUE);
}

/*---------------------------------------------------------------------*/
BOOL ArchiveEndOfData(H_STREAM hstream)
{

    if (!ValidateStreamHandle(hstream))
        return (FALSE);

    if (_archive[_reads[hstream].harchive].last_error == ARC_END_OF_DATA)
        return (TRUE);

    return (FALSE);
}

/*---------------------------------------------------------------------*/
/* Helpers... */
/*---------------------------------------------------------------------*/
BOOL CreateEventHeader(H_STREAM hstream, RF_PACKET * packet)
{
    CHAR string[32];
    UINT8 stream, channel;
    UINT16 i, unit, length;
    INT32 position;
    REAL64 time;
    RF_HEADER hdr;

    ASSERT(packet != NULL);

    if (!ValidateStreamHandle(hstream))
        return (FALSE);

    _archive[_reads[hstream].harchive].last_error = ARC_NO_ERROR;

    /* Open the file if needed */
    if (_reads[hstream].file == VOID_H_FILE) {
        position = VOID_INT32;
        if ((_reads[hstream].file = FileOpenForRead(_reads[hstream].event.filespec)) == VOID_H_FILE) {
            _archive[_reads[hstream].harchive].last_error = ARC_FILE_IO_ERROR;
            return (FALSE);
        }
    }
    else {
        /* Otherwise, save position in file */
        position = FilePosition(_reads[hstream].file);
        if (!FileRewind(_reads[hstream].file)) {
            _archive[_reads[hstream].harchive].last_error = ARC_FILE_IO_ERROR;
            return (FALSE);
        }
    }

    /* Read first packet */
    if (!ReadPacket(_reads[hstream].file, &hdr, &_reads[hstream].eh)) {
        _archive[_reads[hstream].harchive].last_error = ARC_FILE_IO_ERROR;
        return (FALSE);
    }
    /* Is it an EH? */
    if (hdr.type == EH) {
        /* Get the sampling rate */
        _reads[hstream].rate = DecodeRFRate(&_reads[hstream].eh);
        if (!IsValidRate(_reads[hstream].rate)) {
            _archive[_reads[hstream].harchive].last_error = ARC_NO_RATE;
            return (FALSE);
        }
debugf("CEH EHrate: %.1f...", _reads[hstream].rate);
    }
    else {
        /* No, read last packet */
        if (!FileSeekEOF(_reads[hstream].file)) {
            _archive[_reads[hstream].harchive].last_error = ARC_FILE_IO_ERROR;
            return (FALSE);
        }
        _reads[hstream].in_seq = VOID_UINT16;
        if (!UnReadPacket(_reads[hstream].file)) {
            _archive[_reads[hstream].harchive].last_error = ARC_FILE_IO_ERROR;
            return (FALSE);
        }
        if (!ReadPacket(_reads[hstream].file, &hdr, &_reads[hstream].eh)) {
            _archive[_reads[hstream].harchive].last_error = ARC_FILE_IO_ERROR;
            return (FALSE);
        }
        if (!FileRewind(_reads[hstream].file)) {
            _archive[_reads[hstream].harchive].last_error = ARC_FILE_IO_ERROR;
            return (FALSE);
        }
        /* Is it an ET? */
        if (hdr.type == ET) {
            /* Get the sampling rate */
            _reads[hstream].rate = DecodeRFRate(&_reads[hstream].eh);
            if (!IsValidRate(_reads[hstream].rate)) {
                _archive[_reads[hstream].harchive].last_error = ARC_NO_RATE;
                return (FALSE);
            }
debugf("CEH ETrate: %.1f...", _reads[hstream].rate);
        }
        else {
            /* Try to derive rate... */
            unit = VOID_UINT8;
            while (TRUE) {
                if (!ReadPacket(_reads[hstream].file, &hdr, &_reads[hstream].eh)) {
                    _archive[_reads[hstream].harchive].last_error = ARC_FILE_IO_ERROR;
                    return (FALSE);
                }
                if (FileAtEOF()) {
                    _archive[_reads[hstream].harchive].last_error = ARC_NO_RATE;
                    return (FALSE);
                }
                if (hdr.type == DT && hdr.length > 0) {
                    unit = hdr.unit;
                    stream = hdr.stream;
                    channel = hdr.channel;
                    time = hdr.time;
                    length = hdr.length;
                    break;
                }
            }
            if (unit == VOID_UINT8) {
                _archive[_reads[hstream].harchive].last_error = ARC_NO_RATE;
                return (FALSE);
            }
            while (TRUE) {
                if (!ReadPacket(_reads[hstream].file, &hdr, &_reads[hstream].eh)) {
                    _archive[_reads[hstream].harchive].last_error = ARC_FILE_IO_ERROR;
                    return (FALSE);
                }
                if (FileAtEOF()) {
                    _archive[_reads[hstream].harchive].last_error = ARC_NO_RATE;
                    return (FALSE);
                }
                if (hdr.type == DT && hdr.unit == unit &&
                    hdr.stream == stream && hdr.channel == channel) {
                    _reads[hstream].rate = (REAL32) (((REAL64) length / (hdr.time - time) * 10) + 0.5) / 10;
                    if (!IsValidRate(_reads[hstream].rate)) {
                        _archive[_reads[hstream].harchive].last_error = ARC_NO_RATE;
                        return (FALSE);
                    }
                    if (!FileRewind(_reads[hstream].file)) {
                        _archive[_reads[hstream].harchive].last_error = ARC_FILE_IO_ERROR;
                        return (FALSE);
                    }
                    ArchiveLog(ARC_LOG_ERRORS, "Warning: sampling rate (%.1fsps) is derived DT times, EH/ET packets will be incomplete!",
                        _reads[hstream].rate);
                    
                    /* Store rate in packet */
                    memset((UINT8 *) (&_reads[hstream].eh) + 24, ' ', sizeof(RF_PACKET) - 24);
                    if (_reads[hstream].rate < 1)
                        sprintf(string, "%-4.1f", _reads[hstream].rate);
                    else
                        sprintf(string, "%-4hu", (UINT16)_reads[hstream].rate);
                    for (i = 0; i < 4; i++) {
                        if (string[i])
                            _reads[hstream].eh.pac.eh.rate[i] = string[i];
                    }
                    /* Set packet length so this won't be confused with 120 data */
                    _reads[hstream].eh.hdr.bpp[0] = 0x02;
                    _reads[hstream].eh.hdr.bpp[1] = 0x88;
                    break;
                }
            }
        }
debugf("CEH derived: %.1f...", _reads[hstream].rate);
    }

    /* Restore position if it was saved above */
    if (position != VOID_INT32) {
        if (!FileSeekAbsolute(_reads[hstream].file, position)) {
            _archive[_reads[hstream].harchive].last_error = ARC_FILE_IO_ERROR;
            return (FALSE);
        }
    }

    /* Find first data packet */
    if (!GetNextDataPacket(hstream, &hdr, packet))
        return (FALSE);

    /* Fixup an event header */
    _reads[hstream].out_seq = 0;
    _reads[hstream].out_evn = hdr.evn_no;
    FixupHeaderTrailer(hstream, EH);

    /* Unget the packet */
    _reads[hstream].in_seq = VOID_UINT16;
    if (!UnReadPacket(_reads[hstream].file)) {
        _archive[_reads[hstream].harchive].last_error = ARC_FILE_IO_ERROR;
        return (FALSE);
    }

    /* EH is in user buffer and we're ready to read the first data packet */
    _reads[hstream].out_seq++;
    memcpy(packet, &_reads[hstream].eh, RF_PACKET_SIZE);

    return (TRUE);
}  /* end CreateEventHeader() */

/*---------------------------------------------------------------------*/
BOOL GetNextDataPacket(H_STREAM hstream, RF_HEADER * hdr, RF_PACKET * packet)
{
    TIME_RANGE time;

    ASSERT(packet != NULL);

    _archive[_reads[hstream].harchive].last_error = ARC_NO_ERROR;

    /* Return the next in range data packet */
    while (TRUE) {
debugf("GNDP read(%u)...", hstream);
        if (!ReadPacket(_reads[hstream].file, hdr, packet)) {
            /* Deal with read error... */
debugf("GNDP can't...");
            if (FileError()) {
                _archive[_reads[hstream].harchive].last_error = ARC_FILE_IO_ERROR;
                return (FALSE);
            }
            /* If end of file, move on to the next one... */
            if (!FileClose(_reads[hstream].file)) {
                _archive[_reads[hstream].harchive].last_error = ARC_FILE_IO_ERROR;
                return (FALSE);
            }
debugf("GNDP next event...");
            if (!ArchiveFindNextEvent(_reads[hstream].harchive, &_reads[hstream].criteria,
                    &_reads[hstream].event)) {
                _archive[_reads[hstream].harchive].last_error = ARC_END_OF_DATA;
debugf("GNDP failed...");
                return (FALSE);
            }
            if ((_reads[hstream].file = FileOpenForRead(_reads[hstream].event.filespec)) == VOID_H_FILE) {
                _archive[_reads[hstream].harchive].last_error = ARC_FILE_IO_ERROR;
                return (FALSE);
            }
            continue;
        }
debugf("GNDP pkt %u:%04hX?%04hX...", hdr->type, hdr->seq, _reads[hstream].in_seq);

        /* Packet sequence checking */
        if (_reads[hstream].in_seq == VOID_UINT16)
            _reads[hstream].in_seq = hdr->seq;
        else if (_reads[hstream].in_seq != hdr->seq) {
debugf("GNDP bad seq...");
            _reads[hstream].in_seq = VOID_UINT16;
            if (!UnReadPacket(_reads[hstream].file)) {
debugf("GNDP can't UNread");
                _archive[_reads[hstream].harchive].last_error = ARC_FILE_IO_ERROR;
                return (FALSE);
            }
debugf("quit");
            _archive[_reads[hstream].harchive].last_error = ARC_SEQUENCE_BREAK;
            return (FALSE);
        }
        if (hdr->type == ET)
            _reads[hstream].in_seq = 0;
        else
            _reads[hstream].in_seq = ExpectedSeqNumber(hdr->seq);

        /* Is is a DT packet? */
        if (hdr->type == DT) {
            /* Is it from an unmasked channel? */
            if (_reads[hstream].criteria.chn_mask & (0x0001 << (hdr->channel - 1))) {
                time.earliest = hdr->time;
                time.latest = hdr->time + ((REAL64) hdr->length / (REAL64) _reads[hstream].rate);
debugf("GNDP pkt times: %.3f:%.3f (%u:%.1f)...", time.earliest, time.latest, hdr->length, _reads[hstream].rate);

                /* Is latest time < earliest specified? */
                if (!UndefinedTime(_reads[hstream].criteria.time.earliest) &&
                    time.latest < _reads[hstream].criteria.time.earliest)
                    continue;

                /* Is earliest > latest specified */
                if (!UndefinedTime(_reads[hstream].criteria.time.latest) &&
                    time.earliest > _reads[hstream].criteria.time.latest) {
                    /* Clear channel flag */
                    _reads[hstream].channels &= ~(0x0001 << (hdr->channel - 1));

                    /* Are all channels done? */
                    if (_reads[hstream].channels == 0x0000) {
                        _archive[_reads[hstream].harchive].last_error = ARC_END_OF_DATA;
debugf("GNDP EOD");
                        return (FALSE);
                    }

                    continue;
                }

debugf("GNDP good...");
                /* Set the channel flag */
                _reads[hstream].channels |= (0x0001 << (hdr->channel - 1));

                /* Track time range of retrieved data */
                UpdateTimeRange(&_reads[hstream].time, time.earliest);
                UpdateTimeRange(&_reads[hstream].time, time.latest);

                /* Fixup the outbound sequence number */
debugf("GNDP update pkt seq");
                FixupSequenceNumber(hstream, packet, hdr->type);
                _reads[hstream].out_seq = ExpectedSeqNumber(_reads[hstream].out_seq);

                break;
            }
        }
    }

    return (TRUE);
}

/*---------------------------------------------------------------------*/
VOID FixupSequenceNumber(H_STREAM hstream, RF_PACKET * packet, UINT8 type)
{
    CHAR string[8];
    INT32 i;

    if (_reads[hstream].out_seq == VOID_UINT16)
        return;

    if (type == EH || type == DT || type == ET) {
        /* Event number */
        sprintf(string, "%04u", _reads[hstream].out_evn);
        for (i = 0; i < 2; i++) {
            packet->pac.eh.event_num[i] = ((BCD) string[i * 2] - 0x30) << 4;
            packet->pac.eh.event_num[i] += (BCD) string[(i * 2) + 1] - 0x30;
        }
    }

    /* Sequence number */
    sprintf(string, "%04u", _reads[hstream].out_seq);
    for (i = 0; i < 2; i++) {
        packet->hdr.packet[i] = ((BCD) string[i * 2] - 0x30) << 4;
        packet->hdr.packet[i] += (BCD) string[(i * 2) + 1] - 0x30;
    }

    return;
}

/****************************************************************************    
	Purpose:	Adjust an EH or ET packet
	Returns:	Nothing                                                               
	Revised:                                                                      
		28Sep05	---- (pld) put header on & add debugging msgs
===========================================================================*/    
VOID FixupHeaderTrailer(H_STREAM hstream, UINT8 type)
{
    CHAR string[32], string2[32];
    INT32 i;
    MSTIME_COMP time;
    RF_PACKET *packet;

debugf("\nFixup %s...", (type == EH? "EH" : "ET"));

    packet = &_reads[hstream].eh;
debugf("HT pkt %p...", packet);

    /* Type */
    packet->hdr.type[0] = 'E';
    if (type == EH)
        packet->hdr.type[1] = 'H';
    else
        packet->hdr.type[1] = 'T';

debugf("HT deconstruct time...");
    /* Year */
    DecomposeMSTime(_reads[hstream].time.earliest, &time);
debugf("%04u:%03u:%02u:%02u:%06.3f...", time.year, time.doy, time.hour, time.minute, time.second);
    sprintf(string, "%02d", (time.year < 2000 ? time.year - 1900 : time.year - 2000));
    packet->hdr.year = ((BCD) string[0] - 0x30) << 4;
    packet->hdr.year += (BCD) string[1] - 0x30;

    if (type == EH) {
        /* Unit */
        packet->hdr.unit = htons(_reads[hstream].criteria.unit);

        /* Stream */
        sprintf(string, "%02u", _reads[hstream].criteria.stream - 1);
        packet->pac.eh.stream = ((BCD) string[0] - 0x30) << 4;
        packet->pac.eh.stream += (BCD) string[1] - 0x30;

debugf("HT reconstruct EH time...");
        /* Time */
        sprintf(string, "%03d%02d%02d%05d", time.doy, time.hour, time.minute,
            (INT32) ((time.second / SI_MILLI) + 0.5));
        for (i = 0; i < 6; i++) {
            packet->hdr.time[i] = ((BCD) string[i * 2] - 0x30) << 4;
            packet->hdr.time[i] += (BCD) string[(i * 2) + 1] - 0x30;
        }
        sprintf(packet->pac.eh.tr_message, "Trigger time = %04d%s", time.year, string);
        sprintf(string2, "%04d%s", time.year, string);
        memcpy(packet->pac.eh.tr_time, string2, 16);
        memcpy(packet->pac.eh.ist, string2, 16);
    }
    else {
        /* If ET... */
debugf("HT reconstruct ET time...");
        sprintf(string2, "%04d%03d%02d%02d%05d", time.year, time.doy,
            time.hour, time.minute, (INT32) ((time.second / SI_MILLI) + 0.5));
        memcpy(packet->pac.eh.ist, string2, 16);
        DecomposeMSTime(_reads[hstream].time.latest, &time);
debugf("%04u:%03u:%02u:%02u:%06.3f...", time.year, time.doy, time.hour, time.minute, time.second);
        sprintf(string2, "%04d%03d%02d%02d%05d", time.year, time.doy,
            time.hour, time.minute, (INT32) ((time.second / SI_MILLI) + 0.5));
        memcpy(packet->pac.eh.de_tr_time, string2, 16);
        memcpy(packet->pac.eh.lst, string2, 16);
    }

    /* Sequence number */
debugf("HT update sequence...");
    FixupSequenceNumber(hstream, packet, type);

debugf("HT done\n");
}  /* end FixupHeaderTrailer(() */

/*---------------------------------------------------------------------*/
BOOL ReadPacket(H_FILE file, RF_HEADER * hdr, RF_PACKET * packet)
{
    UINT32 bytes;

    ASSERT(hdr != NULL);
    ASSERT(packet != NULL);

    bytes = RF_PACKET_SIZE;
    if (!FileRead(file, packet, &bytes))
        return (FALSE);

    RFDecodeHeader(packet, hdr);

    return (TRUE);
}

/*---------------------------------------------------------------------*/
BOOL UnReadPacket(H_FILE file)
{

    if (FileSeekRelative(file, -(INT32) RF_PACKET_SIZE))
        return (TRUE);

    return (FALSE);
}

/*---------------------------------------------------------------------*/
BOOL ValidateStreamHandle(H_STREAM hstream)
{

    if (hstream > MAX_OPEN_STREAMS) {
        _archive[_reads[hstream].harchive].last_error = ARC_NO_ERROR;
        _archive_error = ARC_BAD_STREAM_HANDLE;
        return (FALSE);
    }

    if (!ValidateHandle(_reads[hstream].harchive))
        return (FALSE);

    return (TRUE);
}

/*---------------------------------------------------------------------*/
H_STREAM GetNextStreamHandle(H_ARCHIVE harchive)
{
    H_STREAM hstream;

    _archive[harchive].last_error = ARC_NO_ERROR;

    hstream = VOID_H_STREAM;
    for (hstream = 0; hstream < MAX_OPEN_STREAMS; hstream++) {
        if (_reads[hstream].harchive == VOID_H_ARCHIVE)
            return (hstream);
    }

    _archive[harchive].last_error = ARC_NO_STREAM_HANDLE;
    return (VOID_H_STREAM);
}

/*---------------------------------------------------------------------*/
VOID InitRead(H_STREAM hstream)
{

    if (hstream > MAX_OPEN_STREAMS)
        return;

    _reads[hstream].harchive = VOID_H_ARCHIVE;
    InitStream(&_reads[hstream].criteria);
    _reads[hstream].options = 0x00000000;
    InitEvent(&_reads[hstream].event);
    _reads[hstream].state = STA_STARTING;
    _reads[hstream].time.earliest = VOID_TIME;
    _reads[hstream].time.latest = VOID_TIME;
    _reads[hstream].out_evn = VOID_EVENT_NO;
    _reads[hstream].out_seq = VOID_SEQ_NO;
    _reads[hstream].channels = 0;
    _reads[hstream].in_seq = VOID_SEQ_NO;
    _reads[hstream].file = VOID_H_FILE;

    return;
}

/* Revision History
 *
 * $Log$
 * Revision 1.2  2005/12/23 12:34:57  andres
 * upgrade to new version with Steim2 support
 *
 * Revision 2.0  2005/10/07 22:24:53  pdavidson
 * Finish Steim2 support.

 * Bug fixes in 0.1 sps, aux data (stream 9) support.

 * Handle all trigger types in EH/ET decoding.

 * Promote archive API, modified programs to v2.0.

 * DOES NOT INCLUDE modifications to RTP log or client protocol.
 *
 * Revision 1.2  2002/01/18 17:53:22  nobody
 * changed interpretation of unit ID from BCD to binary
 *
 * Revision 1.1.1.1  2000/06/22 19:13:09  nobody
 * Import existing sources into CVS
 *
 */
