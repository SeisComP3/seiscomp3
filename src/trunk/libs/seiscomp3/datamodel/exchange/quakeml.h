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


#ifndef __SEISCOMP_QML_XML_H__
#define __SEISCOMP_QML_XML_H__

#include <string>

#include <seiscomp3/datamodel/types.h>

#include <seiscomp3/io/xml/exporter.h>
#include <seiscomp3/io/xml/handler.h>


namespace Seiscomp {
namespace QML {

class TypeMapper {
	public:
		/**
		 * @brief Maps an QML event type string to the EventType enumeration.
		 *
		 * If the conversion fails, an ValueException is thrown.
		 *
		 * @param str The input string
		 * @return The mapped event type or an exception
		 */
		static DataModel::EventType EventTypeFromString(const std::string &str);

		/**
		 * @brief Maps an EventType to an QML event type string.
		 * @param type The EventType
		 * @return The QML event type string
		 */
		static std::string EventTypeToString(DataModel::EventType type);
};

class Exporter : public IO::XML::Exporter {
	public:
		Exporter();

	protected:
		virtual void collectNamespaces(Core::BaseObject *);
};

class RTExporter : public IO::XML::Exporter {
	public:
		RTExporter();

	protected:
		virtual void collectNamespaces(Core::BaseObject *);
};

struct Formatter {
	virtual void to(std::string& v) {}
	virtual void from(std::string& v) {}
};

template <typename T>
struct TypedClassHandler : IO::XML::TypedClassHandler<T> {
	void add(const char *property, const char *name, Formatter *format = NULL,
	         IO::XML::ClassHandler::Type t = IO::XML::ClassHandler::Optional,
	         IO::XML::ClassHandler::Location l = IO::XML::ClassHandler::Element);
	void add(const char *property, Formatter *format = NULL,
	         IO::XML::ClassHandler::Type t = IO::XML::ClassHandler::Optional,
	         IO::XML::ClassHandler::Location l = IO::XML::ClassHandler::Element);
	void addList(const char *properties,
	             IO::XML::ClassHandler::Type t = IO::XML::ClassHandler::Optional,
	             IO::XML::ClassHandler::Location l = IO::XML::ClassHandler::Element);
	void addPID();
	void addEmptyPID();
};

}
}


#endif
