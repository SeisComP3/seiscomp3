/***************************************************************************
 *   Copyright (C) by gempa GmbH                                          *
 *                                                                         *
 *   You can redistribute and/or modify this program under the             *
 *   terms of the SeisComP Public License.                                 *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   SeisComP Public License for more details.                             *
 ***************************************************************************/


#define SEISCOMP_TEST_MODULE test_tabvalues


#define SEISCOMP_COMPONENT test_tabvalues
#include <seiscomp3/logging/log.h>
#include <seiscomp3/config/config.h>
#include <seiscomp3/utils/tabvalues.h>
#include <seiscomp3/unittest/unittests.h>


using namespace std;
using namespace Seiscomp::Logging;
using namespace Seiscomp::Util;
namespace bu = boost::unit_test;
//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>




//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
BOOST_AUTO_TEST_CASE(load_tab) {
	enableConsoleLogging(getAll());
	TabValues table;
	BOOST_REQUIRE(table.read("./data/iasp91.P"));
	BOOST_CHECK(table.x.size() == 181);
	BOOST_CHECK(table.y.size() == 15);

	for ( size_t i = 0; i < table.x.size(); ++i )
		BOOST_CHECK(table.x[i] == i);

	BOOST_CHECK(table.y[0] == 0.00);
	BOOST_CHECK(table.y[1] == 5.00);
	BOOST_CHECK(table.y[2] == 15.00);
	BOOST_CHECK(table.y[3] == 30.00);
	BOOST_CHECK(table.y[4] == 40.00);
	BOOST_CHECK(table.y[5] == 50.00);
	BOOST_CHECK(table.y[6] == 75.00);
	BOOST_CHECK(table.y[7] == 100.00);
	BOOST_CHECK(table.y[8] == 150.00);
	BOOST_CHECK(table.y[9] == 200.00);
	BOOST_CHECK(table.y[10] == 300.00);
	BOOST_CHECK(table.y[11] == 400.00);
	BOOST_CHECK(table.y[12] == 500.00);
	BOOST_CHECK(table.y[13] == 600.00);
	BOOST_CHECK(table.y[14] == 800.00);
}
//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>




//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
BOOST_AUTO_TEST_CASE(interpolate_tab) {
	enableConsoleLogging(getAll());
	TabValues table;
	double value, x1stDeriv, x2ndDeriv, y1stDeriv, y2ndDeriv;
	int error;

	BOOST_REQUIRE(table.read("./data/iasp91.P"));

	for ( size_t i = 0; i < table.x.size(); ++i ) {
		for ( size_t j = 0; j < table.y.size(); ++j ) {
			BOOST_CHECK(table.interpolate(value, false, true, table.x[i], table.y[j],
			                              &x1stDeriv, &x2ndDeriv, &y1stDeriv, &y2ndDeriv,
			                              &error));
			BOOST_CHECK(value == table.value(i, j));
		}
	}
}
