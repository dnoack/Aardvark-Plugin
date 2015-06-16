#ifndef _INCLUDE_UDSREGCLIENT_HPP_
#define _INCLUDE_UDSREGCLIENT_HPP_

#include "errno.h"
#include <sys/un.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cstdio>
#include <list>
#include <pthread.h>
#include <RPCInterface.hpp>
#include "signal.h"
#include <string>

#include "ComPoint.hpp"
#include "AardvarkPlugin.hpp"
#include "JsonRPC.hpp"
#include "RPCMsg.hpp"
#include "OutgoingMsg.hpp"
#include "Plugin.hpp"
#include "Error.hpp"

using namespace std;



/**
 * \class UdsRegClient
 * UdsRegClient handles the registration of the corresponding plugin to RSD.
 * It uses a known unix domain socket file for registering nad tells the RSD
 * the name, id ,unix domain socket file for communication and all known rpc function
 * of the plugin. If something goes wrong during the registration, the connection_socket will be
 * closed, which results in shutting down the whole plugin.
 */
class RegClient : public ProcessInterface{


	public:

		/**
		 * Constructor.
		 * \param plugin
		 * \param rpcClass
		 * \param regPath Path to unix domain socket file for registring a plugin to RSD.
		 */
		RegClient(Plugin* plugin, list<string*>* functionList, const char* regPath);

		/**
		 * Destructor.
		 */
		virtual ~RegClient();


		/**
		 * Trys to connect to RSD over unix domain socket. On success a new instance of
		 * UdsRegWorker will be created, which can receive and send data to RSD.
		 * \throws Instance of Error exception if it fails to connect to RSD.
		 */
		void connectToRSD();

		/**
		 * This sends the first message to RSD, which is part of the registrationprocess.
		 * It announces the name, id and unix domain socket file of the plugin to RSD.
		 * \throws Instance of Error if the message could not be send to RSD.
		 */
		void sendAnnounceMsg();

		/**
		 * Not implemented, yet.
		 */
		void unregisterFromRSD();

		/**
		 * Incomming message from UdsRegWorker during the registration process will be send
		 * to this method. It will analyze the message and if they are correct, it will change
		 * the state of the registration process. If something goes wrong, the state will be set
		 * to BROKEn and the connection_socket will be closed.
		 */
		OutgoingMsg* process(IncomingMsg* input);

		/**
		 * Checks the underlying UdsRegWorker instance if it is deletable.
		 * \return The value of the deletable flag of the underlying UdsRegWorker, if there is none it returns false.
		 */
		bool isDeletable()
		{
			if(comPoint != NULL)
				return comPoint->isDeletable();
			else
				return false;
		}

	private:

		list<string*>* functionList;
		/*! Unix domain socket address struct.*/
		static struct sockaddr_un address;
		/*! Length of address.*/
		static socklen_t addrlen;

		/*! Optionflag variable for connection_socket.*/
		int optionflag;
		/*! Socket fd for unix domain socket for registering.*/
		int connection_socket;


		/*! Contains information about the corresponding plugin.*/
		Plugin* plugin;
		/*! Instance of json rpc parser, using rapidjson.*/
		JsonRPC* json;

		Document* globalDom;

		/*! Path to unix domain socket file for registering the plugin to RSD.*/
		const char* regPath;
		/*! Contains the json rpc message id of the last received message.*/
		Value* currentMsgId;

		/*!
		 * NOT_ACTIVE = A connection via UdsRegServer was accepted. The plugin is not known by RSD nor by the instance of Registration.
		 * ANNOUNCED = A announce message was received, containing the name, id and path to unix domain socket file. Registration knows the plugin, RSD not.
		 * REGISTERED = A register message was received, containing a list of functionnames of the plugin. Registration knows the plugin and it's functions, RSD not.
		 * ACTIVE = A pluginActive notification was received. Registration knows pluginname, plugin kowns plugin nad all of it's functions.
		 * BROKEN = should not happen, yet.
		 */
		enum REG_STATE{NOT_ACTIVE, ANNOUNCED, REGISTERED, ACTIVE, BROKEN};
		/*! Current state of the registration process.*/
		unsigned int state;


		/**
		 * Handles the announceACK message, which we await from RSD after we send an announce message to RSD.
		 * \return If everything is ok with the announceACK message it returns true, otherwhise a exception is thrown.
		 * \throws Error exception.
		 */
		bool handleAnnounceACKMsg();

		/**
		 * Creates a the register message whithin the register process, which is end to RSD after receiving a announce ACk
		 * message. The register message contains all known functions of the corresponding plugin.
		 * \return A valid json rpc message which contains a the register message.
		 */
		OutgoingMsg* createRegisterMsg(IncomingMsg* input);

		/**
		 * Handles the registerACK message, which we await from RSD after we send a register message to RSD:
		  * \return If everything is ok with the registerACK message it returns true, otherwhise a exception is thrown.
		 * \throws Error exception.
		 */
		bool handleRegisterACKMsg();

		/**
		 * Creates a pluginActive message, which completes the registration process and signals RSD that this plugin
		 * is ready to work.
		 */
		OutgoingMsg* createPluginActiveMsg(IncomingMsg* input);


		void deleteFunctionList();
};

#endif /* _INCLUDE_UDSREGCLIENT_HPP_ */
