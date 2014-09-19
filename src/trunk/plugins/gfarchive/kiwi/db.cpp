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



#include "db.h"

#include <seiscomp3/core/strings.h>
#include <seiscomp3/math/geo.h>
#include <iostream>


/***************************************************************************
 * General HDF5 convenience functions
 **************************************************************************/
template <typename T>
hid_t dispatchMemTypeID();

#define MAP_HDF5_TYPE(type, id) \
	template <> \
	hid_t dispatchMemTypeID<type>() { \
		return id; \
	}

MAP_HDF5_TYPE(int, H5T_NATIVE_INT)
MAP_HDF5_TYPE(float, H5T_NATIVE_FLOAT)
MAP_HDF5_TYPE(double, H5T_NATIVE_DOUBLE)


template <typename T>
T hdf5_read_scalar(hid_t file_id, const char *dataset)
throw(Seiscomp::Core::GeneralException)
{
	hid_t dataset_id;

	H5E_BEGIN_TRY {
		dataset_id = H5Dopen(file_id, dataset);
	} H5E_END_TRY;

	if ( dataset_id == -1 )
		throw Seiscomp::Core::GeneralException((std::string("dataset ") +
		                                       dataset + ": not found").c_str());

	T value;
	herr_t error;
	hsize_t dim = 1;

	hid_t mem_id = H5Screate_simple(1,&dim,NULL);

	//H5E_BEGIN_TRY {
		error = H5Dread(dataset_id, dispatchMemTypeID<T>(),
		                mem_id, H5S_ALL, H5P_DEFAULT, &value);
	//} H5E_END_TRY;

	H5Sclose(mem_id);

	if ( error ) {
		H5Dclose(dataset_id);
		throw Seiscomp::Core::GeneralException((std::string("dataset ") +
		                                       dataset + ": error " + Seiscomp::Core::toString(error)).c_str());
	}

	H5Dclose(dataset_id);

	return value;
}


template <typename T>
void hdf5_read_scalar(T &value, hid_t file_id, const char *dataset)
throw(Seiscomp::Core::GeneralException)
{
	value = hdf5_read_scalar<T>(file_id, dataset);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
DB::DB() {
	close();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
DB::~DB() {
	close();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool DB::open(const std::string &path) {
	hid_t file_id;

	H5E_BEGIN_TRY {
		/* Open an existing file. */
		file_id = H5Fopen((path + "/db.index").c_str(), H5F_ACC_RDONLY, H5P_DEFAULT);
	} H5E_END_TRY;

	if ( file_id == -1 ) return false;

	int nchunks;

	// Read header information
	try {
		hdf5_read_scalar(dt, file_id, "dt");
		hdf5_read_scalar(dx, file_id, "dx");
		hdf5_read_scalar(dz, file_id, "dz");

		hdf5_read_scalar(nchunks, file_id, "nchunks");
		hdf5_read_scalar(nx,  file_id, "nx");
		hdf5_read_scalar(nxc, file_id, "nxc");
		hdf5_read_scalar(nz,  file_id, "nz");
		hdf5_read_scalar(ng,  file_id, "ng");
	}
	catch ( Seiscomp::Core::GeneralException &exc ) {
		H5Fclose(file_id);
		//cerr << "error: " << exc.what() << endl;
		return false;
	}

	H5Fclose(file_id);

	chunks.resize(nchunks);

	for ( size_t i = 0; i < chunks.size(); ++i ) {
		int nxcthis = nxc;
		if ( i == chunks.size()-1 )
			nxcthis = nx - i*nxc;

		chunks[i].init(i+1, path + "/db." + Seiscomp::Core::toString(i+1) + ".chunk",
		               nxcthis, nz, ng);

		if ( !chunks[i].open() ) {
			close();
			return false;
		}
	}

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void DB::close() {
	chunks = std::vector<Chunk>();
	dt = dx = dz = 0;
	nx = nxc = nz = ng = 0;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Seiscomp::Core::GreensFunction *DB::readGF(double dist, double depth, double timeSpan) {
	int idist = (int)(dist*1000 / dx);
	int idep  = (int)(depth * 1000/ dz);

	if ( idist >= nx || idep >= nz ) {
		// Out of bounds
		return NULL;
	}

	int ichunk = idist / nxc;
	int ichunkdist = idist % nxc;

	if ( ichunk >= (int)chunks.size() ) {
		// Chunk out of bounds
		return NULL;
	}

	Seiscomp::Core::GreensFunction *gf =
		chunks[ichunk].readGF(ichunkdist, idep, timeSpan / dt);

	if ( gf ) {
		gf->setSamplingFrequency(1.0 / dt);
		gf->setDepth(float(idep)*dz*0.001);
		gf->setDistance(float(idist)*dx*0.001);
		gf->setTimeOffset(gf->timeOffset() * dt);
	}

	return gf;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
