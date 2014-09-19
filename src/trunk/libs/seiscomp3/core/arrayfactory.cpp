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


#include <algorithm>
#include <seiscomp3/core/arrayfactory.h>
#include <seiscomp3/core/typedarray.h>

using namespace Seiscomp;

template<typename TGT, typename SRC>
struct convert {
	TGT operator()(SRC value) {
		return static_cast<TGT>(value);
	}
};

template<typename SRC>
struct convert< std::complex<float>, SRC > {
	std::complex<float> operator()(SRC value) {
		return std::complex<float>(value, 0);
	}
};

template<typename TGT>
struct convert<TGT, std::complex<float> > {
	TGT operator()(std::complex<float> value) {
		return (TGT)abs(value);
	}
};

template<>
struct convert< std::complex<float>, std::complex<float> > {
	std::complex<float> operator()(std::complex<float> value) {
		return value;
	}
};


template<typename SRC>
struct convert< std::complex<double>, SRC > {
	std::complex<double> operator()(SRC value) {
		return std::complex<double>(value, 0);
	}
};

template<typename TGT>
struct convert<TGT, std::complex<double> > {
	TGT operator()(std::complex<double> value) {
		return (TGT)abs(value);
	}
};

template<>
struct convert< std::complex<double>, std::complex<double> > {
	std::complex<double> operator()(std::complex<double> value) {
		return value;
	}
};


template<>
struct convert< std::complex<float>, std::complex<double> > {
	std::complex<float> operator()(std::complex<double> value) {
		return std::complex<float>(value.real(), value.imag());
	}
};


template<>
struct convert< std::complex<double>, std::complex<float> > {
	std::complex<double> operator()(std::complex<float> value) {
		return std::complex<double>(value.real(), value.imag());
	}
};


template <typename CONTAINER, typename T>
void convertArray(CONTAINER &c, int size, const T *data) {
	std::transform(data,data+size,std::back_inserter(c),convert<typename CONTAINER::value_type,T>());
}

Array* ArrayFactory::Create(Array::DataType toCreate, Array::DataType caller, int size, const void *data) {
	Array *ar = NULL;

	switch(toCreate) {
	case Array::CHAR: 
		switch (caller) {
		case Array::CHAR:
			ar = new CharArray();
			convertArray(dynamic_cast<CharArray *>(ar)->_data,size,static_cast<const char *>(data));
			break;
		case Array::INT:
			ar = new CharArray();
			convertArray(dynamic_cast<CharArray *>(ar)->_data,size,static_cast<const int *>(data));
			break;
		case Array::FLOAT:
			ar = new CharArray();
			convertArray(dynamic_cast<CharArray *>(ar)->_data,size,static_cast<const float *>(data));
			break;
		case Array::DOUBLE:
			ar = new CharArray();
			convertArray(dynamic_cast<CharArray *>(ar)->_data,size,static_cast<const double *>(data));
			break;
		case Array::COMPLEX_FLOAT:
			ar = new CharArray();
			convertArray(dynamic_cast<CharArray *>(ar)->_data,size,static_cast<const std::complex<float> *>(data));
			break;
		case Array::COMPLEX_DOUBLE:
			ar = new CharArray();
			convertArray(dynamic_cast<CharArray *>(ar)->_data,size,static_cast<const std::complex<double> *>(data));
			break;
		default:
			break;
		}
		break;
	case Array::INT:
		switch (caller) {
		case Array::CHAR:
			ar = new IntArray();
			convertArray(dynamic_cast<IntArray *>(ar)->_data,size,static_cast<const char *>(data));
			break;
		case Array::INT:
			ar = new IntArray();
			convertArray(dynamic_cast<IntArray *>(ar)->_data,size,static_cast<const int *>(data));
			break;
		case Array::FLOAT:
			ar = new IntArray();
			convertArray(dynamic_cast<IntArray *>(ar)->_data,size,static_cast<const float *>(data));
			break;
		case Array::DOUBLE:
			ar = new IntArray();
			convertArray(dynamic_cast<IntArray *>(ar)->_data,size,static_cast<const double *>(data));
			break;
		case Array::COMPLEX_FLOAT:
			ar = new IntArray();
			convertArray(dynamic_cast<IntArray *>(ar)->_data,size,static_cast<const std::complex<float> *>(data));
			break;
		case Array::COMPLEX_DOUBLE:
			ar = new IntArray();
			convertArray(dynamic_cast<IntArray *>(ar)->_data,size,static_cast<const std::complex<double> *>(data));
			break;
		default:
			break;
		}
		break;
	case Array::FLOAT:
		switch (caller) {
		case Array::CHAR:
			ar = new FloatArray();
			convertArray(dynamic_cast<FloatArray *>(ar)->_data,size,static_cast<const char *>(data));
			break;
		case Array::INT:
			ar = new FloatArray();
			convertArray(dynamic_cast<FloatArray *>(ar)->_data,size,static_cast<const int *>(data));
			break;
		case Array::FLOAT:
			ar = new FloatArray();
			convertArray(dynamic_cast<FloatArray *>(ar)->_data,size,static_cast<const float *>(data));
			break;
		case Array::DOUBLE:
			ar = new FloatArray();
			convertArray(dynamic_cast<FloatArray *>(ar)->_data,size,static_cast<const double *>(data));
			break;
		case Array::COMPLEX_FLOAT:
			ar = new FloatArray();
			convertArray(dynamic_cast<FloatArray *>(ar)->_data,size,static_cast<const std::complex<float> *>(data));
			break;
		case Array::COMPLEX_DOUBLE:
			ar = new FloatArray();
			convertArray(dynamic_cast<FloatArray *>(ar)->_data,size,static_cast<const std::complex<double> *>(data));
			break;
		default:
			break;
		}
		break;
	case Array::DOUBLE:
		switch (caller) {
		case Array::CHAR:
			ar = new DoubleArray();
			convertArray(dynamic_cast<DoubleArray *>(ar)->_data,size,static_cast<const char *>(data));
			break;
		case Array::INT:
			ar = new DoubleArray();
			convertArray(dynamic_cast<DoubleArray *>(ar)->_data,size,static_cast<const int *>(data));
			break;
		case Array::FLOAT:
			ar = new DoubleArray();
			convertArray(dynamic_cast<DoubleArray *>(ar)->_data,size,static_cast<const float *>(data));
			break;
		case Array::DOUBLE:
			ar = new DoubleArray();
			convertArray(dynamic_cast<DoubleArray *>(ar)->_data,size,static_cast<const double *>(data));
			break;
		case Array::COMPLEX_FLOAT:
			ar = new DoubleArray();
			convertArray(dynamic_cast<DoubleArray *>(ar)->_data,size,static_cast<const std::complex<float> *>(data));
			break;
		case Array::COMPLEX_DOUBLE:
			ar = new DoubleArray();
			convertArray(dynamic_cast<DoubleArray *>(ar)->_data,size,static_cast<const std::complex<double> *>(data));
			break;
		default:
			break;
		}
		break;
	case Array::COMPLEX_FLOAT:
		switch (caller) {
		case Array::CHAR:
			ar = new ComplexFloatArray();
			convertArray(dynamic_cast<ComplexFloatArray *>(ar)->_data,size,static_cast<const char *>(data));
			break;
		case Array::INT:
			ar = new ComplexFloatArray();
			convertArray(dynamic_cast<ComplexFloatArray *>(ar)->_data,size,static_cast<const int *>(data));
			break;
		case Array::FLOAT:
			ar = new ComplexFloatArray();
			convertArray(dynamic_cast<ComplexFloatArray *>(ar)->_data,size,static_cast<const float *>(data));
			break;
		case Array::DOUBLE:
			ar = new ComplexFloatArray();
			convertArray(dynamic_cast<ComplexFloatArray *>(ar)->_data,size,static_cast<const double *>(data));
			break;
		case Array::COMPLEX_FLOAT:
			ar = new ComplexFloatArray();
			convertArray(dynamic_cast<ComplexFloatArray *>(ar)->_data,size,static_cast<const std::complex<float> *>(data));
			break;
		case Array::COMPLEX_DOUBLE:
			ar = new ComplexFloatArray();
			convertArray(dynamic_cast<ComplexFloatArray *>(ar)->_data,size,static_cast<const std::complex<double> *>(data));
			break;
		default:
			break;
		}
		break;
	case Array::COMPLEX_DOUBLE:
		switch (caller) {
		case Array::CHAR:
			ar = new ComplexDoubleArray();
			convertArray(dynamic_cast<ComplexDoubleArray *>(ar)->_data,size,static_cast<const char *>(data));
			break;
		case Array::INT:
			ar = new ComplexDoubleArray();
			convertArray(dynamic_cast<ComplexDoubleArray *>(ar)->_data,size,static_cast<const int *>(data));
			break;
		case Array::FLOAT:
			ar = new ComplexDoubleArray();
			convertArray(dynamic_cast<ComplexDoubleArray *>(ar)->_data,size,static_cast<const float *>(data));
			break;
		case Array::DOUBLE:
			ar = new ComplexDoubleArray();
			convertArray(dynamic_cast<ComplexDoubleArray *>(ar)->_data,size,static_cast<const double *>(data));
			break;
		case Array::COMPLEX_FLOAT:
			ar = new ComplexDoubleArray();
			convertArray(dynamic_cast<ComplexDoubleArray *>(ar)->_data,size,static_cast<const std::complex<float> *>(data));
			break;
		case Array::COMPLEX_DOUBLE:
			ar = new ComplexDoubleArray();
			convertArray(dynamic_cast<ComplexDoubleArray *>(ar)->_data,size,static_cast<const std::complex<double> *>(data));
			break;
		default:
			break;
		}
		break;
	default:
		ar = 0;
	}

	return ar;
}

Array* ArrayFactory::Create(Array::DataType toCreate, const Array *source) {
	return Create(toCreate,source->dataType(),source->size(),const_cast<void *>(source->data()));
}

