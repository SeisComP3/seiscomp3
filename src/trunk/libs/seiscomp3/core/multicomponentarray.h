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


#ifndef __SEISCOMP_MULTICOMPONENTARRAY_H__
#define __SEISCOMP_MULTICOMPONENTARRAY_H__


#include <vector>
#include <seiscomp3/core/baseobject.h>
#include <seiscomp3/core/record.h>


namespace Seiscomp {


class GapFillFunctor {
public:
    //! initializing constructor
    GapFillFunctor(double gaplen): _gapLength(gaplen) {}

    //! destructor
    virtual ~GapFillFunctor() {}

    // Returns the gap-fill-data if the gap length was shorter than specified; 0 otherwise.
    virtual Array* operator()(const Array *pred, const Array *succ, double dt, double freq) const = 0;

protected:
    double _gapLength;
};

class FillZeroesFunctor : public GapFillFunctor {
public:
    FillZeroesFunctor(double gaplen): GapFillFunctor(gaplen) {}

    virtual ~FillZeroesFunctor() {}

    virtual Array* operator()(const Array *pred, const Array *succ, double dt, double freq) const;
};

class LinearInterpolateFunctor: public GapFillFunctor {
public:
    LinearInterpolateFunctor(double gaplen): GapFillFunctor(gaplen) {}

    virtual ~LinearInterpolateFunctor() {}

    virtual Array* operator()(const Array *pred, const Array *succ, double dt, double freq) const;
};


DEFINE_SMARTPOINTER(MultiComponent);

class SC_SYSTEM_CORE_API GapNotHandledException : public Core::GeneralException {
 public:
        GapNotHandledException() : Core::GeneralException("gap not handled") {}
        GapNotHandledException(std::string what) : Core::GeneralException(what) {}
};

class SC_SYSTEM_CORE_API MultiComponent: public Core::BaseObject {
    DECLARE_SC_CLASS(MultiComponent);

    friend class MultiComponentArray;

public:
    //! default constructor
    MultiComponent() {}

    //! initializing constructor
    MultiComponent(const std::string &net, const std::string &sta,
                   const std::string &loc, const std::string &cha, float freq);

    //! destructor
    virtual ~MultiComponent();

    //! Returns the network code.
    const std::string &networkCode() const;

    //! Returns the station code.
    const std::string &stationCode() const;

    //! Returns the location code.
    const std::string &locationCode() const;

    //! Returns the component's channel.
    const std::string &channelCode() const;

    //! Return the stream ID.
    std::string streamID() const;

    //! Returns the start time of data.
    const Core::Time& startTime() const;

    //! Returns the end time of data.
    Core::Time endTime() const;

    //! Returns the number of data samples.
    int sampleNumber() const;

    //! Returns the sampling frequency.
    float samplingFrequency() const;

    //! Returns the waveform data.
    Array* data() const;

    //! Inserts the given data using the specified gap filling algorithm if necessary.
    bool feed(const Array *data, const Core::Time &start, const GapFillFunctor &gapfiller,
              const Core::Time &commonstart, const Core::Time &commonend);

    //! Cuts-off the data at the given time.
    bool prune(const Core::Time *beg, const Core::Time *end);

private:
    std::string _network;
    std::string _station;
    std::string _location;
    std::string _channel;
    Core::Time _start;
    float _frequency;
    ArrayPtr _data;
};


DEFINE_SMARTPOINTER(MultiComponentArray);

class SC_SYSTEM_CORE_API MultiComponentArray: public Core::BaseObject {
    DECLARE_SC_CLASS(MultiComponentArray);

public:
    typedef std::vector<MultiComponentPtr> MultiArray;
    typedef std::vector<MultiComponentPtr>::iterator MultiArrayIterator;
    typedef std::vector<MultiComponentPtr>::const_iterator MultiArrayConstIterator;

    //! default constructor
    MultiComponentArray() {}

    //! initializing constructor
    MultiComponentArray(const Core::Time &start, const Core::Time &end);

    //! destructor
    virtual ~MultiComponentArray();

    //! Returns the vector of MultiComponents.
    const MultiArray& components() const;

    //! Sets the start / end time for the component's data.
    void setStartTime(const Core::Time &start);
    void setEndTime(const Core::Time &end);

    //! Inserts the given record to the array using the specified gap filling algorithm
    //! if necessary. Returns true if the record was actually added.
    bool feed(Record *rec, const GapFillFunctor &gapfiller);

    //! Returns those components fitting the given stream ID search pattern (<net|*>.<sta|*>.<loc|*>.<cha|*>).
    //! The caller has to be care about the deletion of the returned MultiComponentArray instance
    //! (can be 0 if nothing could be found).
    MultiComponentArray* get(std::string &pattern);
    MultiComponentArray* get(const char *pattern);

    //! Cuts-off the array data at the given time.
    //! One paramter can be 0; if both parameters are zero no pruning will be done
    bool prune(const Core::Time *beg, const Core::Time *end);
    bool prune(Core::Time beg, Core::Time end);

    //! Cuts-off the array data. The cut will be effected at the biggest common start time and
    //! at the least common end time.
    bool prune();

    //! Rejects the component specified with the given stream ID from the array.
    bool reject(const std::string &streamID);

private:
    //! Sets the given component data.
    void set(const std::string &net, const std::string &sta, const std::string &loc, const std::string &cha,
             float freq, const Core::Time &start, const Array *data);

    MultiArray _multicomp;
    Core::Time _start;
    Core::Time _end;
};
}

#endif

