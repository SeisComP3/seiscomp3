/***************************************************************************** 
 * proto_sadc.cc
 * 
 * Protocol implementation for the SADC18/20/30
 *
 * (c) 2006 Henrik Thoms - GFZ Potsdam - thoms(at)gfz-potsdam.de
 *
 * Created: 12/01/06
 *
 * Comments and code from Mauro Mariotti - SARA elctronic instruments s.r.l.
 * marked as:  MMS    and added on 23/03/2011
 * to be checked and tested
 *
 *
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2, or (at your option) any later
 * version. For more information, see http://www.gnu.org/
 *****************************************************************************/

#include <iomanip>
#include <vector>
#include <algorithm>
#include <cstdlib>
#include <cstddef>
#include <string>
#include <ctime>
#include <cstdio>

#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
// #include <error.h>

#include "qtime.h"

#include "utils.h"
#include "cppstreams.h"
#include "serial_plugin.h"
#include "plugin_channel.h"
#include "diag.h"

#include "little-endian.h"

using namespace std;
using namespace Utilities;
using namespace CPPStreams;
using namespace SeedlinkPlugin;

namespace {

    const int NCHAN = 16;
    const unsigned char DATA_HEADER[]  = {
	0x82, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89,
	0x8a, 0x8b, 0x8c, 0x8d, 0x8e, 0x8f, 0x90, 0x91 };
    
/* **************************************************************************
 * Data Structures
 * **************************************************************************/
    
    // Header of the data segments
    struct SegmentHeader
    {
	unsigned char commandId;
    } PACKED;

    // Time Segment containing date and time
    struct TimeSegmentFull
    {
	unsigned char year, month, day, sec, min, hour, extra, end;
    } PACKED;

    // Time Segment containing just the date
    struct TimeSegmentPartial
    {
	unsigned char sec, min, hour, extra, end;
    } PACKED;

    // Data segment for 24 bit digitizer
    struct DataSegment24Bit
    {
	unsigned char low, middle, high, end;
    } PACKED;

    // Data segment for 18 and 16 bit digitizer
    struct DataSegment18and16Bit
    {
	unsigned char low, high, end;
    } PACKED;


/* ***************************************************************************
 * SADCdataProtocol
 * ***************************************************************************/
    
    class SADCdataProtocol: public Proto
    {
    public:
	SADCdataProtocol(const string &myName):
	    _countdownInitVal(360), _countdown(_countdownInitVal),
	    _samplingRate(100), _currentNrOfChannels(0),
	    _timeOut(10), _maxSamplingRate(200), _timeSegment(NULL),  _dataSegment(NULL),
	    _segmentHeaderSize(1), _timeSegmentSize(0), _dataSegmentSize(0),
	    _sadcChannels(NCHAN), decodeData(NULL), decodeTime(NULL)
	    {
		memset(&_segmentId, 0, _segmentHeaderSize);
	    }
	
	~SADCdataProtocol()
	    {
		if (_timeSegment != NULL) free(_timeSegment);
		if (_dataSegment != NULL) free(_dataSegment);
	    }
	
	void attach_output_channel(const string &source_id,
				   const string &channel_name, const string &station_name,
				   double scale, double realscale, double realoffset,
				   const string &realunit, int precision);
	void flush_channels();
	
	void start();
	
    private:
	const int                      _countdownInitVal;
	int                            _countdown;
	string                         _digitizerVersion;
	int                            _samplingRate;
	int                            _currentNrOfChannels;
	int                            _timeOut;
	int                            _maxSamplingRate;
	int                            _fd;
	SegmentHeader                  _segmentId;
	void                           *_timeSegment;
	void                           *_dataSegment;
	int                            _segmentHeaderSize;
	int                            _timeSegmentSize;
	int                            _dataSegmentSize;
	vector<rc_ptr<OutputChannel> > _sadcChannels;
	
	/** Pointer to member functions */
	int (SADCdataProtocol::*decodeData)(void *);
	string (SADCdataProtocol::*decodeTime)(void *);
	void (SADCdataProtocol::*setTimeFromGPS)(const void *);
	void (SADCdataProtocol::*statusMessage)(const void *);
		
	void   init();
	void   initSegments();
	void   do_start();
	int    decodeData24Bit(void *);
	int    decodeData18Bit(void *);
	int    decodeData16Bit(void *);
	void   statusMessageTimeFull(const void*);
	string decodeTimeFull(void *);
	void   setTimeFromGPSFull(const void *);
	void   setTimeFromGPSPartial(const void *);
	string getFirmwareVersion();
	void   setGMT(unsigned char);
	void   setTime(unsigned char sec=0x00, unsigned char min=0x00, unsigned char hour=0x00);
	void   setDate(unsigned char year=0x00, unsigned char month=0x00, unsigned char day=0x00);
	void   setCrystalErrorCompensationTrim(unsigned char lowTrim=0xFF, unsigned char medTrim=0xFF,
					       unsigned char highTrim=0xFF, unsigned char dirTrim=0xFF);
	void   setSamplingRate3Channels(int samplingRate=100);
	void   setSamplingRate4Channels(int samplingRate=100);
	void   setSamplingRate16Channels(int smaplingRate=100,
				       unsigned char channelsLow=0x00,
				       unsigned char channelsHigh=0x00);
	bool   readReturnStatus(const unsigned char acknowledgeByte);
	bool   sanityCheck(unsigned char *dataSegment, int size) {
	    if (dataSegment[size-1] < 0xF0)
		return false;
	    return true;
	}
    };
    
    /**************************************************************************
     * public definition
     **************************************************************************/
    void SADCdataProtocol::attach_output_channel(const string &source_id,
						 const string &channel_name, const string &station_name,
						 double scale, double realscale, double realoffset,
						 const string &realunit, int precision)
    {
	int n;
	char *tail;

	n = strtoul(source_id.c_str(), &tail, 10);
    
	if(*tail || n >= NCHAN)
	    throw PluginADInvalid(source_id, channel_name);

	if(_sadcChannels[n] != NULL)
	    throw PluginADInUse(source_id, _sadcChannels[n]->channel_name);
	
	_sadcChannels[n] = new OutputChannel(channel_name, station_name,
					     dconf.zero_sample_limit, scale);
    }

    void SADCdataProtocol::flush_channels()
    {
	for(int n = 0; n < NCHAN; ++n)
	{
	    if(_sadcChannels[n] == NULL) continue;
	    _sadcChannels[n]->flush_streams();
	}
    }

    void SADCdataProtocol::start()
    {
	_fd = open_port(O_RDWR);
	// This may hang forever
	// sync_data(_fd);

	try {
	    do_start();
	}
	catch(PluginError &e) {
	    seed_log << "closing device" << endl;
	    close(_fd);
	    throw;
	}

	seed_log << "closing device" << endl;
	close(_fd);
    }

    /* *************************************************************************
     * private definitions
     * *************************************************************************/

    void SADCdataProtocol::initSegments()
    {
	// allocate time segment
	if ((_timeSegment = malloc(_timeSegmentSize)) == NULL)
	    throw bad_alloc();
	memset(_timeSegment, 0, _timeSegmentSize);	    
	
	// allocate data segment
	if ((_dataSegment = malloc(_dataSegmentSize)) == NULL)
	    throw bad_alloc();
	memset(_dataSegment, 0, _dataSegmentSize);
    }
    
    /**
     * Configures the plugin in correspondence to the firmaware version.
     */
    void SADCdataProtocol::init()
    {
        logs(LOG_INFO) << "Trying to detect firmware version" << endl;
        _digitizerVersion = getFirmwareVersion();
	logs(LOG_INFO) << "Firmware version: " << _digitizerVersion << endl;

	// We are using a gps time source, so no GMT corection is
	// necessary. 
	setGMT(0);
	
	setCrystalErrorCompensationTrim();
	    
        time_t t = time(NULL);
        tm *ptm = gmtime(&t);
        EXT_TIME et;

        et.year = ptm->tm_year + 1900;
        et.month = ptm->tm_mon + 1;
        et.day = ptm->tm_mday;
        et.hour = ptm->tm_hour;
        et.minute = ptm->tm_min;
        et.second = ptm->tm_sec;
        et.usec = 0;
        et.doy = mdy_to_doy(et.month, et.day, et.year);
        digitime.it = ext_to_int(et);
        
	if(dconf.use_pctime_if_no_gps)
        {
	    setTime(et.hour, et.minute, et.second);
	    setDate(et.year % 100, et.month, et.day);
            digitime.valid = true;
            digitime.exact = false;

	    for(int n = 0; n < NCHAN; ++n)
	    {
	        if(_sadcChannels[n] == NULL) continue;
	        _sadcChannels[n]->set_timemark(digitime.it, 0, digitime.quality);
	    }
        }
        else
        {
            logs(LOG_INFO) << "Waiting for time segment" << endl;
            digitime.valid = false;
            digitime.exact = true;
        }
        
	if (_digitizerVersion == "V300")
	{
	    _currentNrOfChannels = 16;
	    
	    // setting up pointers to member functions
	    decodeData = &SADCdataProtocol::decodeData16Bit;
	    decodeTime = &SADCdataProtocol::decodeTimeFull;
	    setTimeFromGPS = &SADCdataProtocol::setTimeFromGPSFull;
	    statusMessage = &SADCdataProtocol::statusMessageTimeFull;
	    
	    // set segment sizes
	    _segmentHeaderSize = sizeof(SegmentHeader);
	    _timeSegmentSize = sizeof(TimeSegmentFull);
	    _dataSegmentSize = sizeof(DataSegment18and16Bit);

	    _samplingRate = 100;
	    setSamplingRate16Channels(_samplingRate);
	}
/*	else if (_digitizerVersion == "V200")   MMS previous code*/
	else if (_digitizerVersion == "V200" ||   /* MMS new code */
					 _digitizerVersion == "V201" || 
					 _digitizerVersion == "V202" || 
					 _digitizerVersion == "V203" ||
					 _digitizerVersion == "V204" ||
			  /* _digitizerVersion == "V205" ||    version 2.05 is capable to sample at 600Hz and has different data packets */
					 _digitizerVersion == "V206")   // new code
	{
	    _currentNrOfChannels = 3;
	    
	    // setting up pointers to member functions
	    decodeData = &SADCdataProtocol::decodeData24Bit;
	    decodeTime = &SADCdataProtocol::decodeTimeFull;
	    setTimeFromGPS = &SADCdataProtocol::setTimeFromGPSFull;
	    statusMessage = &SADCdataProtocol::statusMessageTimeFull;
	    	    
	    // set segment sizes
	    _segmentHeaderSize = sizeof(SegmentHeader);
	    _timeSegmentSize = sizeof(TimeSegmentFull);
	    _dataSegmentSize = sizeof(DataSegment24Bit);
	    	    
	    _samplingRate = 100;
	    setSamplingRate3Channels(_samplingRate);
	}
	else if (_digitizerVersion == "V181")
	{
	    _currentNrOfChannels = 4;

	    // setting up pointers to member functions
	    decodeData = &SADCdataProtocol::decodeData18Bit;
	    decodeTime = &SADCdataProtocol::decodeTimeFull;
	    setTimeFromGPS = &SADCdataProtocol::setTimeFromGPSFull;
	    statusMessage = &SADCdataProtocol::statusMessageTimeFull;
	    
	    // set segment sizes
	    _segmentHeaderSize = sizeof(SegmentHeader);
	    _timeSegmentSize = sizeof(TimeSegmentFull);
	    _dataSegmentSize = sizeof(DataSegment18and16Bit);

	    _samplingRate = 100;
	    setSamplingRate4Channels(_samplingRate);
	}
	else if (_digitizerVersion == "V180")
	{
	    logs(LOG_ERR) << "Digitizerversion " << _digitizerVersion << "is _NOT_ supported" << endl;
	    exit(0);
	}
	else if (_digitizerVersion == "V162")
	{
	    _currentNrOfChannels = 4;
	    
	    // setting up pointers to member functions
	    decodeData = &SADCdataProtocol::decodeData16Bit;
	    decodeTime = &SADCdataProtocol::decodeTimeFull;
	    setTimeFromGPS = &SADCdataProtocol::setTimeFromGPSFull;
	    statusMessage = &SADCdataProtocol::statusMessageTimeFull;
	    
	    // set segment sizes
	    _segmentHeaderSize = sizeof(SegmentHeader);
	    _timeSegmentSize = sizeof(TimeSegmentFull);
	    _dataSegmentSize = sizeof(DataSegment18and16Bit);

	    _samplingRate = 100;
	    setSamplingRate4Channels(_samplingRate);
	}
	else if (_digitizerVersion == "V161")
	{
	    logs(LOG_ERR) << "Digitizerversion " << _digitizerVersion << "is _NOT_ supported" << endl;
	    exit(0);
	}
	else if (_digitizerVersion == "V151")
	{
	    logs(LOG_ERR) << "Digitizerversion " << _digitizerVersion << "is _NOT_ supported" << endl;
	    exit(0);
	}
	    
	initSegments();

	seed_log << ident_str << SEED_NEWLINE
	         << "Digitizer firmware: " << _digitizerVersion << SEED_NEWLINE
	         << "Number of channels: " << _currentNrOfChannels << SEED_NEWLINE
	         << "Sample rate: " << _samplingRate << endl;
    }

    inline void SADCdataProtocol::statusMessageTimeFull(const void *timeSegment)
    {
	const TimeSegmentFull * ts = static_cast<const TimeSegmentFull*>(timeSegment);

	if ( ts->sec == 1 && ts->min == 0 && ts->hour == 0 )
	{
	seed_log << ident_str << SEED_NEWLINE
	         << "Digitizer firmware: " << _digitizerVersion << SEED_NEWLINE
	         << "Number of channels: " << _currentNrOfChannels << SEED_NEWLINE
	         << "Sample rate: " << _samplingRate << endl;
	}
    }
    
    bool SADCdataProtocol::readReturnStatus(const unsigned char acknowledgeByte)
    {
	int size = sizeof(acknowledgeByte);
	unsigned char returnStatus = 0x00;
	bool success = false;
 	time_t startTime = 0, currentTime = 0, *status = NULL;
 	startTime = time(status);
		
	while (!terminate_proc)
	{
	    currentTime = time(status);

	    // cout << "diffTime: " << difftime(currentTime, startTime) << endl;
	    if (difftime(currentTime, startTime) >= _timeOut) break;
	    
	    if(read_port(_fd, &returnStatus, size) == 0) continue;
	    if (returnStatus != acknowledgeByte) continue;

	    success = true;
	    break;
	}
	return success;
    }
    
    string SADCdataProtocol::getFirmwareVersion()
    {
	int commandSize = 6;
	const unsigned char command[] =
	    { 0x81, 0x00, 0x00, 0x00, 0x00, 0x00 };
	char answer[4];
	
	while (!terminate_proc)
	{
	    write(_fd, command, commandSize);
	    
	    if (read_port(_fd, answer, 1) == 0) continue;
	    if (answer[0] != 'V') continue;
	    if (read_port(_fd, &(answer[1]), 3) == 0) continue;
	    string ret(answer, answer + 4);

// MMS. Old code
/*	    if(ret == "V300" || ret == "V200" || ret == "V181" ||
		ret == "V180" || ret == "V162" || ret == "V161" ||
		ret == "V150") 
		return ret;  */

// MMS new code
	    if( ret == "V300" || ret == "V200" || ret == "V201" || 
	    	  ret == "V202" || ret == "V203" || ret == "V204" || 
	    	  ret == "V206" || ret == "V181" || ret == "V180" || 
	    	  ret == "V162" || ret == "V161" || ret == "V150") 
							return ret;
	}

        logs(LOG_WARNING) << "Could not read firmware version" << endl;
        return string();
    }

    void SADCdataProtocol::setTimeFromGPSFull(const void *timeSegmentFull)
    {
	const TimeSegmentFull *timeSegment = static_cast<const TimeSegmentFull*>(timeSegmentFull);
	EXT_TIME et;
	
	// In the versions 2.00, 2.01, 2.02, and 2.04
	// exists a bug in the a/d firmware which is manifested very rarely
	// if happen it happen at the first second of the new day.
	// It may happen that at the time packet issue this sequence:
	//
	// 2010/03/01 23:59:58
	// 2010/03/01 23:59:59
	// 2010/03/01 00:00:00   <--- wrong date! It have to be 03/02 and not 03/01
	// 2010/03/02 00:00:01
	// 2010/03/02 00:00:02
	//
	// this exception can be trapped and corrected keeping in account that
	// if the clock passed the midnight AND the day is the day before
	// the date can be corrected to the new day
	//
	
	
	et.year = timeSegment->year + 2000;
	et.month = timeSegment->month;
	et.day = timeSegment->day;
	et.hour = timeSegment->hour;
	et.minute = timeSegment->min;
	et.second = timeSegment->sec;
	et.usec = 0;
	et.doy = mdy_to_doy(et.month, et.day, et.year);
	digitime.it = add_time(ext_to_int(et), 0, dconf.time_offset);
    }

    void SADCdataProtocol::setTimeFromGPSPartial(const void *timeSegmentPartial)
    {
	logs(LOG_WARNING) << "setTimeFromGPSPartial() NOT IMPLEMENTED YET" << endl;
    }
    
    /**
     * Decodes data from the 24 bit digitizer (V200)
     *
     * PARAMETER:
     * ptr:	void pointer to an DataSegment24Bit Segment
     *
     * RETURN VALUE:
     * int:	Integer which hold the decoded data
     */
    int SADCdataProtocol::decodeData24Bit(void *ptr)
    {
	DataSegment24Bit* data = static_cast<DataSegment24Bit*>(ptr);
	
	data->low = data->low + ((data->end & 1)  << 7);
	data->middle = data->middle + ((data->end & 2)  << 6);
	
  	int tmp = 0;
	int value = 0;
  	tmp = ((data->high << 16) + (data->middle << 8 ) + data->low);
  	if (data->end & 4)
	    value = -8388608 + tmp;
 	else
  	    value = tmp;
	
  	return value;
    }

    /**
     * Decodes data from the 18 bit digitizer (V180/V181)
     *
     * PARAMETER:
     * ptr:	void pointer to an DataSegment18Bit Segment
     *
     * RETURN VALUE:
     * int:	Integer which hold the decoded data
     */
    int SADCdataProtocol::decodeData18Bit(void* ptr)
    {
	DataSegment18and16Bit *data = static_cast<DataSegment18and16Bit*>(ptr);

	data->low = data->low + ((data->end & 1)  << 7);
	data->high = data->high + ((data->end & 2)  << 6);
	
	int value = (data->high << 8) + data->low;
	
	if (data->end & 4) 
	    value += 65536;
	if (data->end & 8) 
	    value += -131072;
		
	return value;
    }
    
    /**
     * Decodes data from the 16 bit digitizer (V162/161)
     *
     * PARAMETER:
     * ptr:	void pointer to an DataSegment16Bit Segment
     *
     * RETURN VALUE:
     * int:	Integer which hold the decoded data
     */
    int SADCdataProtocol::decodeData16Bit(void* ptr)
    {
	DataSegment18and16Bit *data = static_cast<DataSegment18and16Bit*>(ptr);
	
	if (data->end == 253)
	    data->low += 128;
	else if (data->end == 254)
	    data->high += 128;
	else if (data->end == 255)
	{
	    data->low += 128;
	    data->high += 128;
	}
	short value = (data->high << 8) + data->low;
	
	return value;
    }
    
    /**
     * Decodes the time and date.
     *
     * PARAMETER:
     * time:	void pointer to a TimeSegmentFull Segment
     *
     * RETURN VALUE
     * string:  A string which contains both time and date.
     */
    string SADCdataProtocol::decodeTimeFull(void *time)
    {
	TimeSegmentFull* ts = static_cast<TimeSegmentFull*>(time);
	const char* formatStr = "%02d:%02d:%02d %02d.%02d.20%02d";
	const int strLen = 21;
	char timeStr[strLen];

	snprintf(timeStr, strLen, formatStr,
		 ts->hour, ts->min, ts->sec,
		 ts->day, ts->month, ts->year);

	return string(timeStr);
    }
    
    void SADCdataProtocol::setGMT(unsigned char GMT = 0x00)
    {
	unsigned char acknowledgeByte = 0xF8;
	unsigned char commandGMT[] =
	    { 0x82, GMT, 0x00, 0x00, 0x00, 0x00 };
	int commandSize = 6;
	
	while(!terminate_proc)
	{
	    if (write(_fd, commandGMT, commandSize) == -1 )
	        logs(LOG_WARNING) << "Could not write to filedescriptor - ERROR: "
	    	          << strerror(errno) << endl;

	    // readReturnStatus
	    if (!readReturnStatus(acknowledgeByte))
	    {
	        logs(LOG_WARNING) << "[ setGMT ]  Acknowledgebyte _NOT_ found " << endl;
		continue;
	    }
	    else
	    {
	        logs(LOG_INFO) << "[ setGMT ]  Acknowledgebyte _FOUND_ " << endl;
		break;
	    }
	}
    }

    void SADCdataProtocol::setTime(unsigned char sec, unsigned char min, unsigned char hour)
    {
	unsigned char ackByte = 0xF8;
	unsigned char timeCmd[] =
	    { 0x83, sec, min, hour, 0x00, 0x00 };
	int cmdSize = 6;
	
	while(!terminate_proc)
	{
	    if (write(_fd, timeCmd, cmdSize) == -1)
	        logs(LOG_WARNING) << "[ setTime ] Could not write to filedescriptor - _ERROR_: "
			      << strerror(errno) << endl;
	    // readReturnStatus
	    if (!readReturnStatus(ackByte))
	    {
	        logs(LOG_WARNING) << "[ setTime ]  Acknowledgebyte _NOT_ found " << endl;
		continue;
	    }
	    else
	    {
	        logs(LOG_INFO) << "[ setTime ]  Acknowledgebyte _FOUND_ " << endl;
		break;
	    }
	}
    }

    void SADCdataProtocol::setDate(unsigned char year, unsigned char month, unsigned char day)
    {
	unsigned char ackByte = 0xF8;
	unsigned char dateCmd[] =
	    { 0x87, year, month, day, 0x00, 0x00 };
	int cmdSize = 6;
	
	while(!terminate_proc)
	{
	    if (write(_fd, dateCmd, cmdSize) == -1 )
	        logs(LOG_WARNING) << "[ setDate ] Could not write to filedescriptor - _ERROR_: "
			      << strerror(errno) << endl;

	    // readReturnStatus
	    if (!readReturnStatus(ackByte))
	    {
	        logs(LOG_WARNING) << "[ setDate ]  Acknowledgebyte _NOT_ found " << endl;
		continue;
	    }
	    else
	    {
	        logs(LOG_INFO) << "[ setDate ]  Acknowledgebyte _FOUND_ " << endl;
		break;
 	    }
	}
    }
    
	void SADCdataProtocol::setCrystalErrorCompensationTrim(unsigned char lowTrim,
	                                                       unsigned char medTrim,
	                                                       unsigned char highTrim,
	                                                       unsigned char dirTrim)
	{
	unsigned char ackByte = 0xF8;
	unsigned char dateCmd[] =
	    { 0x85, lowTrim, medTrim, highTrim, dirTrim, 0x00 };
	int cmdSize = 6;
	
	while(!terminate_proc)
	{
	    if (write(_fd, dateCmd, cmdSize) == -1 )
	        logs(LOG_WARNING) << "[ setCrystalErrorCompensationTrim ]"
		    "Could not write to filedescriptor - _ERROR_: "
			      << strerror(errno) << endl;

	    // readReturnStatus
	    if (!readReturnStatus(ackByte))
	    {
	        logs(LOG_WARNING) << "[ setCrystalErrorCompensationTrim ]  Acknowledgebyte _NOT_ found " << endl;
		continue;
	    }
	else
	    {
	    	logs(LOG_INFO) << "[ setCrystalErrorCompensationTrim ]  Acknowledgebyte _FOUND_ " << endl;
		break;
	    }
	}
    }
    
    void SADCdataProtocol::setSamplingRate16Channels(int samplingRate,
						     unsigned char channelsLow,
						     unsigned char channelsHigh)
    {
	if (samplingRate > _maxSamplingRate || samplingRate < 0)
	{
		logs(LOG_WARNING) << "sample rate has to be between "
		                  << _maxSamplingRate << " and 0" << endl;
 	    return;
	}
	
	unsigned char spsX = 0x00;
	if (samplingRate > 0)
	    spsX = static_cast<unsigned char>(_maxSamplingRate / samplingRate);

	int commandSize = 6;
	unsigned char samplingCommand[] =
	    { 0x84, spsX, channelsLow, channelsHigh, 0x00, 0x00 };
	
	if (write(_fd, samplingCommand, commandSize) == -1 )
	    logs(LOG_WARNING) << "Could not write to filedescriptor (setSamplingRate)"
			      << " - ERROR: " << strerror(errno) << endl;
    }
    
    void SADCdataProtocol::setSamplingRate4Channels(int samplingRate)
    {
	if (samplingRate > _maxSamplingRate || samplingRate < 0)
	{
	    logs(LOG_WARNING) << "sample rate has to be between " << _maxSamplingRate << " and 0" << endl;
	    return;
	}
	
	unsigned char spsX = 0x00;
	if (samplingRate > 0)
	    spsX = static_cast<unsigned char>(_maxSamplingRate / samplingRate);

	int commandSize = 6;
	unsigned char samplingCommand[] =
	    { 0x84, spsX, spsX, spsX, spsX, 0x00 };
	
	if (write(_fd, samplingCommand, commandSize) == -1 )
	    logs(LOG_WARNING) << "Could not write to filedescriptor (setSamplingRate)"
		 << " - ERROR: " << strerror(errno) << endl;
    }

     void SADCdataProtocol::setSamplingRate3Channels(int samplingRate)
     {
	if (samplingRate > _maxSamplingRate || samplingRate < 0)
	{
	    logs(LOG_WARNING) << "sample rate has to be between " << _maxSamplingRate << " and 0" << endl;
	    return;
	}
	
	unsigned char spsX = 0x00;
	if (samplingRate > 0)
	    spsX = static_cast<unsigned char>(_maxSamplingRate / samplingRate);

	int commandSize = 6;
	unsigned char samplingCommand[] =
	    { 0x84, spsX, spsX, spsX, 0x00, 0x00 };
	
	if (write(_fd, samplingCommand, commandSize) == -1 )
	    logs(LOG_WARNING) << "Could not write to filedescriptor (setSamplingRate)"
		 << " - ERROR: " << strerror(errno) << endl;
    }
    
    /**
     * do_start() reads the data from the digitatizer, does the
     * processing and sends the results to the SeisComp server.
     */
    void SADCdataProtocol::do_start()
    {
	bool GPSlock = false;
	
	// Initialize the plugin according to the firmware version.
       	init();
	
	while (!terminate_proc)
 	{
	    // Read the segment header (command)
	    //
	    if (read_port(_fd, &_segmentId, _segmentHeaderSize) == 0)
		continue;
	    
	    // Time command
	    if (_segmentId.commandId == 0x81)
 	    {
		logs(LOG_DEBUG) << "Time segment found: " << hex << "0x"
			       << static_cast<int>(_segmentId.commandId) << dec << endl;
		if (read_port(_fd, _timeSegment, _timeSegmentSize) == 0)
		{
		    logs(LOG_WARNING) << "read timeout for TimeSegment" << endl;
		    continue;
		}
		
		if (!sanityCheck((unsigned char *)_timeSegment, _timeSegmentSize))
		{
		    logs(LOG_WARNING) << "sanity check for time segment failed: timeSgmentSize: "
			    << _timeSegmentSize << endl;
		    continue;
		}

		// Set digitime - important for seed logging/sending etc.
		(this->*setTimeFromGPS)(_timeSegment);
                if(!digitime.valid)
                {
                    logs(LOG_INFO) << "Got time segment" << endl;
                    digitime.valid = true;
                }
				
	        for(int n = 0; n < NCHAN; ++n)
	        {
	            if(_sadcChannels[n] == NULL) continue;
	            _sadcChannels[n]->set_timemark(digitime.it, 0, digitime.quality);
	        }

		logs(LOG_DEBUG) << "TIME: " << (this->*decodeTime)(_timeSegment) << endl;
		// Send status message every full hour
		(this->*statusMessage)(_timeSegment);
				
		// Check wether we have a GPS lock. Otherwise we
		// should do the time calculation on our own.
		//
		if (((char *)_timeSegment)[_timeSegmentSize - 2] & 32 && !GPSlock)
		{
		    logs(LOG_INFO) << "[ TIME ] " << (this->*decodeTime)(_timeSegment) << ", GPS _IS_ locked" << endl;
		    
		    GPSlock = true;
		    digitime.quality = 100;
		    _countdown = _countdownInitVal;
		}

		if (_countdown == 0)
		{
		    logs(LOG_INFO) << "[ TIME ] " << (this->*decodeTime)(_timeSegment) << ", GPS is _NOT_ locked" << endl;

		    GPSlock = false;
		    digitime.quality = dconf.unlock_tq;
		}
		if (_countdown >= 0)
		    _countdown--;
	    }
	    else if(digitime.valid)
	    {
		// Calculate channels index
		int i = static_cast<int>(_segmentId.commandId - DATA_HEADER[0]);
		
		if (i >= _currentNrOfChannels || i < 0) // invalid index
		{
		    logs(LOG_WARNING) << "UNKNOWN HEADER: " << hex << "0x"
		    		      << static_cast<int>(_segmentId.commandId)
				      << " for CHANNEL: " << dec << i << endl;
		}
		else 
		{
		    if (read_port(_fd, _dataSegment, _dataSegmentSize) == 0)
		    { 
			logs(LOG_WARNING) << "read timeout for channel: " << i << endl;
			continue;
		    }
		    if (!sanityCheck((unsigned char *)_dataSegment, _dataSegmentSize))
		    {
			logs(LOG_WARNING) << "sanity check for data segment failed - CHANNEL: " << i << endl;
			continue;
		    }
                    
		    int sample = (this->*decodeData)(_dataSegment);
		    logs(LOG_DEBUG) << "CHANNEL[" << i << "]: " << sample << endl;
		    _sadcChannels[i]->put_sample(sample);
		}
	    }
	}
    }
    
    RegisterProto<SADCdataProtocol> proto("sadc");
	
} // unnamed namespace
    
