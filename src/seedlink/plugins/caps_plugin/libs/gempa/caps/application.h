/***************************************************************************
 * libcapsclient
 * Copyright (C) 2016  gempa GmbH
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 ***************************************************************************/


#ifndef __GEMPA_CAPS_APPLICATION_H__
#define __GEMPA_CAPS_APPLICATION_H__


namespace Gempa {
namespace CAPS {


class Application {
	public:
		Application(int argc, char **argv);
		virtual ~Application() {}


	public:
		/**
		 * Exit the application and set the returnCode.
		 * @param returnCode The value returned from exec()
		 */
		virtual void exit(int returnCode);

		/**
		 * @brief Conventient function to simplify usage
		 * @return The value returned from exec()
		 */
		int operator()() { return exec(); }

		/**
		 * @brief In case of an interrupt this method can be used to
		 *        forward the signal to the internal signal handling.
		 * @param signal
		 */
		static void Interrupt(int signal);


	protected:
		/**
		 * @brief Cleanup method called before exec() returns.
		 */
		virtual void done();

		/**
		 * Execs the mainloop and waits until exit() is called
		 * or a appropriate signal has been fired (e.g. SIGTERM).
		 * @return The value that was set with to exit()
		 */
		int exec();
		/**
		 * @brief This method can be used to implement custom
		 *        signal handling.
		 * @param signal The emitted signal
		 */
		virtual void handleInterrupt(int signal);

		/**
		 * @brief Initialization method. This method calls the initCommandLine
		 *        initConfiguration and validateParameters function
		 */
		virtual bool init();
		/**
		 * @brief Handles commandline arguments
		*/
		virtual bool initCommandLine();
		/**
		 * @brief Handles configuration files
		 */
		virtual bool initConfiguration();

		/**
		 * @brief This method must be implemented in the inherited class to
		 *        execute the plugin specific code.
		 */
		virtual bool run();
		/**
		 * @brief This method can be used to verify custom configuration or
			      commandline parameters
		 */
		virtual bool validateParameters();


	protected:
		bool   _exitRequested;
		int    _argc;
		char **_argv;


	private:
		static Application *_app;
		int    _returnCode;
};


}
}


#endif
