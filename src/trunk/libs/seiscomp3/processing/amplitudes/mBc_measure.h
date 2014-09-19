#include<vector>

class Measurement
{
  public:
	Measurement(int nsamp);
	virtual ~Measurement() {}

	// offset of the data samples
	// must be measured prior to the measurement
	void set_offset(double x) { offset = x; }

	// Feed n broadband ground velocity values v.
	// feed() calls update() automatically!
	virtual void feed(int n, const double *v) = 0;

	double progress() const { return nsamp>0 ? double(processed)/nsamp : 0; }

  protected:
	// an offset to be subtracted before the actual measurement
	double offset;

	// number of samples anticipated to contribute to measurement
	unsigned int nsamp;

	// number of samples already processed
	unsigned int processed;
};


class Measurement_mBc : public Measurement
{
  public:
	Measurement_mBc(int nsamp, double q=0.6);

	void feed(int n, const double *v);

  public:
	// Accessors to subevents. mBc specific and meant for debugging, e.g.
	// for plotting the individual subevents contributing to the sum.
	//
	// "Subevents" are individual extrema that contribute to the sum.
	int    subevent_count() const;
	int    subevent_index(int i) const;
	double subevent_value(int i) const;

  public:
	double vcum; // cumulative velocity
	double vmax; // zero-to-peak maximum velocity
	int    icum; // index at which vcum was observed ( = index of last subevent)
	int    imax; // index at which vmax was observed

  private:
	// Cutoff threshold, normally fixed to 0.6, don't change!
	// See also Bormann & Saul (2009)
	double q;

	std::vector<int>    subevent_indices;
	std::vector<double> subevent_values;

	int previous, ipeak;
	double vpeak;
};
