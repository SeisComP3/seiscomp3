//- ****************************************************************************
//- 
//- Copyright 2009 Sandia Corporation. Under the terms of Contract
//- DE-AC04-94AL85000 with Sandia Corporation, the U.S. Government
//- retains certain rights in this software.
//- 
//- BSD Open Source License.
//- All rights reserved.
//- 
//- Redistribution and use in source and binary forms, with or without
//- modification, are permitted provided that the following conditions are met:
//- 
//-    * Redistributions of source code must retain the above copyright notice,
//-      this list of conditions and the following disclaimer.
//-    * Redistributions in binary form must reproduce the above copyright
//-      notice, this list of conditions and the following disclaimer in the
//-      documentation and/or other materials provided with the distribution.
//-    * Neither the name of Sandia National Laboratories nor the names of its
//-      contributors may be used to endorse or promote products derived from
//-      this software without specific prior written permission.
//- 
//- THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
//- AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
//- IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
//- ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
//- LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
//- CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
//- SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
//- INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
//- CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
//- ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
//- POSSIBILITY OF SUCH DAMAGE.
//-
//- ****************************************************************************

#ifndef ARRAYREUSE_OBJECT_H
#define ARRAYREUSE_OBJECT_H

// **** _SYSTEM INCLUDES_ ******************************************************

#include <iostream>
#include <fstream>
#include <vector>

// use standard library objects
using namespace std;

// **** _LOCAL INCLUDES_ *******************************************************

#include "CPPUtils.h"
#include "GeoTessException.h"

// **** _BEGIN GEOTESS NAMESPACE_ **********************************************

namespace geotess {

// **** _FORWARD REFERENCES_ ***************************************************

// **** _CLASS DEFINITION_ *****************************************************

/**
 * \brief An array reuse object for cases where arrays of some fixed type and size
 * are required by the application over and over.
 *
 * An array reuse object for cases where arrays of some fixed type and size
 * are required by the application over and over but it is desirable to avoid
 * the constant allocation, and subsequent deletion, of many small arrays.
 */
template<typename T>
class GEOTESS_EXP_IMP ArrayReuse {
private:

	/**
	 * The length of each array provided by the function getArray(). All
	 * provided arrays are fixed to this length.
	 */
	int arrayLength;

	/**
	 * The number of arrays (each of arrayLength) added to the store when
	 * more arrays are required.
	 */
	int arraysAddedPerAlloc;

	/**
	 * The initial number of arrays (each of arrayLength) added to the store
	 * when initialize is called.
	 */
	int initArraysAddedPerAlloc;

	/**
	 * The total number of arrays (each of arrayLength) allocated thus far.
	 */
	int allocArrays;

	/**
	 * The permanent storage container of all allocated arrays. Deleted at
	 * destruction.
	 */
	vector<T*> store;

	/**
	 * The temporary container of all arrays (each of arrayLength) provided
	 * to requesting clients by function getArray(). Arrays are returned for
	 * reuse by calling function reuseArray(T* a).
	 */
	vector<T*> refStore;

	/**
	 *
	 * Add count more T arrays, each of length arrayLength, to store and
	 * refStore. This function is called at construction and by function
	 * getArray() if more entries are needed.
	 */
	void add(int count) {
		store.push_back(new T[count * arrayLength]);
		for (int i = 0; i < count; ++i)
			refStore.push_back(&store.back()[i * arrayLength]);

		allocArrays += count;
	}

	/**
	 * Destroys all allocated arrays.
	 */
	void destroy() {
		if ((int)refStore.size() != allocArrays)
		{
			ostringstream os;
			os << endl << "ERROR in ArrayReuse::destroy()" << endl
					<< allocArrays << " arrays have been allocated but "
					<< allocArrays - refStore.size()
					<< " of them have not been returned." << endl;
			throw GeoTessException(os, __FILE__, __LINE__, 23001);
		}
		refStore.clear();
		for (int i = 0; i < (int) store.size(); ++i)
			delete[] store[i];
	}

public:

	/**
	 * Default constructor. Made private to prevent it from being used.
	 */
	ArrayReuse() :
			arrayLength(1), arraysAddedPerAlloc(1), allocArrays(0) {
	}
	;

	/**
	 * Standard constructor where alngth is the size of the arrays
	 * to be provided to requesting clients, iacnt is the initial
	 * number of arrays to create for used by clients, and acnt is
	 * the subsequent array count allocation used when getArray()
	 * is called but no more arrays are available.
	 *
	 * @param	alngth The size of the arrays to be provided to
	 * 							 requesting clients.
	 * @param acnt   The subsequent array count allocation used
	 * 							 when getArray() is called but no more arrays
	 * 							 are available
	 */
	ArrayReuse(int alngth, int acnt) {
		initialize(alngth, acnt, acnt, 8, acnt + acnt);
	}

	/**
	 * Standard constructor where alngth is the size of the arrays
	 * to be provided to requesting clients, iacnt is the initial
	 * number of arrays to create for used by clients, and acnt is
	 * the subsequent array count allocation used when getArray()
	 * is called but no more arrays are available.
	 *
	 * @param	alngth The size of the arrays to be provided to
	 * 							 requesting clients.
	 * @param iacnt  The initial number of arrays to create for
	 * 							 use by clients.
	 * @param acnt   The subsequent array count allocation used
	 * 							 when getArray() is called but no more arrays
	 * 							 are available
	 */
	ArrayReuse(int alngth, int iacnt, int acnt) {
		initialize(alngth, iacnt, acnt, 8, iacnt + acnt);
	}

	/**
	 * Standard constructor where alngth is the size of the arrays
	 * to be provided to requesting clients, iacnt is the initial
	 * number of arrays to create for used by clients, and acnt is
	 * the subsequent array count allocation used when getArray()
	 * is called but no more arrays are available, rsrvstr is the
	 * capacity of the permanent store (store), and rsrvrefstr is
	 * the capacity of the reuseable reference store (refStore).
	 *
	 * @param	alngth The size of the arrays to be provided to
	 * 							 requesting clients.
	 * @param iacnt  The initial number of arrays to create for
	 * 							 use by clients.
	 * @param acnt   The subsequent array count allocation used
	 * 							 when getArray() is called but no more arrays
	 * 							 are available
	 * @param rsrvstr    The capacity of the permanent store
	 * 									 (store).
	 * @param	rsrvrefstr The capacity of the reuseable reference
	 * 									 store (refStore).
	 */
	ArrayReuse(int alngth, int iacnt, int acnt, int rsrvstr, int rsrvrefstr) {
		initialize(alngth, iacnt, acnt, rsrvstr, rsrvrefstr);
	}

	/**
	 * Destructor.
	 */
	virtual ~ArrayReuse() {
		destroy();
	}

	/**
	 * Returns a new array of size arrayLength for use by requesting clients.
	 * This function will create more arrays if necessary.
	 * @return a new array of size arrayLength
	 */
	T* getArray() {
		if (refStore.size() == 0)
			add(arraysAddedPerAlloc);

		T* a = refStore.back();
		refStore.pop_back();
		return a;
	}

	/**
	 * Returns an array that was being used by a client back into the reuse
	 * pool.
	 * @param a the array that is to be returned to the reuse pool
	 */
	void reuseArray(T* a) {
		refStore.push_back(a);
	}

	/**
	 * Resets refStore to all allocated references in store. NOTE: if a client
	 * is still using a reference after this reset() is called the possibility
	 * exists that it will be given out as a new request from function
	 * getArray(). Only call this function when you know all entries are no
	 * longer in use or needed by the requesting clients.
	 */
	void reset() {
		refStore.clear();
		for (int i = 0; i < (int) store.size(); ++i)
			for (int j = 0;
					j
							< ((i == 0) ?
									initArraysAddedPerAlloc :
									arraysAddedPerAlloc); ++j) {
				refStore.push_back(&store[i][j * arrayLength]);
			}
	}

	/**
	 * Calls reset if the size of refStore is not equal to the total number of
	 * allocated arrays (allocArrays).
	 */
	void resetIfRequired() {
		if ((int) refStore.size() != allocArrays)
			reset();
	}

	/**
	 * Initializes sizes where alngth is the size of the arrays
	 * to be provided to requesting clients, iacnt is the initial
	 * number of arrays to create for used by clients, and acnt is
	 * the subsequent array count allocation used when getArray()
	 * is called but no more arrays are available, rsrvstr is the
	 * capacity of the permanent store (store), and rsrvrefstr is
	 * the capacity of the reuseable reference store (refStore).
	 *
	 * @param	alngth The size of the arrays to be provided to
	 * 							 requesting clients.
	 * @param acnt   The subsequent array count allocation used
	 * 							 when getArray() is called but no more arrays
	 * 							 are available
	 */
	void initialize(int alngth, int acnt) {
		initialize(alngth, acnt, acnt, 8, acnt + acnt);
	}

	/**
	 * Initializes sizes where alngth is the size of the arrays
	 * to be provided to requesting clients, iacnt is the initial
	 * number of arrays to create for used by clients, and acnt is
	 * the subsequent array count allocation used when getArray()
	 * is called but no more arrays are available, rsrvstr is the
	 * capacity of the permanent store (store), and rsrvrefstr is
	 * the capacity of the reuseable reference store (refStore).
	 *
	 * @param	alngth The size of the arrays to be provided to
	 * 							 requesting clients.
	 * @param iacnt  The initial number of arrays to create for
	 * 							 use by clients.
	 * @param acnt   The subsequent array count allocation used
	 * 							 when getArray() is called but no more arrays
	 * 							 are available
	 */
	void initialize(int alngth, int iacnt, int acnt) {
		initialize(alngth, iacnt, acnt, 8, iacnt + acnt);
	}

	/**
	 * Initializes sizes where alngth is the size of the arrays
	 * to be provided to requesting clients, iacnt is the initial
	 * number of arrays to create for used by clients, and acnt is
	 * the subsequent array count allocation used when getArray()
	 * is called but no more arrays are available, rsrvstr is the
	 * capacity of the permanent store (store), and rsrvrefstr is
	 * the capacity of the reuseable reference store (refStore).
	 *
	 * @param	alngth The size of the arrays to be provided to
	 * 							 requesting clients.
	 * @param iacnt  The initial number of arrays to create for
	 * 							 use by clients.
	 * @param acnt   The subsequent array count allocation used
	 * 							 when getArray() is called but no more arrays
	 * 							 are available
	 * @param rsrvstr    The capacity of the permanent store
	 * 									 (store).
	 * @param	rsrvrefstr The capacity of the reuseable reference
	 * 									 store (refStore).
	 */
	void initialize(int alngth, int iacnt, int acnt, int rsrvstr,
			int rsrvrefstr) {
		// first delete any previous allocation

		destroy();

		// reinitialize

		arrayLength = alngth;
		arraysAddedPerAlloc = acnt;
		initArraysAddedPerAlloc = iacnt;
		store.reserve(rsrvstr);
		refStore.reserve(rsrvrefstr);
		add(iacnt);
	}

	/**
	 * Returns the total number of allocated arrays (each of arrayLength).
	 * @return  the total number of allocated arrays (each of arrayLength).
	 */
	int getAllocatedArrays() const {
		return allocArrays;
	}

	/**
	 * Returns the number of arrays currently in use by clients.
	 * @return  the number of arrays currently in use by clients.
	 */
	int getUsedArrayCount() const {
		return allocArrays - (int) refStore.size();
	}

	/**
	 * Returns the current number of unused allocated arrays.
	 * @return  the current number of unused allocated arrays.
	 */
	int getUnusedArrayCount() const {
		return (int) refStore.size();
	}

	/**
	 * Returns the array length setting.
	 * @return the array length settting
	 */
	int getArrayLength() const {
		return arrayLength;
	}

};
// end class ArrayReuse

}// end namespace geotess

#endif  // ARRAYREUSE_OBJECT_H
