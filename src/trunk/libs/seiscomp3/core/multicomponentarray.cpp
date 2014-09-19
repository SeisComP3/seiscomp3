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


#define SEISCOMP_COMPONENT MULTICOMPONENTARRAY
#include <seiscomp3/logging/log.h>
#include <seiscomp3/core/strings.h>
#include <seiscomp3/core/multicomponentarray.h>
#include <seiscomp3/core/typedarray.h>


using namespace std;
using namespace Seiscomp;
using namespace Seiscomp::Core;


IMPLEMENT_SC_CLASS(MultiComponent, "MultiComponent");
IMPLEMENT_SC_CLASS(MultiComponentArray, "MultiComponentArray");


Array* FillZeroesFunctor::operator()(const Array *pred, const Array *succ, double dt, double freq) const {
    if (dt <= _gapLength) {
        int number = (int)floor(dt * freq);
	switch (pred->dataType()) {
		case Array::CHAR:
			return new CharArray(number);
		case Array::INT:
			return new IntArray(number);
		case Array::FLOAT:
			return new FloatArray(number);
		case Array::DOUBLE:
			return new DoubleArray(number);
		default:
			break;
	}
    }

    return NULL;
}


template<typename T>
Array* linearInterpolate(const Array *pred, const Array *succ, int number) {
    if (number > 0) {
        TypedArray<T> *result = new TypedArray<T>(number);
        TypedArray<T> const *array = TypedArray<T>::ConstCast(pred);
        T pval = array->get(array->size() - 1);
        array = TypedArray<T>::ConstCast(succ);
        T sval = array->get(0);

        double step = (double)abs(pval - sval) / number;
        int factor = 1;
        if (pval - sval > 0)
            factor = -1;

        for (int i = 0; i < number; ++i)
            result->set(i, (int)(i * step * factor + pval));

        return result;
    }

    return NULL;
}

Array* LinearInterpolateFunctor::operator()(const Array *pred, const Array *succ, double dt, double freq) const {
    if (dt <= _gapLength) {        
        int number = (int)floor(dt * freq);
        switch (pred->dataType()) {
            case Array::CHAR:
                return linearInterpolate<char>(pred, succ, number);
            case Array::INT:
                return linearInterpolate<int>(pred, succ, number);
            case Array::FLOAT:
                return linearInterpolate<float>(pred, succ, number);
            case Array::DOUBLE:
                return linearInterpolate<double>(pred, succ, number);
            default:
                break;
        }
    }

    return NULL;
}

///////////////////////////////////////////////////////////////////////////////////////

MultiComponent::MultiComponent(const std::string &net, const std::string &sta,
                               const std::string &loc, const string &cha, float freq):
    _network(net),
    _station(sta),
    _location(loc),
    _channel(cha),
    _frequency(freq),
    _data(0)
{}

MultiComponent::~MultiComponent() {}

const string &MultiComponent::networkCode() const {
    return _network;
}

const string &MultiComponent::stationCode() const {
    return _station;
}

const string &MultiComponent::locationCode() const {
    return _location;
}

const string &MultiComponent::channelCode() const {
    return _channel;
}

string MultiComponent::streamID() const {
    return _network + "." + _station + "." + _location + "." + _channel;
}

const Core::Time& MultiComponent::startTime() const {
    return _start;
}

Core::Time MultiComponent::endTime() const {
    return (_start + Core::Time(_data->size() / _frequency));
}

int MultiComponent::sampleNumber() const {
    return _data->size();
}

float MultiComponent::samplingFrequency() const {
    return _frequency;
}

Array* MultiComponent::data() const {
    return _data.get();
}

bool MultiComponent::feed(const Array *array, const Core::Time &time, const GapFillFunctor &gapfiller, 
                          const Core::Time &commstart, const Core::Time &commend) {
    int dsize = array->size();
    Core::Time stime = time;
    Core::Time etime = stime + (Core::Time((double)dsize / _frequency));
    ArrayCPtr data = array;

    if ((commend != Core::Time() && commend < stime) || commstart > etime) {
        SEISCOMP_INFO("Record is not in the given time window!");
        return false;
    }

    if (stime < commstart) {
        int samples = (int)ceil((double)(commstart - stime) * _frequency);
        data = data->slice(samples, dsize);
        dsize = data->size();
        stime = Core::Time(etime - Core::Time(dsize / _frequency));
    }

    if (commend != Core::Time() && etime > commend) {
        int samples = (int)floor((double)(commend - stime) * _frequency);        
        data = data->slice(0, samples);
        dsize = data->size();
        etime = stime + (Core::Time((double)dsize / _frequency));
    }

    // first data
    if (!_data) {
        _data = data->copy(data->dataType());
        _start = stime;
        return true;
    }

    int cursize = _data->size();
    Core::Time end = _start + (Core::Time((double)cursize / _frequency));
    double seconds = (double)(stime - end);

    // continiously following data
    if (fabs(seconds) < 1 / _frequency) {
        _data->append(data.get());
        return true;
    }

    // gap
    if (end < stime) {
        ArrayPtr gaparray = gapfiller(_data.get(), data.get(), seconds, _frequency);
        if (gaparray != 0) {
            _data->append(gaparray.get());
            _data->append(data.get());
            return true;
        } else {            
            SEISCOMP_WARNING("Gap (%.2f sec) not handled for %s!", seconds, streamID().c_str());
            throw GapNotHandledException();
            // Information about a gap that has not been handled must be communicated
            // to the user. Otherwise discarding gappy data is not possible at all. Gaps cannot
            // be handled in all cases. E.g. MT traces are discarded completely if a gap
            // occurs. Since this class is able to fill gaps later on using out-of-order-data
            // a gap tracking vector has to be maintained. This is overkill and therefore I
            // recommend to remove out-of-order-data support. Use a RecordSequence for out-of-
            // order-data and create a continuous record that way.
        }
    }

    // overlap (newer data overwrites the existing one)
    if (end > stime) {
        if (_start >= etime) {
            SEISCOMP_INFO("Out of order data not handled for %s!", streamID().c_str());
            return false;
        }

        ArrayPtr newdata = 0;
        if (_start >= stime) {
            int samples = (int)floor((double)(etime - _start) * _frequency);
            newdata = data->slice(dsize - samples, dsize);
            if (cursize > samples) {
                ArrayPtr tmp = _data->slice(samples, cursize);
                newdata->append(tmp.get());
            }
            _data = newdata;
            return true;
        }

        if (end <= etime) {
            int samples = (int)floor((double)(end - stime) *  _frequency);
            _data = _data->slice(0, cursize - samples);
            _data->append(data.get());
            return true;
        }
        
        if (end > etime) {
            int samples = (int)floor((double)(stime - _start) * _frequency);
            newdata = _data->slice(0, samples);
            newdata->append(data.get());
            ArrayPtr tmp = _data->slice(samples + dsize, cursize);
            newdata->append(tmp.get());
            _data = newdata;
            return true;
        }
    }

    return false;
}

bool MultiComponent::prune(const Core::Time *beg, const Core::Time *end) {
    if ((!beg && !end) || (beg && end && *beg >= *end))
        return false;

    Core::Time etime;
    if (_frequency > 0.0)
        etime = _start + Core::Time(_data->size() / _frequency);
    else {
        SEISCOMP_INFO("Pruning impossible because of a wrong sampling frequency (%f)!", _frequency);
        return false;
    }

    bool result = false;

    if ((beg && _start == *beg) || (end && etime == *end)) {
        result = true;
    }

    if ((beg && _start < *beg && *beg < etime) || 
        (end && _start < *end && *end < etime)) {
        if (!_data) {
            SEISCOMP_INFO("Pruning impossible because of missing data!");
            return false;
        }

        if (beg && _start < *beg) {
            double seconds = (double)(*beg - _start);
            int samples = (int)ceil(seconds * _frequency); // cut at or after the given start

            _data = _data->slice(samples, _data->size());
            _start = Core::Time(etime - Core::Time(_data->size() / _frequency));
            result = true;
        }

        if (end && *end < etime) {
            double seconds = (double)(*end - _start);
            int samples = (int)floor(seconds * _frequency); // cut at or before the given end

            _data = _data->slice(0, samples);
            result = true;
        }
    }

    return result;
}


///////////////////////////////////////////////////////////////////////////////////////////

MultiComponentArray::MultiComponentArray(const Core::Time &start, const Core::Time &end)
    : _start(start),
      _end(end) {}

MultiComponentArray::~MultiComponentArray() {}

const MultiComponentArray::MultiArray& MultiComponentArray::components() const {
    return _multicomp;
}

void MultiComponentArray::setStartTime(const Core::Time &start) {
    _start = start;
}

void MultiComponentArray::setEndTime(const Core::Time &end) {
    _end = end;
}

MultiComponentArray* MultiComponentArray::get(std::string &pattern) {
    return get(pattern.c_str());
}

MultiComponentArray* MultiComponentArray::get(const char *pattern) {
    MultiComponentArray *multi = 0;

    for (MultiArrayIterator it = _multicomp.begin(); it != _multicomp.end(); ++it) {
        if (wildcmp(pattern, (*it)->streamID().c_str())) {
            if (!multi)
                multi = new MultiComponentArray(_start, _end);
            multi->set((*it)->networkCode(), (*it)->stationCode(),
                       (*it)->locationCode(), (*it)->channelCode(),
                       (*it)->samplingFrequency(), (*it)->startTime(), (*it)->data());
        }
    }

    return multi;
}

void MultiComponentArray::set(const string &net, const string &sta, const string &loc, const string &cha, 
                              float freq, const Time &start, const Array *data) {
    bool found = false;
    for (MultiArrayIterator it = _multicomp.begin(); it != _multicomp.end(); ++it) {
        if ((*it)->streamID() == net + "." + sta + "." + loc + "." + cha) {
            found = true;
            (*it)->_frequency = freq;
            (*it)->_start = start;
            (*it)->_data = const_cast<Array *>(data);
            break;
        }
    }

    if (!found) {
        MultiComponent *comp = new MultiComponent(net, sta, loc, cha, freq);
        comp->_start = start;
        comp->_data = const_cast<Array *>(data);
        _multicomp.push_back(comp);
    }
}


bool MultiComponentArray::feed(Record *rec, const GapFillFunctor &gapfiller) {
    float freq = rec->samplingFrequency();
    if (freq <= 0.0) {
        SEISCOMP_INFO("MultiComponentArray::feed: ignore incoming record because of wrong sampling frequency (%f).", freq);
        return false;
    }

    bool result = false;
    bool found = false;
    for (MultiArrayIterator it = _multicomp.begin(); it != _multicomp.end() && !found; ++it) {
        if ((*it)->streamID() == rec->streamID()) {
            found = true;
            if (freq == (*it)->samplingFrequency()) {
                if ((*it)->data() && (*it)->data()->dataType() != rec->data()->dataType())
                    rec->setDataType((*it)->data()->dataType());
                result = (*it)->feed(rec->data(), rec->startTime(), gapfiller, _start, _end);
            } else
                SEISCOMP_WARNING("MultiComponentArray::feed: ignore incoming record because of different sampling frequencies (record (%f) vs. component (%f)).", freq, (*it)->samplingFrequency());
            break;
        }
    }

    if (!found) {
        MultiComponent *comp = new MultiComponent(rec->networkCode(), rec->stationCode(),
                                                  rec->locationCode(), rec->channelCode(), freq);
        result = comp->feed(rec->data(), rec->startTime(), gapfiller, _start, _end);
        _multicomp.push_back(comp);
    }

    return result;
}


bool MultiComponentArray::prune(const Core::Time *beg, const Core::Time *end) {
    bool result = false;

    for (MultiArrayIterator it = _multicomp.begin(); it != _multicomp.end(); ++it) 
        result = (*it)->prune(beg, end) || result;

    return result;
}

bool MultiComponentArray::prune(Core::Time beg, Core::Time end) {
    bool result = false;
    
    for (MultiArrayIterator it = _multicomp.begin(); it != _multicomp.end(); ++it)
        result = (*it)->prune(&beg, &end) || result;
    
    return result;
}

bool MultiComponentArray::prune() {
    Core::Time start;
    Core::Time end;
    
    for (MultiArrayIterator it = _multicomp.begin(); it != _multicomp.end(); ++it)  {
        Core::Time itstart = (*it)->startTime();
        Core::Time itend = itstart + Core::Time((*it)->sampleNumber() / (*it)->samplingFrequency());
	
        if (start < itstart)
            start = itstart;
        if (end == Core::Time() || end > itend)
            end = itend;
    }

    return prune(start, end);
}

bool MultiComponentArray::reject(const string &streamID) {
    for (MultiArrayIterator it = _multicomp.begin(); it != _multicomp.end(); ++it)  {
        if ((*it)->streamID() == streamID) {
            _multicomp.erase(it);
            return true;
        }
    }

    return false;
}
