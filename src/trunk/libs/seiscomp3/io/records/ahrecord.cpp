/***************************************************************************
 *   Copyright (C) by GFZ Potsdam                                          *
 *                                                                         *
 *   You can redistribute and/or modify this program under the             *
 *   terms of the SeisComP Public License.                                 *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   SeisComP Public License for more details.                             *
 ***************************************************************************/


#include <rpc/rpc.h>
#include <vector>
#include <string.h>
#include <iostream>
#include <fstream>

#include <seiscomp3/io/records/ahrecord.h>
#include <seiscomp3/core/arrayfactory.h>
#include <seiscomp3/core/typedarray.h>


typedef std::complex<float> ahcomplex;
using namespace std;


// if one of these is defined, some xdr routines are skipped and
// replaced by much faster equivalents.
# define IEEE_INTEL
//# define IEEE_SPARC

// The following are fixed for the AH format and must *never* be changed
#define HEADER_SIZE 1080
#define COMSIZE       80
#define LOGSIZE      202
#define CODESIZE       6
#define CHANSIZE       6
#define STYPESIZE      8
#define NEXTRA        21
#define NOCALPTS      30

#define AH_SUCCESS     0
#define AH_ERROR       1


namespace Seiscomp {

namespace IO {


static int ah_get(istream &is, AHRecord &rec);
static int ah_put(ostream &os, AHRecord const &rec);


IMPLEMENT_SC_CLASS_DERIVED(AHRecord, Record, "AHRecord");
REGISTER_RECORD(AHRecord, "ah");


AHRecord::AHRecord(string net, string sta, string loc, string cha,
                   Core::Time stime, double fsamp, int tqual,
	       Array::DataType dt, Hint h)
    : Record(dt,h,net,sta,loc,cha,stime,0,fsamp,tqual), _data(0), _gain(1)
{
	cal = vector<float>(120);
	for(int i=0; i<120; cal[i++] = 0.);
/*
//	*xcom = *ecom = *log = '\0';

	for(int i=0; i<COMSIZE; i++)
		com[i] = ecom[i] = '\0';
	for(int i=0; i<LOGSIZE; i++)
		log[i] = '\0';
*/
}

AHRecord::AHRecord(const AHRecord& rec)
	: Record(rec)
{
	_data = rec._data ? rec._data->clone() : NULL;
	_gain  = rec._gain;
	_delta = rec._delta;

	slat   = rec.slat;
	slon   = rec.slon;
	salt   = rec.salt;
	elat   = rec.elat;
	elon   = rec.elon;
	edep   = rec.edep;
	etim   = rec.etim;
	snorm  = rec.snorm;
	maxamp = rec.maxamp;
	rmin   = rec.rmin;
	cal    = rec.cal;
	com    = rec.com;
	ecom   = rec.ecom;
	log    = rec.log;
	//strcpy(com, rec.com);
	//strcpy(log, rec.log);
	//strcpy(ecom, rec.ecom);

	for (int i=0; i<NEXTRA; i++)
		setExtra(i, rec.extra(i));
}


AHRecord::AHRecord(const Record& rec)
	: Record(rec), _data(0)
{
	if ( samplingFrequency() > 0 )
		_delta = 1./samplingFrequency();
	else
		_delta = 0.;
}


AHRecord::~AHRecord() {}

AHRecord& AHRecord::operator=(const AHRecord& rec)
{
	std::cerr << "incomplete AHRecord::operator= called" << std::endl;

	if (this != &rec) {
		if (_data) {
			_data = NULL;
		}
		_data = rec._data ? rec._data->clone() : NULL;
// FIXME incomplete
	}

	return *this;
}

/*
double AHRecord::samplingFrequency() const
{
	return _fsamp;
//	return 1./_delta;
}
*/

void AHRecord::setSamplingFrequency(double freq)
{
	_fsamp = freq;
	_delta = 1./freq;
}

Array* AHRecord::data()
{
	return _data.get();
}

const Array* AHRecord::raw() const
{
	return NULL;
}

const Array* AHRecord::data() const
{
	return _data.get();
}

void AHRecord::setData(Array* data)
{
	switch (data->dataType()) {
	case Array::INT: // XXX will be serialized as float
	case Array::FLOAT:
	case Array::DOUBLE:
	// TODO: support for complex<float>, complex<double> needed
		break;
	default:
		throw Core::TypeException("illegal AH data type");
	}
	_data = data;
	_nsamp = _data->size();
	_datatype = _data->dataType();
}


void AHRecord::setData(int size, const void *data, Array::DataType dataType) {
	switch (dataType) {
	case Array::INT: // XXX will be serialized as float
	case Array::FLOAT:
	case Array::DOUBLE:
	// TODO: support for complex<float>, complex<double> needed
		break;
	default:
		throw Core::TypeException("illegal AH data type");
	}
	_data = ArrayFactory::Create(_datatype, dataType, size, data);
	_nsamp = _data->size();
}


AHRecord* AHRecord::copy() const
{
	return new AHRecord(*this);
}


void AHRecord::_reset()
{
	_data = 0;
}


void AHRecord::read(istream &in) {
	_reset();

	if (ah_get(in, *this) == AH_SUCCESS) {

		if (in.eof()) {
			_datatype = Array::DT_QUANTITY; // FIXME hackish!
			throw Core::EndOfStreamException();
		}

		return;
	}

	throw Core::StreamException("failed to read AH record");
}


void AHRecord::write(ostream &out) {
	if (ah_put(out, *this) != AH_SUCCESS)
		throw Core::StreamException();
}


void AHRecord::setGain(float value) {
	if (value==0.)
		throw Core::ValueException("Gain must not be 0");
	_gain = value;
}


float AHRecord::gain() const
{
	return _gain;
}


float AHRecord::extra(int i) const
{
	if (i>=0 && i<NEXTRAS)
		return _extra[i];

	return 0.; // maybe throw exception instead
}


void AHRecord::setExtra(int i, float value)
{
	if (i>=0 && i<NEXTRAS)
		_extra[i] = value;

	// else: maybe throw exception
}


// remove white space from the beginning
// and end of a character string
//
// returns the number of removed characters
static int 
strnorm(char *str)
{
	int n = 0, removed = 0, len = strlen(str);

	if (len == 0)
		return removed;

	while (isspace(str[n]))
		n++;

	if (n) {
		int i;

		len -= n;

		for (i = 0; i < len; i++)
			str[i] = str[i + n];
	}

	str[len] = '\0';

	removed = n + len;

	while (len && isspace(str[len - 1]))
		str[--len] = '\0';

	return removed - len;
}


static int
xdr_Calib(XDR *xdrs, AHRecord *rec)
{

	vector<float> cal(120);
	if (xdrs->x_op == XDR_ENCODE) {
		int n=rec->cal.size();

		for(int i=0;i<n; i++) {
			cal[i] = rec->cal[i];
		}
		for(int i=n;i<120; i++) {
			cal[i] = 0;
		}
	}
	for (int i=0; i<120; i++) {
		if ( ! xdr_float(xdrs, &(cal[i]))) {
		        return 0;
		}
	}
	if (xdrs->x_op == XDR_DECODE) {
		rec->cal = cal;
	}

	return 1;
}


static int
xdr_Time(XDR *xdrs, Core::Time *t)
{
	int yr, mo, day, hr, mn, sec, usec; float fsec;
	if (xdrs->x_op == XDR_ENCODE) {
		t->get(&yr, &mo, &day, &hr, &mn, &sec, &usec);
		fsec = sec + 0.000001*usec;
	}
	if ( ! xdr_int(xdrs,   &yr))   return 0;
	if ( ! xdr_int(xdrs,   &mo))   return 0;
	if ( ! xdr_int(xdrs,   &day))  return 0;
	if ( ! xdr_int(xdrs,   &hr))   return 0;
	if ( ! xdr_int(xdrs,   &mn))   return 0;
	if ( ! xdr_float(xdrs, &fsec)) return 0;
	if (xdrs->x_op == XDR_DECODE) {
		sec = (int) floor(fsec);
		usec = (int) floor((fsec-sec)*1000000+0.5);
		if (usec==1000000) {
			sec += 1;
			usec = 0;
		}
		t->set(yr, mo, day, hr, mn, sec, usec);
	}

	return 1;
}

static int
xdr_Header(XDR *xdrs, AHRecord *rec, int &ndata, int &dtype)
{   
	// read/write an AH header from/to an XDR stream

	// This is the lowest level dealing with XDR directly

	// return code:
	//   0 -> error
	//   1 -> success

	int op = xdrs->x_op;

//	else if (op != XDR_DECODE)
//		ah_error(1, "illegal XDR operation in _xdr_Header");

// station information ****************************************
	float lat=0, lon=0, alt=0, gain=1, norm=1;
	unsigned int l = CODESIZE;
	char *p, **pp, sta[CODESIZE+1]="\0", cha[CHANSIZE+1]="\0",
			typ[STYPESIZE+1]="\0";

	if (xdrs->x_op==XDR_ENCODE) {
		lat = rec->slat;
		lon = rec->slon;
		alt = rec->salt;
		gain = rec->gain();

		norm = rec->snorm;
		strncpy(sta, rec->stationCode().c_str(), CODESIZE);
		strncpy(cha, rec->channelCode().c_str(), CHANSIZE);
		strncpy(typ, rec->networkCode().c_str(), STYPESIZE);
		dtype = AH_DATATYPE_FLOAT; // XXX XXX XXX
		ndata = rec->sampleCount();
	}

	// station code
	l = CODESIZE;
	p = sta;
	pp = &p;
	if ( ! xdr_bytes(xdrs, pp, &l, CODESIZE))
		return 0;

	// channel code
	l = CHANSIZE;
	p = cha;
	pp = &p;
	if ( ! xdr_bytes(xdrs, pp, &l, CHANSIZE))
		return 0;

	// station type
	l = STYPESIZE;
	p = typ;
	pp = &p;
	if ( ! xdr_bytes(xdrs, pp, &l, STYPESIZE))
		return 0;
	if ( ! xdr_float(xdrs, &lat))
		return 0;
	if ( ! xdr_float(xdrs, &lon))
		return 0;
	if ( ! xdr_float(xdrs, &alt))
		return 0;
	if ( ! xdr_float(xdrs, &gain))
		return 0;
	if ( ! xdr_float(xdrs, &norm))
		return 0;
	if ( ! xdr_Calib(xdrs, rec))
		return 0;

	if (xdrs->x_op == XDR_DECODE) {
		// ensure properly aligned station and channel codes
		sta[CODESIZE] ='\0'; strnorm(sta); rec->setStationCode(sta);
		cha[CHANSIZE] ='\0'; strnorm(cha); rec->setChannelCode(cha);
		typ[STYPESIZE]='\0'; strnorm(typ); rec->setNetworkCode(typ);

		rec->slat = lat;
		rec->slon = lon;
		rec->salt = alt;
		rec->setGain(gain);
		rec->snorm = norm;
	}

// event information ****************************************
	float dep;
	char buf[COMSIZE], *com=buf, log[LOGSIZE];
	Core::Time tim;

	if (xdrs->x_op==XDR_ENCODE) {
		lat = rec->elat;
		lon = rec->elon;
		dep = rec->edep;
		tim = rec->etim;
		strncpy(com, rec->ecom.c_str(), COMSIZE);
	}

	if ( ! xdr_float(xdrs, &lat))
		return 0;
	if ( ! xdr_float(xdrs, &lon))
		return 0;
	if ( ! xdr_float(xdrs, &dep))
		return 0;
	if ( ! xdr_Time(xdrs, &tim))
		return 0;

	l = COMSIZE;
	p  = com;
	pp = &p;
	if ( ! xdr_bytes(xdrs, pp, &l, COMSIZE))
		return 0;

	if (xdrs->x_op==XDR_DECODE) {
		rec->elat = lat;
		rec->elon = lon;
		rec->edep = dep;
		rec->etim = tim;
		rec->ecom = com;
	}

// record information ****************************************
	float delta; // XXX AH saves sampling interval instead of frequency

	if ( ! xdr_int(xdrs, &dtype))
		return 0;

	Core::Time ttmp;
	if (xdrs->x_op == XDR_ENCODE) {
		ttmp = rec->startTime();
		delta = rec->_delta; // XXX
		strncpy(com, rec->com.c_str(), COMSIZE);
		strncpy(log, rec->log.c_str(), LOGSIZE);
		com[COMSIZE-1] = '\0'; // force and of C string
		log[LOGSIZE-1] = '\0'; // force and of C string
	}

	if ( ! xdr_int(xdrs, &ndata))
		return 0;
	if ( ! xdr_float(xdrs, &delta))
		return 0;
	if ( ! xdr_float(xdrs, &rec->maxamp))
		return 0;
	if ( ! xdr_Time(xdrs, &ttmp))
		return 0;
	if ( ! xdr_float(xdrs, &rec->rmin))
		return 0;

	l = COMSIZE;
	p = com;
	pp = &p;
	if ( ! xdr_bytes(xdrs, pp, &l, COMSIZE))
		return 0;

	l = LOGSIZE;
	p = log;
	pp = &p;
	if ( ! xdr_bytes(xdrs, pp, &l, LOGSIZE))
		return 0;

	if (xdrs->x_op == XDR_DECODE) {
		rec->setStartTime(ttmp);
		rec->_delta = delta;
		rec->setSamplingFrequency(1./delta);
		rec->com = com;
		rec->log = log;
	}

// extras ****************************************
	float x[NEXTRAS];
	if (op == XDR_ENCODE) {
		for (int i=0; i<NEXTRAS; i++)
			x[i] = rec->extra(i);
	}
	l = NEXTRAS;
	float *pf = x, **ppf = &pf;

	if ( ! xdr_array(xdrs, (caddr_t*) ppf,
		      &l, (unsigned int) NEXTRAS,
		      sizeof(float), (xdrproc_t) xdr_float))
		return 0;

	if (op == XDR_DECODE) {
		for (int i=0; i<NEXTRAS; i++)
			rec->setExtra(i, x[i]);
	}

	return 1;
}


static int
buflen(int ndata, int dtype)
{
	int n = 0;
	switch (dtype) {
	case AH_DATATYPE_FLOAT:
		n = ndata * sizeof(float);
		break;

	case AH_DATATYPE_COMPLEX:
		cerr << "data type complex not yet supported" << endl;
		n = ndata * sizeof(ahcomplex);
		break;

	default:
		cerr << "unsuported data type " << dtype << endl;
		exit(1);
	}

	return n;
}


static int
ah_get(istream &is, AHRecord &rec)
{
	// read header+data from an istream
    
	int ndata, dtype;

// read header *********************************
	char hbuf[HEADER_SIZE];
	XDR xdrs;

	// read a header from an istream
	is.read(hbuf, HEADER_SIZE);

	if (is.eof()) {

		// no bytes read -> proper EOF
		// otherwise -> unexpected EOF
		return is.gcount()==0 ? AH_SUCCESS : AH_ERROR;
	}

	if (is.fail()) {
		cerr << "failed to read header" << endl;
		return AH_ERROR;
	}

	xdrmem_create(&xdrs, hbuf, (unsigned int) HEADER_SIZE, XDR_DECODE);

	int status = AH_SUCCESS;

	// read a header from an XDR stream
	if ( ! xdr_Header(&xdrs, &rec, ndata, dtype))
		status = AH_ERROR;

	// check data type -> XXX do this inside xdr_Header
//      if ( ! is_valid_data_type(rec->type))
//              status = AH_ERROR;

	if (status == AH_SUCCESS && is.eof()) // XXX XXX XXX XXX XXX XXX XXX
		return AH_SUCCESS;      // proper EOF

// read data samples ***************************
	size_t bufsize = buflen(ndata, dtype);

	vector<char> dbuf(bufsize);
	char *buf = &dbuf[0];
	is.read(buf, bufsize);

	// While reading data we shall never reach EOF
	if (is.eof())
		return AH_ERROR;        // unexpected EOF
	if (is.fail())
		return AH_ERROR;        // <- obsolete ???

	xdrmem_create(&xdrs, buf, bufsize, XDR_DECODE);

	int n = ndata;

	switch (dtype) {
	case AH_DATATYPE_FLOAT:
		{
		// allocate new data
		FloatArray *arr = new FloatArray(n);
		rec.setData(arr);
		float *f = (float*)(arr->data());
# ifdef IEEE_INTEL
		float *f0 = (float*) xdrs.x_private;
		char  *b2 = (char*) f;
# endif

		// read data
# ifdef IEEE_SPARC
		assert(sizeof(float)==4);
		// copy without swapping
		memcpy(f, (void*) xdrs.x_private, n*sizeof(float));
# else
		while (n--)
		{
#   ifdef IEEE_INTEL
			char *b1 = (char*)(++f0);

			*(b2++) = *(--b1);
			*(b2++) = *(--b1);
			*(b2++) = *(--b1);
			*(b2++) = *(--b1);
#   else
			if ( ! xdr_float(&xdrs, f++))
				cerr << "error while reading data" << endl;
#   endif
		}
# endif
		}
		break;

	case AH_DATATYPE_COMPLEX:
/*
		{
		float re, im;

		// allocate new data
		Complex *c = new Complex [n];
		rec->xdata   = (void*) c;

		// read data
		for (int i=0; i<n; i++, c++)
		{
			re = im = 0.0;

			if ( ! xdr_float(&xdrs, &re) ||
			     ! xdr_float(&xdrs, &im))
				ah_error(1, "error while reading data");

			*c = Complex(re, im);
		}
		}
*/
		break;

	default:
		cerr << "ah_error_illegal_data_type XY" << endl;
		break;
	}

	xdr_destroy(&xdrs);

	return status;
}

static int
ah_put(ostream &os, AHRecord const &rec0)
{
	// write header+data to an ostream
	AHRecord rec(rec0); // XXX for const'ness
    
// write header *********************************
	int status=AH_SUCCESS, ndata, dtype;
	{
	char hbuf[HEADER_SIZE];
	XDR xdrs;

	xdrmem_create(&xdrs, hbuf, HEADER_SIZE, XDR_ENCODE);

	if ( ! xdr_Header(&xdrs, &rec, ndata, dtype)) // sets ndata, dtype
		status = AH_ERROR;

	if (status != AH_SUCCESS)
		return AH_ERROR;

	os.write(hbuf, HEADER_SIZE);

	if (os.fail())
		return AH_ERROR;

	xdr_destroy(&xdrs);
	}
// write data samples ***************************
	XDR xdrs;
	int bufsize = buflen(ndata, dtype);

	// write data to an ostream
	vector<char> dbuf(bufsize+100);
	char *buf = &dbuf[0];

	xdrmem_create(&xdrs, buf, bufsize, XDR_ENCODE);

	int n = ndata;

	switch (dtype) {
	case AH_DATATYPE_FLOAT:
		{
# ifdef IEEE_INTEL
		char  *b2 = (char*) xdrs.x_private;
# endif
		Array *arr = rec.data();
		if (arr==0)
			cerr << "THIS MUST NEVER HAPPEN: arr==0\n";
		float *f  = (float*)(arr->data());

# ifdef IEEE_SPARC
		assert(sizeof(float)==4);
		memcpy((void*) xdrs.x_private, (void*)f, n*sizeof(float));
# else
		while (n--) {
#   ifdef IEEE_INTEL
			char *b1 = (char*)(++f);

			*(b2++) = *(--b1);
			*(b2++) = *(--b1);
			*(b2++) = *(--b1);
			*(b2++) = *(--b1);
#   else
			if ( ! xdr_float(&xdrs, f++)) {
				cerr << "error while reading data" << endl;
				status = AH_ERROR;
				break;
			}
#   endif
		}
# endif
		}
		break;

	case AH_DATATYPE_COMPLEX:
/*
		{
		float re, im;
		Complex *c = (Complex*) rec.xdata;

		for (i=0; i<n; i++) {
			re = real(c[i]);
			im = imag(c[i]);

			if ( ! xdr_float(&xdrs, &re) ||
			     ! xdr_float(&xdrs, &im)) {
			    ah_error(ahERR_IO_WR, "error while writing data");
			    status = AH_ERROR;
			    break;
			}
		}
		}
*/
		break;

	default:
		cerr << "ah_error_illegal_data_type YY" << endl;
		break;
	}

	xdr_destroy(&xdrs);

	os.write(buf, bufsize);
	if (os.fail())
		status = AH_ERROR;

	return status;
}


AHRecord* read_one(istream &is)
{
	AHRecord *rec = new AHRecord();

	try {
		rec->read(is);
	}
	catch(...) {
		delete rec;
		rec = 0;
	}

	return rec;
}

bool write_one(AHRecord *rec, ostream &os)
{
	try {
		ah_put(os, *rec);
	}
	catch(...) {
		return false;
	}

	return true;
}


AHOutput::AHOutput(const std::string &filename)
 : _ofstream(0)
{
	_filename = filename;
	
	if (_filename != "") {
		_ofstream = new std::ofstream(filename.c_str());
	}
}

AHOutput::~AHOutput()
{
	if (_ofstream) {
		_ofstream->close();
		delete _ofstream;
	}
}

bool AHOutput::put(AHRecord *rec) {
	if ( !rec ) return false;

	try {
		ah_put(_ofstream ? *_ofstream : std::cout, *((AHRecord*)rec));
	}
	catch(...) {
		return false;
	}
	
	return true;
}


}

}

