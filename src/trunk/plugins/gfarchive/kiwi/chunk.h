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


#include <seiscomp3/core/greensfunction.h>
#include <hdf5.h>


struct Chunk {
	Chunk();
	~Chunk();

	void init(int id, const std::string &fn, int nxc, int nz, int ng);
	bool open();
	void close();
	Seiscomp::Core::GreensFunction *readGF(size_t ix, size_t iz, size_t sampleCount);

	size_t id;   // this chunks number
	size_t nxc;  // number of receiver-distances in this chunk
	size_t nz;   // number of source depths
	size_t ng;   // number of greens functions (components) (=8)

	int    nipx; // total traces / real traces
	             // (increment between 2 non-interpolated traces)
	int    nipz; // total traces / real traces
	             // (increment between 2 non-interpolated traces)

	std::string filename;
	hid_t       file_id;
	hid_t       dataset_index;

	hobj_ref_t *references;
};
