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


#ifndef __SC_LOGGING_INIT_H__
#define __SC_LOGGING_INIT_H__


namespace Seiscomp {
namespace Logging {

/*! @class Module <seiscomp3/logging/init.h>
	@brief Allows registration of external modules to log.

	Currently this only allows for initialization callbacks.  When init()
	is called, init() is called on all modules which have been registered.

	@author Valient Gough
*/
class SC_SYSTEM_CORE_API Module {
	public:
		virtual ~Module();

		/*! Called by init() to give the modules the command-line arguments */
		virtual void init( int &argv, char **argc );

		/*! Must be implemented to return the name of the module. */
		virtual const char *moduleName() const =0;
};

/*! @relates Module
	Registers the module - which will have init() called when ::init is
	called.
	Returns the module so that it can be used easily as a static initializer.
	@code
	class MyModule : public Seiscomp::Logging::Module
	{
	public:
		virtual const char *moduleName() const {return "MyModule";}
	};
	static Module * testModule = Seiscomp::Logging::RegisterModule( new MyModule() );
	@endcode
*/
SC_SYSTEM_CORE_API Module *RegisterModule(Module * module);

}
}

#endif
