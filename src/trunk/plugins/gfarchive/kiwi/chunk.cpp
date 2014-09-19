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



#include <seiscomp3/core/typedarray.h>
#include <iostream>

#include "chunk.h"


using namespace std;
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Chunk::Chunk() {
	id = -1;
	nxc = 0;
	nz = 0;
	ng = 0;
	references = NULL;

	file_id = 0;
	dataset_index = 0;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Chunk::~Chunk() {
	close();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Chunk::init(int id, const string &fn, int nxc, int nz, int ng) {
	this->id = id;
	this->filename = fn;
	this->nxc = nxc;
	this->nz = nz;
	this->ng = ng;

	nipx = 1;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Chunk::open() {
	if ( file_id != 0 ) return true;

	H5E_BEGIN_TRY {
		file_id = H5Fopen(filename.c_str(), H5F_ACC_RDONLY, H5P_DEFAULT);
	} H5E_END_TRY;

	if ( file_id == -1 )
		return false;

	H5E_BEGIN_TRY {
		dataset_index = H5Dopen(file_id, "index");
	} H5E_END_TRY;

	if ( dataset_index == -1 )
		return false;

	// Read whatever here
	size_t records = nxc*nz*ng;
	herr_t err;
	hsize_t dims[3] = {nxc,nz,ng};

	hid_t mem_id = H5Screate_simple(3,dims,NULL);

	references = new hobj_ref_t[records];
	err = H5Dread(dataset_index, H5T_STD_REF_OBJ, mem_id, H5S_ALL,
	              H5P_DEFAULT, references);

	H5Sclose(mem_id);

	if ( err ) {
		close();
		file_id = -1;
		return false;
	}

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Chunk::close() {
	if ( references != NULL ) {
		delete[] references;
		references = NULL;
	}

	if ( dataset_index > 0 ) {
		H5Dclose(dataset_index);
		dataset_index = 0;
	}

	if ( file_id > 0 ) {
		H5Fclose(file_id);
		file_id = 0;
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Seiscomp::Core::GreensFunction *Chunk::readGF(size_t ix, size_t iz, size_t sampleCount) {
	// open dataset
	if ( !open() ) return NULL;

	// check bounds
	if ( ix >= nxc || iz >= nz )
		return NULL;

	hid_t dataset, attr_ofs, attr_pofs, space;
	vector< vector<int> > ofs(ng);
	vector< vector<int> > pofs(ng);
	vector<hssize_t> data_len(ng);
	vector<Seiscomp::FloatArrayPtr> arrays(ng);

	int min_ofs;
	size_t max_len;

	// First stage: read all strip information and data
	// lengths
	for ( size_t igf = 0; igf < ng; ++igf ) {
		// open dataset referencing the greens function
		dataset = H5Rdereference(dataset_index, H5R_OBJECT, &references[ix*nz*ng + iz*ng + igf]);
		if ( dataset == -1 ) {
			H5Dclose(dataset);
			return NULL;
		}

		// read the data length
		space = H5Dget_space(dataset);
		if ( space == -1 ) {
			H5Dclose(dataset);
			return NULL;
		}

		data_len[igf] = H5Sget_simple_extent_npoints(space);
		H5Sclose(space);

		// open ofs and pofs attributes
		attr_ofs = H5Aopen_idx(dataset, 1);
		if ( attr_ofs == -1 ) {
			H5Dclose(dataset);
			return NULL;
		}

		attr_pofs = H5Aopen_idx(dataset, 0);
		if ( attr_pofs == -1 ) {
			H5Aclose(attr_ofs);
			H5Dclose(dataset);
			return NULL;
		}

		hssize_t len_ofs, len_pofs;

		space = H5Aget_space(attr_ofs);
		if ( space == -1 ) {
			H5Aclose(attr_ofs);
			H5Aclose(attr_pofs);
			H5Dclose(dataset);
			return NULL;
		}

		len_ofs = H5Sget_simple_extent_npoints(space);
		H5Sclose(space);

		space = H5Aget_space(attr_pofs);
		if ( space == -1 ) {
			H5Aclose(attr_ofs);
			H5Aclose(attr_pofs);
			H5Dclose(dataset);
			return NULL;
		}

		len_pofs = H5Sget_simple_extent_npoints(space);
		H5Sclose(space);

		// the ofs and pofs vector must be of same size and not empty
		if ( len_ofs <= 0 || len_pofs <= 0 || len_ofs != len_pofs ) {
			H5Aclose(attr_ofs);
			H5Aclose(attr_pofs);
			H5Dclose(dataset);
			return NULL;
		}

		ofs[igf].resize(len_ofs);
		pofs[igf].resize(len_pofs);

		if ( H5Aread(attr_ofs, H5T_NATIVE_INT, &ofs[igf][0]) ) {
			H5Aclose(attr_ofs);
			H5Aclose(attr_pofs);
			return NULL;
		}

		if ( H5Aread(attr_pofs, H5T_NATIVE_INT, &pofs[igf][0]) ) {
			H5Aclose(attr_ofs);
			H5Aclose(attr_pofs);
			return NULL;
		}

		H5Aclose(attr_ofs);
		H5Aclose(attr_pofs);
		H5Dclose(dataset);

		// offset indicies are starting from 1 (fortran convention)
		size_t unpacked_length = data_len[igf] - pofs[igf].back() + 1 + ofs[igf].back() - ofs[igf].front();
		if ( igf == 0 ) {
			min_ofs = ofs[igf].front();
			max_len = unpacked_length;
		}
		else {
			min_ofs = min(min_ofs, ofs[igf].front());
			max_len = max(max_len, unpacked_length);
		}
	}

	max_len = max(max_len, sampleCount-min_ofs);

	//cout << "min_ofs = " << min_ofs << endl;
	//cout << "max_len = " << max_len << endl;

	for ( size_t igf = 0; igf < ng; ++igf ) {
		// open dataset referencing the greens function
		dataset = H5Rdereference(dataset_index, H5R_OBJECT, &references[ix*nz*ng + iz*ng + igf]);
		if ( dataset == -1 ) {
			H5Dclose(dataset);
			return NULL;
		}

		hsize_t len = data_len[igf];
		size_t unpacked_len = len - pofs[igf].back() + 1 + ofs[igf].back() - ofs[igf].front();

		arrays[igf] = new Seiscomp::FloatArray(max_len);
		float *data = arrays[igf]->typedData();
		//std::cout << "len = " << len << std::endl;

		hid_t mem_id = H5Screate_simple(1, &len, NULL);

		if ( H5Dread(dataset, H5T_NATIVE_FLOAT, mem_id, H5S_ALL, H5P_DEFAULT, data) ) {
			H5Sclose(mem_id);
			H5Dclose(dataset);
			return NULL;
		}

		H5Sclose(mem_id);
		H5Dclose(dataset);

		for ( int i = 0; i < arrays[igf]->size(); ++i )
			(*arrays[igf])[i] *= 10E5;

		/*
		cout << "[len] = " << len << endl;
		cout << "[pofs] ";
		for ( size_t i = 0; i < pofs[igf].size(); ++i ) {
			if ( i ) cout << ", ";
			cout << pofs[igf][i];
		}
		cout << endl;

		cout << "[ofs] ";
		for ( size_t i = 0; i < ofs[igf].size(); ++i ) {
			if ( i ) cout << ", ";
			cout << ofs[igf][i];
		}
		cout << endl;
		*/

		// Unpack trace
		size_t lastPackedSize = len;
		size_t start_ofs = ofs[igf].front() - min_ofs;

		// Fill leading zeros
		for ( size_t i = 0; i < start_ofs; ++i )
			data[i] = 0;

		for ( size_t i = 0; i < ofs[igf].size(); ++i ) {
			size_t idx = (size_t)(ofs[igf][ofs[igf].size()-1-i] - ofs[igf].front()) + start_ofs;
			size_t len = lastPackedSize - pofs[igf][pofs[igf].size()-1-i] + 1;

			size_t start_a = idx, end_a = idx+len-1;
			size_t start_b = lastPackedSize-len, end_b = lastPackedSize-1;

			lastPackedSize -= len;

			if ( start_a == start_b && end_a == end_b ) continue;

			/*
			cout << "[" << start_a << ":" << end_a << "] = "
			     << "[" << start_b << ":" << end_b << "]" << endl;
			*/

			// Move strips to the real position
			for ( size_t j = 0; j < len; ++j )
				data[end_a-1] = data[end_b-1];
		}

		if ( ofs[igf].size() > 1 ) {
			// Fill gaps with zero
			for ( size_t i = 1; i < ofs[igf].size(); ++i ) {
				size_t start_gap = ofs[igf][i-1] + pofs[igf][i] - min_ofs;
				size_t end_gap = ofs[igf][i] - min_ofs;
				for ( size_t j = start_gap; j < end_gap; ++j )
					data[j] = 0;
			}
		}

		if ( sampleCount < (size_t)arrays[igf]->size() )
			arrays[igf]->resize(sampleCount);

		// Fill remaining samples (duplicate the last real one)
		for ( int i = unpacked_len; i < arrays[igf]->size(); ++i )
			data[i] = data[unpacked_len-1];
	}

	Seiscomp::Core::GreensFunction *gf = new Seiscomp::Core::GreensFunction;
	gf->setTimeOffset((double)min_ofs);

	// Flip and copy gf6
	Seiscomp::FloatArrayPtr gf6flipped = new Seiscomp::FloatArray(*arrays[5]);
	for ( int i = 0; i < gf6flipped->size(); ++i )
		(*gf6flipped)[i] *= -1;

	// Flip gf7
	for ( int i = 0; i < arrays[6]->size(); ++i )
		(*arrays[6])[i] *= -1;

	// Flip and copy gf1
	Seiscomp::FloatArrayPtr gf1flipped = new Seiscomp::FloatArray(*arrays[0]);
	for ( int i = 0; i < gf1flipped->size(); ++i )
		(*gf1flipped)[i] *= -1;

	// Flip gf2
	for ( int i = 0; i < arrays[1]->size(); ++i )
		(*arrays[1])[i] *= -1;

	// Flip gf5
	for ( int i = 0; i < arrays[4]->size(); ++i )
		(*arrays[4])[i] *= -1;

	gf->setData(Seiscomp::Core::ZSS, arrays[5].get());
	gf->setData(Seiscomp::Core::ZDS, arrays[6].get());
	gf->setData(Seiscomp::Core::ZDD, gf6flipped.get());

	gf->setData(Seiscomp::Core::RSS, arrays[0].get());
	gf->setData(Seiscomp::Core::RDS, arrays[1].get());
	gf->setData(Seiscomp::Core::RDD, gf1flipped.get());

	gf->setData(Seiscomp::Core::TSS, arrays[3].get());
	gf->setData(Seiscomp::Core::TDS, arrays[4].get());

	// Invert Z from down to up
	for ( int i = 0; i < gf->data(Seiscomp::Core::ZSS)->size(); ++i )
		(*static_cast<Seiscomp::FloatArray*>(gf->data(Seiscomp::Core::ZSS)))[i] *= -1;

	for ( int i = 0; i < gf->data(Seiscomp::Core::ZDS)->size(); ++i )
		(*static_cast<Seiscomp::FloatArray*>(gf->data(Seiscomp::Core::ZDS)))[i] *= -1;

	for ( int i = 0; i < gf->data(Seiscomp::Core::ZDD)->size(); ++i )
		(*static_cast<Seiscomp::FloatArray*>(gf->data(Seiscomp::Core::ZDD)))[i] *= -1;


	return gf;
}
