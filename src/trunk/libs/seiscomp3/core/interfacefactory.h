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


#ifndef __SC_CORE_INTERFACEFACTORY_H__
#define __SC_CORE_INTERFACEFACTORY_H__

#include <map>
#include <vector>
#include <string>

namespace Seiscomp {
namespace Core {
namespace Generic {
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<





// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
/** \brief Template based service factory interface

	To create an object, use the following code
	\code
	InterfaceFactory::Create(servicename);
	\endcode
 */
template <typename T>
class InterfaceFactoryInterface {
	public:
		typedef T Interface;
		typedef std::vector<InterfaceFactoryInterface<T>*> ServicePool;
		typedef std::vector<std::string> ServiceNames;


	protected:
		InterfaceFactoryInterface() {}
		InterfaceFactoryInterface(const char* serviceName);

	public:
		virtual ~InterfaceFactoryInterface();


	public:
		static T* Create(const char* serviceName);

		static unsigned int ServiceCount();

		static ServiceNames* Services();

		static InterfaceFactoryInterface* Find(const char* serviceName);

		const char* serviceName() const;

		virtual Interface* create() const = 0;


	private:
		static bool RegisterFactory(InterfaceFactoryInterface* factory);

		static bool UnregisterFactory(InterfaceFactoryInterface* factory);

		static ServicePool &Pool();

	private:
		std::string _serviceName;
};

#define DEFINE_INTERFACE_FACTORY(Class) \
typedef Seiscomp::Core::Generic::InterfaceFactoryInterface<Class> Class##Factory

#define DEFINE_TEMPLATE_INTERFACE_FACTORY(Class) \
template <typename T> \
struct Class##Factory : Seiscomp::Core::Generic::InterfaceFactoryInterface< Class<T> > {}


#define IMPLEMENT_INTERFACE_FACTORY(Class, APIDef) \
template class APIDef Seiscomp::Core::Generic::InterfaceFactoryInterface<Class>
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<





// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
/**	\brief Template based service factory
 */
template <typename T, typename TYPE>
class InterfaceFactory : public InterfaceFactoryInterface<T> {
	public:
		//! The type that represents the actual polymorphic class.
		typedef TYPE Type;

	public:
		InterfaceFactory(const char* serviceName)
		 : InterfaceFactoryInterface<T>(serviceName) {}

	public:
		//! The actual creation
		typename InterfaceFactoryInterface<T>::Interface* create() const { return new TYPE; }
};
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<





// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
#define DECLARE_INTERFACEFACTORY_FRIEND(Interface, Class) \
friend class Seiscomp::Core::Generic::InterfaceFactory<Interface, Class>
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

}
}
}

#endif
