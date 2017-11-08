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

#ifndef __SEISCOMP_GEO_GEOFEATURESET_H__
#define __SEISCOMP_GEO_GEOFEATURESET_H__

#include <seiscomp3/core.h>
#include <seiscomp3/geo/geofeature.h>

#include <vector>
#include <boost/filesystem/path.hpp>


namespace Seiscomp
{
namespace Geo
{
	
class SC_SYSTEM_CORE_API GeoFeatureSet : public Core::BaseObject {
public:
	/** Default constructor */
	GeoFeatureSet();
	/** Destructor */
	virtual ~GeoFeatureSet();
	/** Copy operator, intentionally left undefined */
	GeoFeatureSet & operator=(const GeoFeatureSet &);

	/**
	 * Removes and destructs all elements from the _features and
	 * _categories vectors
	 */
	void clear();

	/**
	 * Initializes the _feature vector with all BNA-files of the specified
	 * directory. The directory is searched recursively. The name of any
	 * subdirectory is used as a category.
	 */
	size_t readBNADir(const std::string& dirPath);

	/** Reads one BNA-file */
	bool readBNAFile(const std::string& filename, const Category* category);

	/** Returns reference to GeoFeature vector */
	const std::vector<GeoFeature*> &features() const { return _features; };
	
	/** Returns reference to Category vector */
	const std::vector<Category*> &categories() const { return _categories; };

private:
	/** Copy constructor, private -> non copyable */
	GeoFeatureSet(const GeoFeatureSet &);

	/** Reads a BNADir recursively, used by readBNADir() */
	size_t readBNADirRecursive(const boost::filesystem::path &directory,
	                           Category *category);

	/** Prints the number of polygons read */
	const std::string initStatus(const std::string &directory,
	                             unsigned int fileCount) const;

	/** Reads the BNA-header */
	bool readBNAHeader(std::string& segment, unsigned int& rank,
	                   unsigned int& points, bool& isClosed, std::string& error,
	                   const std::string &line) const;

	/** Compares two GeoFeatures by their rank */
	static const bool compareByRank(const GeoFeature* gf1,
	                        	const GeoFeature* gf2);
	
	/**
	 * Creates and inserts a new Category object into the Category vector.
	 * The name of the new category is set to the name parameter, prefixed
	 * (if available) by the name of the parent category.
	 */
	Category* addNewCategory(const std::string name,
	                         const Category* parent = NULL);


private:
	/** Vector of GeoFeatures */
	std::vector<GeoFeature*> _features;
	
	/** Vector of Categories */
	std::vector<Category*> _categories;
};

class GeoFeatureSetSingleton {
	public:
		/** Returns the singleton instance of this class */
		static const GeoFeatureSet &getInstance();

	private:
		/** Default constructor */
		GeoFeatureSetSingleton();

	private:
		GeoFeatureSet         _geoFeatureSet;
};

} // of ns Geo
} // of ns Seiscomp

#endif // __SEISCOMP_GEO_FEATURESET_H__
