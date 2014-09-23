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

// This file was created by a source code generator.
// Do not modify the contents. Change the definition and run the generator
// again!


#ifndef __SEISCOMP_DATAMODEL_MOMENTTENSOR_H__
#define __SEISCOMP_DATAMODEL_MOMENTTENSOR_H__


#include <seiscomp3/datamodel/creationinfo.h>
#include <string>
#include <seiscomp3/datamodel/sourcetimefunction.h>
#include <seiscomp3/datamodel/types.h>
#include <vector>
#include <seiscomp3/datamodel/tensor.h>
#include <seiscomp3/datamodel/realquantity.h>
#include <seiscomp3/datamodel/comment.h>
#include <seiscomp3/datamodel/momenttensorphasesetting.h>
#include <seiscomp3/datamodel/notifier.h>
#include <seiscomp3/datamodel/publicobject.h>
#include <seiscomp3/core/exceptions.h>


namespace Seiscomp {
namespace DataModel {


DEFINE_SMARTPOINTER(MomentTensor);
DEFINE_SMARTPOINTER(Comment);
DEFINE_SMARTPOINTER(DataUsed);
DEFINE_SMARTPOINTER(MomentTensorPhaseSetting);
DEFINE_SMARTPOINTER(MomentTensorStationContribution);

class FocalMechanism;


/**
 * \brief This class represents a moment tensor solution for an
 * \brief event. It is an
 * \brief optional part of a FocalMechanism description.
 */
class SC_SYSTEM_CORE_API MomentTensor : public PublicObject {
	DECLARE_SC_CLASS(MomentTensor);
	DECLARE_SERIALIZATION;
	DECLARE_METAOBJECT;

	// ------------------------------------------------------------------
	//  Xstruction
	// ------------------------------------------------------------------
	protected:
		//! Protected constructor
		MomentTensor();

	public:
		//! Copy constructor
		MomentTensor(const MomentTensor& other);

		//! Constructor with publicID
		MomentTensor(const std::string& publicID);

		//! Destructor
		~MomentTensor();
	

	// ------------------------------------------------------------------
	//  Creators
	// ------------------------------------------------------------------
	public:
		static MomentTensor* Create();
		static MomentTensor* Create(const std::string& publicID);


	// ------------------------------------------------------------------
	//  Lookup
	// ------------------------------------------------------------------
	public:
		static MomentTensor* Find(const std::string& publicID);


	// ------------------------------------------------------------------
	//  Operators
	// ------------------------------------------------------------------
	public:
		//! Copies the metadata of other to this
		//! No changes regarding child objects are made
		MomentTensor& operator=(const MomentTensor& other);
		//! Checks for equality of two objects. Childs objects
		//! are not part of the check.
		bool operator==(const MomentTensor& other) const;
		bool operator!=(const MomentTensor& other) const;

		//! Wrapper that calls operator==
		bool equal(const MomentTensor& other) const;


	// ------------------------------------------------------------------
	//  Setters/Getters
	// ------------------------------------------------------------------
	public:
		//! Refers to the publicID of the Origin derived in the moment
		//! tensor inversion.
		void setDerivedOriginID(const std::string& derivedOriginID);
		const std::string& derivedOriginID() const;

		//! Refers to the publicID of the Magnitude object which
		//! represents the derived moment
		//! magnitude.
		void setMomentMagnitudeID(const std::string& momentMagnitudeID);
		const std::string& momentMagnitudeID() const;

		//! Scalar moment as derived in moment tensor inversion in Nm.
		void setScalarMoment(const OPT(RealQuantity)& scalarMoment);
		RealQuantity& scalarMoment() throw(Seiscomp::Core::ValueException);
		const RealQuantity& scalarMoment() const throw(Seiscomp::Core::ValueException);

		//! Tensor object holding the moment tensor elements.
		void setTensor(const OPT(Tensor)& tensor);
		Tensor& tensor() throw(Seiscomp::Core::ValueException);
		const Tensor& tensor() const throw(Seiscomp::Core::ValueException);

		//! Variance of moment tensor inversion.
		void setVariance(const OPT(double)& variance);
		double variance() const throw(Seiscomp::Core::ValueException);

		//! Variance reduction of moment tensor inversion, given in
		//! percent (Dreger 2003). This is a
		//! goodness-of-fit measure.
		void setVarianceReduction(const OPT(double)& varianceReduction);
		double varianceReduction() const throw(Seiscomp::Core::ValueException);

		//! Double couple parameter obtained from moment tensor
		//! inversion (decimal fraction between 0
		//! and 1).
		void setDoubleCouple(const OPT(double)& doubleCouple);
		double doubleCouple() const throw(Seiscomp::Core::ValueException);

		//! CLVD (compensated linear vector dipole) parameter obtained
		//! from moment tensor inversion (decimal
		//! fraction between 0 and 1).
		void setClvd(const OPT(double)& clvd);
		double clvd() const throw(Seiscomp::Core::ValueException);

		//! Isotropic part obtained from moment tensor inversion
		//! (decimal fraction between 0 and 1).
		void setIso(const OPT(double)& iso);
		double iso() const throw(Seiscomp::Core::ValueException);

		//! Resource identifier of the Green's function used in moment
		//! tensor inversion.
		void setGreensFunctionID(const std::string& greensFunctionID);
		const std::string& greensFunctionID() const;

		//! Resource identifier of the filter setup used in moment
		//! tensor inversion.
		void setFilterID(const std::string& filterID);
		const std::string& filterID() const;

		//! Source time function used in moment-tensor inversion.
		void setSourceTimeFunction(const OPT(SourceTimeFunction)& sourceTimeFunction);
		SourceTimeFunction& sourceTimeFunction() throw(Seiscomp::Core::ValueException);
		const SourceTimeFunction& sourceTimeFunction() const throw(Seiscomp::Core::ValueException);

		//! Resource identifier of the method used for moment-tensor
		//! inversion.
		void setMethodID(const std::string& methodID);
		const std::string& methodID() const;

		//! Moment tensor method used.
		void setMethod(const OPT(MomentTensorMethod)& method);
		MomentTensorMethod method() const throw(Seiscomp::Core::ValueException);

		//! Status of moment tensor.
		void setStatus(const OPT(MomentTensorStatus)& status);
		MomentTensorStatus status() const throw(Seiscomp::Core::ValueException);

		void setCmtName(const std::string& cmtName);
		const std::string& cmtName() const;

		void setCmtVersion(const std::string& cmtVersion);
		const std::string& cmtVersion() const;

		//! CreationInfo for the MomentTensor object.
		void setCreationInfo(const OPT(CreationInfo)& creationInfo);
		CreationInfo& creationInfo() throw(Seiscomp::Core::ValueException);
		const CreationInfo& creationInfo() const throw(Seiscomp::Core::ValueException);

	
	// ------------------------------------------------------------------
	//  Public interface
	// ------------------------------------------------------------------
	public:
		/**
		 * Add an object.
		 * @param obj The object pointer
		 * @return true The object has been added
		 * @return false The object has not been added
		 *               because it already exists in the list
		 *               or it already has another parent
		 */
		bool add(Comment* obj);
		bool add(DataUsed* obj);
		bool add(MomentTensorPhaseSetting* obj);
		bool add(MomentTensorStationContribution* obj);

		/**
		 * Removes an object.
		 * @param obj The object pointer
		 * @return true The object has been removed
		 * @return false The object has not been removed
		 *               because it does not exist in the list
		 */
		bool remove(Comment* obj);
		bool remove(DataUsed* obj);
		bool remove(MomentTensorPhaseSetting* obj);
		bool remove(MomentTensorStationContribution* obj);

		/**
		 * Removes an object of a particular class.
		 * @param i The object index
		 * @return true The object has been removed
		 * @return false The index is out of bounds
		 */
		bool removeComment(size_t i);
		bool removeComment(const CommentIndex& i);
		bool removeDataUsed(size_t i);
		bool removeMomentTensorPhaseSetting(size_t i);
		bool removeMomentTensorPhaseSetting(const MomentTensorPhaseSettingIndex& i);
		bool removeMomentTensorStationContribution(size_t i);

		//! Retrieve the number of objects of a particular class
		size_t commentCount() const;
		size_t dataUsedCount() const;
		size_t momentTensorPhaseSettingCount() const;
		size_t momentTensorStationContributionCount() const;

		//! Index access
		//! @return The object at index i
		Comment* comment(size_t i) const;
		Comment* comment(const CommentIndex& i) const;
		DataUsed* dataUsed(size_t i) const;

		MomentTensorPhaseSetting* momentTensorPhaseSetting(size_t i) const;
		MomentTensorPhaseSetting* momentTensorPhaseSetting(const MomentTensorPhaseSettingIndex& i) const;
		MomentTensorStationContribution* momentTensorStationContribution(size_t i) const;

		//! Find an object by its unique attribute(s)
		DataUsed* findDataUsed(DataUsed* dataUsed) const;
		MomentTensorStationContribution* findMomentTensorStationContribution(const std::string& publicID) const;

		FocalMechanism* focalMechanism() const;

		//! Implement Object interface
		bool assign(Object* other);
		bool attachTo(PublicObject* parent);
		bool detachFrom(PublicObject* parent);
		bool detach();

		//! Creates a clone
		Object* clone() const;

		//! Implement PublicObject interface
		bool updateChild(Object* child);

		void accept(Visitor*);


	// ------------------------------------------------------------------
	//  Implementation
	// ------------------------------------------------------------------
	private:
		// Attributes
		std::string _derivedOriginID;
		std::string _momentMagnitudeID;
		OPT(RealQuantity) _scalarMoment;
		OPT(Tensor) _tensor;
		OPT(double) _variance;
		OPT(double) _varianceReduction;
		OPT(double) _doubleCouple;
		OPT(double) _clvd;
		OPT(double) _iso;
		std::string _greensFunctionID;
		std::string _filterID;
		OPT(SourceTimeFunction) _sourceTimeFunction;
		std::string _methodID;
		OPT(MomentTensorMethod) _method;
		OPT(MomentTensorStatus) _status;
		std::string _cmtName;
		std::string _cmtVersion;
		OPT(CreationInfo) _creationInfo;

		// Aggregations
		std::vector<CommentPtr> _comments;
		std::vector<DataUsedPtr> _dataUseds;
		std::vector<MomentTensorPhaseSettingPtr> _momentTensorPhaseSettings;
		std::vector<MomentTensorStationContributionPtr> _momentTensorStationContributions;

	DECLARE_SC_CLASSFACTORY_FRIEND(MomentTensor);
};


}
}


#endif
