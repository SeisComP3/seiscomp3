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


#ifndef __SEISCOMP_IO_XMLIMPORTER_H__
#define __SEISCOMP_IO_XMLIMPORTER_H__


#include <seiscomp3/io/xml/handler.h>
#include <seiscomp3/io/importer.h>


namespace Seiscomp {
namespace IO {
namespace XML {


class SC_SYSTEM_CORE_API Importer : public IO::Importer {
	// ------------------------------------------------------------------
	//  Xstruction
	// ------------------------------------------------------------------
	public:
		//! C'tor
		Importer();


	// ------------------------------------------------------------------
	//  Public interface
	// ------------------------------------------------------------------
	public:
		TypeMap* typeMap();
		void setTypeMap(TypeMap *map);

		//! Enables/disables strict namespace checking.
		//! If disabled, tags will be accepted even if the
		//! registered namespace doesn't match and the tag
		//! is only registered with one namespace. Tags with
		//! multiple namespaces will still fail.
		void setStrictNamespaceCheck(bool);

	// ----------------------------------------------------------------------
	// Protected Inteface
	// ----------------------------------------------------------------------
	protected:

		//! Sets the required name of the root tag.
		//! If empty no header is used.
		void setRootName(std::string);

		//! Interface method that must be implemented by real importers.
		virtual Core::BaseObject *get(std::streambuf* buf);


	// ------------------------------------------------------------------
	//  Private interface
	// ------------------------------------------------------------------
	private:
		bool traverse(NodeHandler *handler,
		              void *node, void *childs,
		              Core::BaseObject *target);


	private:
		static NoneHandler _none;
		GenericHandler _any;
		bool _strictNamespaceCheck;

		Core::BaseObject *_result;
		std::string _headerNode;
		TypeMap *_typemap;
};


}
}
}

#endif
