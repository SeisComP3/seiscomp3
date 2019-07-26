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

%{
#include <seiscomp3/datamodel/exchange/quakeml.h>
%}

// QML Typemapper
%ignore Seiscomp::QML::Exporter;
%ignore Seiscomp::QML::RTExporter;
%ignore Seiscomp::QML::Importer;
%ignore Seiscomp::QML::Formatter;
%rename (QMLTypeMapper) Seiscomp::QML::TypeMapper;
%include "seiscomp3/datamodel/exchange/quakeml.h"
