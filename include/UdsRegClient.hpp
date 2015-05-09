#ifndef AARDVARK_PLUGIN_INCLUDE_UDSREGCLIENT_HPP_
#define AARDVARK_PLUGIN_INCLUDE_UDSREGCLIENT_HPP_

#include "errno.h"
#include <sys/un.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cstdio>
#include <list>
#include <pthread.h>
#include "signal.h"
#include <string>

#include <UdsRegWorker.hpp>
#include "UdsRegClient.hpp"
#include "AardvarkPlugin.hpp"
#include "JsonRPC.hpp"
#include "Plugin.hpp"
#include "Error.hpp"

using namespace std;


class UdsRegClient{


	public:
		UdsRegClient(const char* pluginName, int pluginNumber,const char* regPath, int size, const char* comPath);
		virtual ~UdsRegClient();


		void connectToRSD();
		void sendAnnounceMsg();
		void unregisterFromRSD();
		void processRegistration(string* msg);

		bool isDeletable()
		{
			if(regWorker != NULL)
				return regWorker->isDeletable();
			else
				return false;
		}

	private:

		static struct sockaddr_un address;
		static socklen_t addrlen;

		int optionflag;
		int connection_socket;

		UdsRegWorker* regWorker;
		Plugin* plugin;
		JsonRPC* json;
		const char* regPath;

		const char* error;
		Value* currentMsgId;

		enum REG_STATE{NOT_ACTIVE, ANNOUNCED, REGISTERED, ACTIVE, BROKEN};
		unsigned int state;


		bool handleAnnounceACKMsg(string* msg);
		const char* createRegisterMsg();
		bool handleRegisterACKMsg(string* msg);
		const char* createPluginActiveMsg();
};

#endif /* AARDVARK_PLUGIN_INCLUDE_UDSREGCLIENT_HPP_ */
