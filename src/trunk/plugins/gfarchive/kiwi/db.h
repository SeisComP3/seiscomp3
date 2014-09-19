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



#include "chunk.h"

#include <vector>


struct DB {
	DB();
	~DB();

	bool open(const std::string &path);
	void close();

	Seiscomp::Core::GreensFunction *readGF(double dist, double depth, double timeSpan);

	std::vector<Chunk> chunks;
	float              dt, dx, dz;
	int                nx, nxc, nz, ng;
};
