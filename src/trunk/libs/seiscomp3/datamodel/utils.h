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


#ifndef __SEISCOMP_DATAMODEL_UTIL_H__
#define __SEISCOMP_DATAMODEL_UTIL_H__

#include <seiscomp3/core/datetime.h>
#include <seiscomp3/core/exceptions.h>
#include <seiscomp3/core.h>
#include <seiscomp3/datamodel/notifier.h>
#include <seiscomp3/datamodel/object.h>
#include <seiscomp3/datamodel/types.h>
#include <iostream>
#include <string>
#include <vector>


namespace Seiscomp {
namespace DataModel {

class Event;
class Origin;
class Pick;
class Inventory;
class Station;
class SensorLocation;
class Stream;
class ConfigStation;
class Setup;


SC_SYSTEM_CORE_API std::string eventRegion(const DataModel::Event *);


template <typename T>
char objectStatusToChar(const T *o) {
	try {
		switch ( o->evaluationStatus() ) {
			case PRELIMINARY:
				return 'P';
			case CONFIRMED:
				return 'C';
			case REVIEWED:
				return 'V';
			case FINAL:
				return 'F';
			case REJECTED:
				return 'X';
			case REPORTED:
				return 'R';
			default:
				break;
		}
	}
	catch ( ... ) {}

	try {
		if ( o->evaluationMode() == 'M' )
			return 'M';
	}
	catch ( ... ) {}

	return 'A';
}


template <typename T>
std::string objectAgencyID(const T *o) {
	try {
		return o->creationInfo().agencyID();
	}
	catch ( Core::ValueException & ) {}

	return "";
}


template <typename T>
std::string objectAuthor(const T *o) {
	try {
		return o->creationInfo().author();
	}
	catch ( Core::ValueException & ) {}

	return "";
}


template <typename T>
double quantityUncertainty(const T &o) {
	try {
		return o.uncertainty();
	}
	catch ( Core::ValueException & ) {
		return (o.lowerUncertainty() + o.upperUncertainty()) * 0.5;
	}
}

MAKEENUM(
	InventoryError,
	EVALUES(
		NETWORK_CODE_NOT_FOUND,
		NETWORK_EPOCH_NOT_FOUND,
		STATION_CODE_NOT_FOUND,
		STATION_EPOCH_NOT_FOUND,
		SENSOR_CODE_NOT_FOUND,
		SENSOR_EPOCH_NOT_FOUND,
		STREAM_CODE_NOT_FOUND,
		STREAM_EPOCH_NOT_FOUND
	),
	ENAMES(
		"network code not found",
		"no matching network epoch found",
		"station code not found",
		"no matching station epoch found",
		"sensor location code not found",
		"no matching sensor location epoch found",
		"stream code not found",
		"no matching stream epoch found"
	)
);


//! Returns the station for a network- and stationcode and
//! a time. If the station has not been found NULL will be returned.
SC_SYSTEM_CORE_API
Station* getStation(const Inventory *inventory,
                    const std::string &networkCode,
                    const std::string &stationCode,
                    const Core::Time &, InventoryError *error = NULL);

//! Returns the sensorlocation for a network-, station- and locationcode and
//! a time. If the sensorlocation has not been found NULL will be returned.
SC_SYSTEM_CORE_API
SensorLocation* getSensorLocation(const Inventory *inventory,
                                  const std::string &networkCode,
                                  const std::string &stationCode,
                                  const std::string &locationCode,
                                  const Core::Time &,
                                  InventoryError *error = NULL);

//! Returns the stream for a network-, station-, location- and channelcode and
//! a time. If the stream has not been found NULL will be returned.
SC_SYSTEM_CORE_API
Stream* getStream(const Inventory *inventory,
                  const std::string &networkCode,
                  const std::string &stationCode,
                  const std::string &locationCode,
                  const std::string &channelCode,
                  const Core::Time &,
                  InventoryError *error = NULL);

//! Returns the station used for a pick. If the station has not been found
//! NULL will be returned.
SC_SYSTEM_CORE_API
Station* getStation(const Inventory *inventory, const Pick *pick);


//! Returns the sensor location used for a pick. If the sensor location has
//! not been found NULL will be returned.
SC_SYSTEM_CORE_API
SensorLocation* getSensorLocation(const Inventory *inventory,
                                  const Pick *pick);


//! Returns the stream used for a pick. If the stream has
//! not been found NULL will be returned.
Stream* getStream(const Inventory *inventory,
                  const Pick *pick);


struct ThreeComponents {
	enum Component {
		Vertical         = 0,  /* usually Z */
		FirstHorizontal  = 1,  /* usually N */
		SecondHorizontal = 2   /* usually E */
	};

	Stream *vertical() const { return comps[Vertical]; }
	Stream *firstHorizontal() const { return comps[FirstHorizontal]; }
	Stream *secondHorizontal() const { return comps[SecondHorizontal]; }

	ThreeComponents();

	Stream  *comps[3];
};


//! Returns the best matching stream for the vertical component having
//! the stream code set to streamCode (without component code, first two letters).
//! The method returns the component with the lowest horizontal dip. If several
//! components share the same dip the first one found is returned, e.g. UVW or ABC
//! orientations.
SC_SYSTEM_CORE_API Stream *
getVerticalComponent(const SensorLocation *loc, const char *streamCode, const Core::Time &time);

//! Returns the best matching streams for the vertical and horizontal components
//! having the stream code set to streamCode (without component code, first two letters).
//! Returns true if the resulting 3 components are forming an orthogonal system,
//! false otherwise.
//! The method tries to find 3 orthogonal components and select the first with
//! the lowest dip (largest Z value) as vertical. The remaining two are returned
//! as 1st horizontal (Y axis or North) and 2nd horizontal (X axis or East)
//! respectively.
//! NOTE: Each of the comps entries in res can be NULL.
SC_SYSTEM_CORE_API bool
getThreeComponents(ThreeComponents &res, const SensorLocation *loc, const char *streamCode, const Core::Time &time);


/**
 * Looks for a setup which name is [setupName] (or "global" as fallback).
 * @param configStation The ConfigStation object with Setup's attached
 * @param setupName The name compared with Setup::name()
 * @param allowGlobal Defines if "global" is allowed as fallback setup
 *                    if setupName has not been found explicitely.
 * @return The setup if available, NULL otherwise
 */
SC_SYSTEM_CORE_API Setup *
findSetup(const ConfigStation *configStation, const std::string &setupName,
          bool allowGlobal = true);


/**
 * Creates a deep copy of an object including all child objects.
 */
SC_SYSTEM_CORE_API Object *copy(Object* obj);


///////////////////////////////////////////////////////////////////////////////
// DataModel Diff
///////////////////////////////////////////////////////////////////////////////
class SC_SYSTEM_CORE_API DiffMerge {
	public:
		DiffMerge();


	public:
		void setLoggingLevel(int level);
		void showLog(std::ostream &os = std::cerr, int padding = 0,
		             int indent = 1, bool ignoreFirstPad = false);

		/**
		 * Scans a object tree for a particular node. Objects are compared on base of
		 * their indexes, @see equalsIndex
		 * @param tree object tree to scan
		 * @param node item to search for
		 * @return pointer to the item within the object tree or NULL if the node was
		 * not found
		 */
		Object *find(Object *tree, Object *node);

		/**
		 * Recursively compares two objects and collects all differences.
		 * The root element of one of the objects must be included in the other object
		 * tree, @see find(o1, o2)
		 * @param o1 first object to compare
		 * @param o2 second object to compare
		 * @param diffList list to collect differences in
		 * @return true if the diff could be performed, false if one object was null or
		 * no common child object could be found
		 * @throw TypeException if any type restriction is violated
		 */
		bool diff(Object *o1, Object *o2, std::vector<NotifierPtr> &diffList);

		/**
		 * Recursively compares two objects and collects all differences.
		 * The root element of one of the objects must be included in the other object
		 * tree, @see find(o1, o2)
		 * @param o1 first object to compare
		 * @param o2 second object to compare
		 * @return A notifier message with all diff notifiers
		 * @throw TypeException if any type restriction is violated
		 */
		NotifierMessage *diff2Message(Object *o1, Object *o2);

		/**
		 * Recursively merges the node object (and its children) into the tree object.
		 * The node must be part of the tree, @ref find(o1, o2). Properties of the node
		 * object override properties of the tree.
		 * @param tree main object to merge the node into
		 * @param node object to be merged into the tree
		 * @param idMap map that keeps track of any publicID attribute changes applied
		 * during the merge
		 * @return true if the merge could be performed, false if the node was not found
		 * in the tree
		 * @throw TypeException if any type restriction is violated
		 */
		bool merge(Object *tree, Object *node, std::map<std::string, std::string> &idMap);

		/**
		 * Merges all all objects in the vector in order of their appearance into the
		 * mergeResult object, @ref merge(Object*, Object*). The mergeResult object must
		 * be not null and must serve as a parent for the objects being merged. In a
		 * subsequent processing step changes to publicIDs are applied to references,
		 * @ref mapReferences.
		 * @param mergeResult object to merge the vector into
		 * @param objects vector of objects to merge
		 * @return true if all objects could be merged successfully, else false.
		 */
		bool merge(Object *mergeResult, const std::vector<Object*> &objects);

		/**
		 * Validates the internal publicID references of the specified object. In a
		 * first step all publicIDs are collected, then the object is traversed top-down
		 * and each reference's value is searched in the publicID set.
		 * @param o object to validate
		 * @return true if all references point to an existing publicID, else false
		 */
		bool validateReferences(Object *o);

		/**
		 * Maps publicID references of the specified object. While the object is
		 * traversed top-down, a lookup for each reference in the publicID map is
		 * performed. If a matching entry is found the reference's value is updated.
		 * @param o object which references should be mapped
		 * @param map publicIDMap of deprecated to current publicIDs
		 * @return number of mappings performed
		 */
		size_t mapReferences(Object *o, const std::map<std::string, std::string> &publicIDMap);

		bool compareObjects(const Object *o1, const Object *o2);


	private:
		std::string getPublicID(Object* o);
		void diffRecursive(Object *o1, Object *o2, const std::string &o1ParentID,
		                   std::vector<NotifierPtr> &diffList);

		bool equalsIndex(Object *o1, Object *o2);
		bool compareNonArrayProperty(const Core::BaseObject *o1, const Core::BaseObject *o2);
		bool compareNonArrayProperty(const Core::MetaProperty* prop,
		                             const Core::BaseObject *o1, const Core::BaseObject *o2);
		void mergeRecursive(Object *o1, Object *o2,
		                    std::map<std::string, std::string> &idMap);


	private:
		DEFINE_SMARTPOINTER(LogNode);
		class LogNode: public Core::BaseObject {
			private:
				LogNode                 *_parent;
				std::string              _title;

				// Logging level
				// 2 Logs all (Default)
				// 1 Logs only differences
				// 0 Logs none - no messages are accepted
				int                      _level;
				std::vector<LogNodePtr>  _childs;
				std::vector<std::string> _messages;

				std::string o2t(const Object *o);
				void setParent(LogNode *n);
				void add(std::string s1, std::string n);

				template <class T>
				std::string compare(T a, T b, bool quotes = false);

				void reset ();
			
			public:
				LogNode(const Object* o1, int level);
				LogNode(std::string title, int level);
				
			public:
				template <class T>
				void add(std::string title, T a, T b);

				void add(std::string title, bool status, std::string comment);
				void add(std::string title, const Object *o1);
				LogNodePtr add(LogNodePtr child);

				void show(std::ostream &os, int padding = 0, int indent = 1,
				          bool ignoreFirstPad = false);
		};


	private:
		LogNodePtr _currentNode;
		int        _logLevel;
};


} // of ns DataModel
} // of ns Seiscomp


#endif
