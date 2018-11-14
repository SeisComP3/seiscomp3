/***************************************************************************
 *   Copyright (C) by GFZ Potsdam, gempa GmbH                              *
 *                                                                         *
 *   You can redistribute and/or modify this program under the             *
 *   terms of the SeisComP Public License.                                 *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   SeisComP Public License for more details.                             *
 ***************************************************************************/

%module(package="seiscomp3") Core

%exceptionclass GeneralException;

%{
#include <exception>
#include "seiscomp3/core/baseobject.h"
#include "seiscomp3/core/exceptions.h"
#include "seiscomp3/core/array.h"
#include "seiscomp3/core/bitset.h"
#include "seiscomp3/core/enumeration.h"
#include "seiscomp3/core/typedarray.h"
#include "seiscomp3/core/record.h"
#include "seiscomp3/core/greensfunction.h"
#include "seiscomp3/core/genericrecord.h"
#include "seiscomp3/core/interruptible.h"
#include "seiscomp3/core/datamessage.h"
#include "seiscomp3/core/status.h"
#include "seiscomp3/core/version.h"
#ifdef HAVE_NUMPY
#include <numpy/ndarrayobject.h>
#endif
%}

%init
%{
#ifdef HAVE_NUMPY
import_array();
#endif
%}

%newobject *::Cast;
%newobject *::copy;
%newobject *::clone;
%newobject Seiscomp::Core::DataMessage::get;
%ignore Seiscomp::Core::Smartpointer<Seiscomp::Core::BaseObject>::Impl;
%ignore Seiscomp::Core::GreensFunction::setData(int,Seiscomp::Array*);
%ignore Seiscomp::Core::GreensFunction::data(int) const;

%include exception.i
%include std_except.i
%include "base.i"

enum(Seiscomp::Core::GreensFunctionComponent);

%include "seiscomp3/core/status.h"
%include "seiscomp3/core/array.h"
%include "seiscomp3/core/bitset.h"
%include "seiscomp3/core/typedarray.h"
%include "seiscomp3/core/record.h"
%include "seiscomp3/core/greensfunction.h"
%include "seiscomp3/core/genericrecord.h"
%include "seiscomp3/core/message.h"
%import "seiscomp3/core/genericmessage.h"

%template(DataMessageBase) Seiscomp::Core::GenericMessage<Seiscomp::Core::BaseObject>;

%include "seiscomp3/core/datamessage.h"

%extend Seiscomp::Array {
#if defined(SWIGPYTHON)
	PyObject* numpy() {
%#ifdef HAVE_NUMPY
		npy_intp n = (npy_intp) self->size();
		int type = PyArray_CHAR;
		switch ( self->dataType() ) {
			case Seiscomp::Array::CHAR:
				type = PyArray_CHAR;
				break;
			case Seiscomp::Array::INT:
				type = PyArray_INT;
				break;
			case Seiscomp::Array::FLOAT:
				type = PyArray_FLOAT;
				break;
			case Seiscomp::Array::DOUBLE:
				type = PyArray_DOUBLE;
				break;
			default:
				SWIG_exception(SWIG_TypeError, "unsupported array type");
				goto fail;
		}
		return PyArray_SimpleNewFromData(1, &n, type, (char*)self->data());
%#else
		SWIG_exception(SWIG_SystemError, "missing support for NumPy");
%#endif
		fail:
			return NULL;
	}

	PyObject* setNumpy(PyObject *obj) {
%#ifdef HAVE_NUMPY
		PyArrayObject *arr;
		switch ( self->dataType() ) {
			case Seiscomp::Array::CHAR:
				arr = (PyArrayObject*) PyArray_ContiguousFromObject(obj, PyArray_CHAR, 1, 1);
				if ( arr == NULL )
					return PyErr_Format(PyExc_TypeError,
						"Unable to convert object to 1-D char array");
				static_cast<Seiscomp::CharArray*>(self)->setData(arr->dimensions[0],(char *)(arr->data));
				break;
			case Seiscomp::Array::INT:
				arr = (PyArrayObject*) PyArray_ContiguousFromObject(obj, PyArray_INT, 1, 1);
				if ( arr == NULL )
					return PyErr_Format(PyExc_TypeError,
						"Unable to convert object to 1-D int array");
				static_cast<Seiscomp::IntArray*>(self)->setData(arr->dimensions[0],(int *)(arr->data));
				break;
			case Seiscomp::Array::FLOAT:
				arr = (PyArrayObject*) PyArray_ContiguousFromObject(obj, PyArray_FLOAT, 1, 1);
				if ( arr == NULL )
					return PyErr_Format(PyExc_TypeError,
						"Unable to convert object to 1-D float array");
				static_cast<Seiscomp::FloatArray*>(self)->setData(arr->dimensions[0],(float *)(arr->data));
				break;
			case Seiscomp::Array::DOUBLE:
				arr = (PyArrayObject*) PyArray_ContiguousFromObject(obj, PyArray_DOUBLE, 1, 1);
				if ( arr == NULL )
					return PyErr_Format(PyExc_TypeError,
						"Unable to convert object to 1-D double array");
				static_cast<Seiscomp::DoubleArray*>(self)->setData(arr->dimensions[0],(double *)(arr->data));
				break;
			default:
				SWIG_exception(SWIG_TypeError, "unsupported array type");
				goto fail;
		}

		Py_XDECREF(arr);
		Py_RETURN_NONE;
%#else
		SWIG_exception(SWIG_SystemError, "missing support for NumPy");
%#endif
		fail:
			Py_RETURN_NONE;
	}

	%pythoncode %{
		def __str__(self):
			return self.str()
		def numeric(self):
			import sys
			sys.stderr.write("Use of Array.numeric() is deprecated - use numpy() instead\n")
			return self.numpy()
	%}
#endif
};

%template(CharArrayT) Seiscomp::TypedArray<char>;
%template(IntArrayT) Seiscomp::TypedArray<int>;
%template(FloatArrayT) Seiscomp::TypedArray<float>;
%template(DoubleArrayT) Seiscomp::TypedArray<double>;
%template(ComplexFloatArray) Seiscomp::TypedArray< std::complex<float> >;
%template(ComplexDoubleArray) Seiscomp::TypedArray< std::complex<double> >;
%template(DateTimeArray) Seiscomp::TypedArray<Seiscomp::Core::Time>;
%template(StringArray) Seiscomp::TypedArray<std::string>;

%template(CharArray) Seiscomp::NumericArray<char>;
%template(IntArray) Seiscomp::NumericArray<int>;
%template(FloatArray) Seiscomp::NumericArray<float>;
%template(DoubleArray) Seiscomp::NumericArray<double>;


%extend Seiscomp::Core::Time {
        %pythoncode %{
                def __str__(self):
                        return self.toString("%Y-%m-%d %H:%M:%S.%f000000")[:23]
        %}
};

%extend Seiscomp::Core::TimeSpan {
        %pythoncode %{
                def __float__(self):
                        return self.length()
        %}
};


%extend Seiscomp::Core::Message {
	%pythoncode %{
		def __iter__(self):
			return self.iter()
	%}
};

%extend Seiscomp::Core::MessageIterator {
	void step() {
		++(*self);
	}

	%pythoncode %{
		def next(self):
			o = self.get()
			if not o:
				raise StopIteration
			
			self.step()
			return o
	%}
};

