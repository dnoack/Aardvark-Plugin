
#ifndef INCLUDE_AARDVARKPLUGIN_HPP_
#define INCLUDE_AARDVARKPLUGIN_HPP_

/*! Path to uds file for registring the plugin to RSD.*/
#define REG_PATH "/tmp/RsdRegister.uds"
/*! Path to uds file for communication (except registring), this has to be a unique path for every plugin.*/
#define COM_PATH "/tmp/AardvarkPlugin.uds"
/*! Plugin name, every rpc method for this plugin has to begin with this name, the name has to be unique.*/
#define PLUGIN_NAME "Aardvark"
/*! Unique plugin number.*/
#define PLUGIN_NUMBER 1
/*! Wait time in seconds for main poll loop.*/
#define WAIT_TIME 3

#include <ComServer.hpp>
#include "RegClient.hpp"
#include "RemoteAardvark.hpp"
#include "AardvarkCareTaker.hpp"
#include "LogUnit.hpp"

class RegClient;

/**
 * \class AardvarkPlugin
 * Main class of the plugin to connect a Totalphase Aardvark to RSD.
 * Creating a instance of this class will create a client for registring the plugin
 * and a communication part (UdsServer) for receiving json rpc requests and sending
 * json rpc responses. Both use unix domain sockets for communication with RSD. The
 * defines REG_PATH, COM_PATH, PLUGIN_NAME and PLUGIN_NUMBER are very important (see
 * their description) for the connection.
 */
class AardvarkPlugin : LogUnit{

	public:

		/**
		 * Constructor.
		 */
		AardvarkPlugin();

		/**
		 * Destructor.
		 */
		virtual ~AardvarkPlugin();

		/**
		 * Start the plugin. This means, it tries to register to RSD and after that executing a loop
		 * where it constantly searches for deletable worker.
		 */
		void start();


	private:

		/*! Handles incomming json rpc calls from RSD.*/
		ComServer* comServer;
		/*! Handles the registry process to RSD.*/
		RegClient* regClient;

		sigset_t sigmask;
		sigset_t origmask;

		/*! As long as this flag is true, the server will run his mainloop in start().*/
		bool pluginActive;
};

#endif /* INCLUDE_AARDVARKPLUGIN_HPP_ */
