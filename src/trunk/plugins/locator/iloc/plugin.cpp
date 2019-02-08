/***************************************************************************
 * Copyright (C) 2019 by gempa GmbH                                        *
 *                                                                         *
 * All Rights Reserved.                                                    *
 *                                                                         *
 * NOTICE: All information contained herein is, and remains                *
 * the property of gempa GmbH and its suppliers, if any. The intellectual  *
 * and technical concepts contained herein are proprietary to gempa GmbH   *
 * and its suppliers.                                                      *
 * Dissemination of this information or reproduction of this material      *
 * is strictly forbidden unless prior written permission is obtained       *
 * from gempa GmbH.                                                        *
 *                                                                         *
 * Author: Jan Becker                                                      *
 * Email: jabe@gempa.de                                                    *
 ***************************************************************************/


#include <seiscomp3/core/plugin.h>
#include <seiscomp3/core/version.h>


#if SC_API_VERSION >= SC_API_VERSION_CHECK(10,0,0)

ADD_SC_PLUGIN(
	"Locator test implementation for iLoc",
	"Jan Becker, gempa GmbH",
	0, 5, 0
)


#endif
