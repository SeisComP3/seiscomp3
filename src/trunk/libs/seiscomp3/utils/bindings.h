/***************************************************************************
 * Copyright (C) 2015 by gempa GmbH
 *
 * Author: Jan Becker
 * Email: jabe@gempa.de
 ***************************************************************************/


#include <seiscomp3/core/metaobject.h>
#include <seiscomp3/datamodel/station.h>
#include <seiscomp3/datamodel/configmodule.h>
#include <seiscomp3/utils/keyvalues.h>
#include <seiscomp3/core.h>

#include <map>


namespace Seiscomp {
namespace Util {


DEFINE_SMARTPOINTER(Bindings);

/**
 * @brief The Bindings class holds a map of all stations and its corresponding
 *        parameter set.
 *
 * The bindings are initialized from a Seiscomp::DataModel::ConfigModule object.
 * It provides a simple mechanims to iterate over all bound stations with its
 * parameters.
 */
class SC_SYSTEM_CORE_API Bindings : public Core::BaseObject {
	// ----------------------------------------------------------------------
	//  Public types
	// ----------------------------------------------------------------------
	public:
		struct Binding {
			KeyValuesPtr    keys;
			Core::MetaValue data;
		};

		typedef std::map<std::string, Binding> StationMap;
		typedef std::map<std::string, StationMap> NetworkMap;

		/**
		 * @brief The const_iterator class to iterate through bindings.
		 *
		 * It provides access to the current station code, network code,
		 * parameter list and user data.
		 */
		class const_iterator {
			public:
				const_iterator() {}

			public:
				// Prefix ++
				const_iterator &operator++() {
					++_sIt;
					if ( _sIt == _stations->end() ) {
						++_nIt;
						if ( _nIt != _networks->end() ) {
							_stations = &_nIt->second;
							_sIt = _stations->begin();
						}
						else {
							*this = const_iterator();
						}
					}

					return *this;
				}

				// Postfix ++
				const_iterator operator++(int) {
					const_iterator res(*this);
					++(*this);
					return res;
				}

				bool operator==(const const_iterator &other) const {
					return _nIt == other._nIt && _sIt == other._sIt;
				}

				bool operator!=(const const_iterator &other) const {
					return _nIt != other._nIt || _sIt != other._sIt;
				}


			public:
				/**
				 * @brief Returns the network code of the current element.
				 *
				 * If calling this function on a singular iterator the behaviour
				 * is undefined.
				 * @return The network code.
				 */
				const std::string &networkCode() const {
					return _nIt->first;
				}

				/**
				 * @brief Returns the station code of the current element.
				 *
				 * If calling this function on a singular iterator the behaviour
				 * is undefined.
				 * @return The station code.
				 */
				const std::string &stationCode() const {
					return _sIt->first;
				}

				/**
				 * @brief Returns the parameter list of the current element.
				 *
				 * If calling this function on a singular iterator the behaviour
				 * is undefined.
				 * @return A pointer to the parameter list.
				 */
				const KeyValues *keys() const {
					return _sIt->second.keys.get();
				}

				/**
				 * @brief Returns the user data of the current element.
				 *
				 * If calling this function on a singular iterator the behaviour
				 * is undefined.
				 * @return The user data.
				 */
				const Core::MetaValue &data() const {
					return _sIt->second.data;
				}


			private:
				const NetworkMap          *_networks;
				const StationMap          *_stations;

				NetworkMap::const_iterator _nIt;
				StationMap::const_iterator _sIt;


			friend class Bindings;
		};


	// ----------------------------------------------------------------------
	//  X'truction
	// ----------------------------------------------------------------------
	public:
		Bindings();


	// ----------------------------------------------------------------------
	//  Public methods
	// ----------------------------------------------------------------------
	public:
		/**
		 * @brief Builds the bindings map from a ConfigModule and compiles
		 *        all inherited ParameterSets into a single list of key-value
		 *        pairs per station.
		 * @param mod The ConfigModule, e.g. Application::configModule()
		 * @param setupName The name of the setup to be read, e.g. Application::name()
		 * @param allowGlobal Whether global bindings are allowed to be read if
		 *        an application (setup) specific bindings has not been found.
		 *        E.g. if scautopick reads its bindings, it sets allowGlobal to
		 *        false. That means, even if a global binding for a station
		 *        exists, it won't be read unless an scautopick binding exists
		 *        as well.
		 * @return true if everything went smoothly, false otherwise.
		 */
		bool init(const DataModel::ConfigModule *mod,
		          const std::string &setupName,
		          bool allowGlobal);

		/**
		 * @brief Returns the associated parameter list for a station.
		 * @param networkCode The stations network code
		 * @param stationCode The station code
		 * @return A pointer to the parameter list if available, NULL otherwise.
		 */
		const KeyValues *getKeys(const std::string &networkCode,
		                         const std::string &stationCode);

		/**
		 * @brief Convenience function for getKeys(netCode, staCode)
		 * @param station The station object
		 * @return A pointer to the parameter list if available, NULL otherwise.
		 */
		const KeyValues *getKeys(const DataModel::Station *station);

		/**
		 * @brief Removes a binding for a station.
		 * @param networkCode The network code of the station
		 * @param stationCode The code of the station
		 * @return true if removed, false otherwise
		 */
		bool remove(const std::string &networkCode,
		            const std::string &stationCode);

		/**
		 * @brief Convenience function that takes a station object.
		 * @param station The station object
		 * @return true if removed, false otherwise
		 */
		bool remove(const DataModel::Station *station);

		/**
		 * @brief Binds a user defined object to a station. That can
		 *        be used for application specific extensions.
		 * @param networkCode The network code
		 * @param stationCode The station code
		 * @param value The MetaValue to be bound.
		 * @return True if station is bound already, false otherwise.
		 */
		bool setData(const std::string &networkCode,
		             const std::string &stationCode,
		             const Core::MetaValue &value);

		/**
		 * @brief Convenience function that takes a station object.
		 * @param station The station the value should be associated with.
		 * @param value The MetaValue to be bound.
		 * @return True if station is bound already, false otherwise.
		 */
		bool setData(const DataModel::Station *station,
		             const Core::MetaValue &value);

		/**
		 * @brief Returns the associated user data of a station.
		 * @param networkCode The network code
		 * @param stationCode The station code
		 * @return The associated meta value if found, an empty meta value
		 *         otherwise.
		 */
		const Core::MetaValue &data(const std::string &networkCode,
		                            const std::string &stationCode);

		/**
		 * @brief Convenience function that takes a station object.
		 * @param station The network code
		 * @return The associated meta value if found, an empty meta value
		 *         otherwise.
		 */
		const Core::MetaValue &data(const DataModel::Station *station);

		/**
		 * @brief Returns an iterator to the first element of the bindings.
		 *        If the bindings are empty, the returned iterator will be
		 *        equal to end.
		 * @return The iterator pointing to the first element or end().
		 */
		const_iterator begin() const;

		/**
		 * @brief Returns an iterator to the element following the last
		 *        element of the bindings. Attempting to acces it results
		 *        in undefined behaviour.
		 * @return The iterator pointing past the last element.
		 */
		const_iterator end() const;


	// ----------------------------------------------------------------------
	//  Private members
	// ----------------------------------------------------------------------
	private:
		NetworkMap  _bindings;
};


}
}
