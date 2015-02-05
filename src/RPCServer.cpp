/*
 * RPCServer.cpp
 *
 *  Created on: 19.01.2015
 *      Author: dnoack
 */

#include <RPCServer.hpp>
#include "zhelpers.h"


using namespace Plugin;


RPC_Server::~RPC_Server()
{
	delete(json);
	zmq_close(socketToRCD);
	zmq_ctx_destroy(context);
}


void RPC_Server::init()
{
	int option = 0;


	json = new JsonRPC();
	context = zmq_ctx_new();
	socketToRCD = zmq_socket(context, ZMQ_ROUTER);
	//zmq_setsockopt(socketToRCD, ZMQ_ROUTER_MANDATORY,&option,4);
	zmq_bind(socketToRCD, "ipc:///tmp/feeds/aardvark1");


}


void RPC_Server::listen()
{
	string* receivMsg = NULL;
	char* responseMsg = NULL;
	bool delimiterFound = false;
	active = true;
	string* currentIdentity;
	int routeSize;


	while(active)
	{
		printf("RPC_Server is listening...\n");
		//s_dump(socketToRCD);

		//analyse the IDENTITY FRAMES
		//if we found a valid json rpc, use the very inner IDENTITY to lock some hardware for a the user with this identity
		//we also need to save all identitys (in the right order) to find the way back to the client !
		while(!delimiterFound)
		{
			receivMsg = receive(socketToRCD);
			printf("RPC_Server received: %s \n", receivMsg->c_str());
			if(receivMsg->compare(DELIMITER) == 0)
			{
				delimiterFound = true;
				delete(receivMsg);//delete delimeter
			}
			else
			{
				identityRoute.push_front(receivMsg);
			}
		}

		//next msg part has to be json !
		receivMsg = receive(socketToRCD);
		peerIdentity = identityRoute.front();
		responseMsg = json->handle(receivMsg, peerIdentity);

		if(responseMsg != NULL) //if NULL then it is NO json rpc !
		{
			printf("RPCServer responseMsg: %s\n", responseMsg);

			//for all route identitys
			routeSize = identityRoute.size();
			for(int i = 0; i < routeSize ; i++)
			{
				currentIdentity = identityRoute.back();
				zmq_send(socketToRCD, currentIdentity->c_str(), currentIdentity->size(), ZMQ_SNDMORE);
				identityRoute.pop_back();
			}

			zmq_send (socketToRCD, "", 0, ZMQ_SNDMORE);
			zmq_send (socketToRCD, responseMsg, strlen(responseMsg), 0);
		}

		delete receivMsg;
		receivMsg = NULL;
		delimiterFound = false;
	}
}



string* RPC_Server::receive(void *socket)
{
	int size = 0;


	memset(receiveBuffer, 0, RECEIVE_BUFFER_SIZE);
	size = zmq_recv (socket, receiveBuffer, RECEIVE_BUFFER_SIZE, 0);

    if (size == -1)
        return NULL; //TODO: error handling

    if (size > RECEIVE_BUFFER_SIZE)
        size = RECEIVE_BUFFER_SIZE;

    receiveBuffer[RECEIVE_BUFFER_SIZE] = 0;
    return new string(receiveBuffer);
}


int RPC_Server::send(void *socket, char *string)
{
    int size = zmq_send (socket, string, strlen (string), 0);
    return size;
}




int main(void)
{
	Plugin::RPC_Server* server = new Plugin::RPC_Server();
	server->init();
	server->listen();

	return 0;
}

