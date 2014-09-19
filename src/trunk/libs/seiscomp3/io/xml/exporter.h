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


#ifndef __SEISCOMP_IO_XMLEXPORTER_H__
#define __SEISCOMP_IO_XMLEXPORTER_H__


#include <seiscomp3/io/xml/handler.h>
#include <seiscomp3/io/exporter.h>

#include <ostream>
#include <map>


namespace Seiscomp {
namespace IO {
namespace XML {


class SC_SYSTEM_CORE_API Exporter : public IO::Exporter, public OutputHandler {
	// ------------------------------------------------------------------
	//  Xstruction
	// ------------------------------------------------------------------
	public:
		//! C'tor
		Exporter();


	// ----------------------------------------------------------------------
	// Public Interface
	// ----------------------------------------------------------------------
	public:
		TypeMap* typeMap();
		void setTypeMap(TypeMap *map);

	// ------------------------------------------------------------------
	//  Protected interface
	// ------------------------------------------------------------------
	protected:
		//! Sets the required name of the root tag.
		//! If empty no header is used.
		void setRootName(std::string);

		virtual void collectNamespaces(Core::BaseObject *);

		//! Interface method that must be implemented by real exporters.
		virtual bool put(std::streambuf* buf, Core::BaseObject *);


	// ------------------------------------------------------------------
	//  Private interface
	// ------------------------------------------------------------------
	private:
		void handle(Core::BaseObject *, const char *tag, const char *ns, NodeHandler *);
		bool openElement(const char *name, const char *ns);
		void addAttribute(const char *name, const char *ns, const char *value);
		void closeElement(const char *name, const char *ns);

		void put(const char *content);

		void writeString(const char *str);


	protected:
		// Maps a namespace to its prefix
		typedef std::map<std::string, std::string> NamespaceMap;

		NamespaceMap _defaultNsMap;
		NamespaceMap _namespaces;


	private:
		std::string  _headerNode;
		std::ostream _ostr;
		TypeMap     *_typemap;
		int          _lastTagState;
		int          _indent;
		bool         _tagOpen;
		bool         _firstElement;
};


}
}
}

#endif
