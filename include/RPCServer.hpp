/*
 * RPCServer.h
 *
 *  Created on: 19.01.2015
 *      Author: dnoack
 */

#ifndef RPCSERVER_H_
#define RPCSERVER_H_

#include <JsonRPC.hpp>
#include "stddef.h"
#include "string.h"
#include <list>
#include "zmq.h"


#define RECEIVE_BUFFER_SIZE 1024
#define DELIMITER ""


using namespace std;


namespace Plugin {


/**
 * \brief RPC-Server Impelementation for VH-Plugins
 *
 */
class RPC_Server {

	public:

		RPC_Server()
		{
			context = NULL;
			socketToRCD = NULL;
			active = false;
			json = NULL;
			peerIdentity = NULL;
		}

		~RPC_Server();

		void init();
		void listen();

	private:

		void* context;
		void* socketToRCD;
		bool active;

		JsonRPC* json;
		list<string*> identityRoute;

		char receiveBuffer[RECEIVE_BUFFER_SIZE];
		string* peerIdentity;


		string* receive(void* socket);
		int send (void *socket, char *string);

};


} /* namespace Plugin */

#endif /* RPCSERVER_H_ */
