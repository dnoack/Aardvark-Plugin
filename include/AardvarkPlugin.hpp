#ifndef INCLUDE_AARDVARKPLUGIN_H_
#define INCLUDE_AARDVARKPLUGIN_H_

#include "PluginInterface.hpp"

/*! Path to uds file for registring the plugin to RSD.*/
#define REG_PATH "/tmp/RsdRegister.uds"
/*! Path to uds file for communication (except registring), this has to be a unique path for every plugin.*/
#define COM_PATH "/tmp/AardvarkPlugin.uds"
/*! Plugin name, every rpc method for this plugin has to begin with this name, the name has to be unique.*/
#define PLUGIN_NAME "Aardvark"
/*! Unique plugin number.*/
#define PLUGIN_NUMBER 1

/**
 * \class AardvarkPlugin
 * \brief Makes it possible to call driver functions from the shared library of the Totalphase Aardvark from Remote-Server-Daemon via
 * JSON RPC.
 * The AardvarkPlugin is the highest instance and the initial class for the Totalphase Aardvark plugin of the Remote-Server-Daemon.
 * It waits for incoming IPC connections over a defined unix domain socket and processes all incoming messages.
 * A valid request for AardvarkPlugin has to be in JSON RPC 2.0 format and the method member has to be like "Aardvark.functionname".
 * All supported functions are registered through the RemoteAardvark class. For every connection there will be a separate instance
 * of AardvarkCareTaker which manages a static list of Aardvark devcies, represented as instances of RemoteAardvark. These instances
 * are connected to the ConnectionContext id of the Remote-Server-Daemon. As long as a user doesn't close the connection or explicitly
 * use the "Aardvark.aa_close", the corresponding device will be locked for all other users.
 */
class AardvarkPlugin : public PluginInterface{

	public:

		/**
		 * Base-Constructor.
		 * Creates and initializes AardvarkPlugin and makes it ready to start working.
		 * \param pluginInfo Containing necessary information for configuring the plugin.
		 */
		AardvarkPlugin(PluginInfo* pluginInfo);

		/** Base-Destructor.*/
		~AardvarkPlugin();

		/**
		 * Implementation of the pure virtual function of PluginInterface.
		 * Listens to the configurired Unix domain socket and accepts incoming connection.
		 * A incoming connection will create a new instance of AardvarkCareTaker + ComPoint.
		 * AardvarkCarTaker implements the processInterface and will be connected to the created ComPoint.
		 * The comPoint will be configured (logging, etc.) and added to the list of comPoints.
		 * \note Because PluginInterface inherits from AcceptThread, this function will run in a separate thread.
		 */
		void thread_accept();
};

#endif /* INCLUDE_AARDVARKPLUGIN_H_ */
