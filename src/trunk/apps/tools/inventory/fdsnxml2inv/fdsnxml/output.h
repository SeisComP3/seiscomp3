/***************************************************************************
 *   Copyright (C) 2013 by gempa GmbH
 *
 *   Author: Jan Becker
 *   Email: jabe@gempa.de $
 *
 ***************************************************************************/


#ifndef __SEISCOMP_FDSNXML_OUTPUT_H__
#define __SEISCOMP_FDSNXML_OUTPUT_H__


#include <fdsnxml/metadata.h>
#include <fdsnxml/types.h>
#include <seiscomp3/core/baseobject.h>
#include <seiscomp3/core/exceptions.h>


namespace Seiscomp {
namespace FDSNXML {


DEFINE_SMARTPOINTER(Output);


/**
 * \brief The type of data this channel collects. Corresponds to channel
 * \brief flags in SEED blockette 52. The SEED volume producer could use the
 * \brief first letter of an Output value as the SEED channel flag.
 */
class Output : public Core::BaseObject {
	DECLARE_CASTS(Output);
	DECLARE_RTTI;
	DECLARE_METAOBJECT_DERIVED;

	// ------------------------------------------------------------------
	//  Xstruction
	// ------------------------------------------------------------------
	public:
		//! Constructor
		Output();

		//! Copy constructor
		Output(const Output &other);

		//! Custom constructor
		Output(OutputType type);

		//! Destructor
		~Output();


	// ------------------------------------------------------------------
	//  Operators
	// ------------------------------------------------------------------
	public:
		//! Copies the metadata of other to this
		Output& operator=(const Output &other);
		bool operator==(const Output &other) const;


	// ------------------------------------------------------------------
	//  Setters/Getters
	// ------------------------------------------------------------------
	public:
		//! XML tag: type
		void setType(OutputType type);
		OutputType type() const;


	// ------------------------------------------------------------------
	//  Implementation
	// ------------------------------------------------------------------
	private:
		// Attributes
		OutputType _type;
};


}
}


#endif
