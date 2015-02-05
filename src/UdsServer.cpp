/*
 * Uds.cpp
 *
 *  Created on: 04.02.2015
 *      Author: dnoack
 */

#include <UdsServer.hpp>
#include "JsonRPC.hpp"



UdsServer::UdsServer(int mode, const char* udsFile, int nameSize)
{

	json = new Plugin::JsonRPC();
	connection_socket = socket(AF_UNIX, SOCK_STREAM, 0);
	address.sun_family = AF_UNIX;
	strncpy(address.sun_path, udsFile, nameSize);
	addrlen = sizeof(address);

	switch(mode)
	{
		case SERVER_MODE:
			unlink(udsFile);
			setsockopt(connection_socket, SOL_SOCKET, SO_REUSEADDR, &optionflag, sizeof(optionflag));
			bind(connection_socket, (struct sockaddr*)&address, addrlen);
			break;
		default:
			break;
	}
}



UdsServer::~UdsServer()
{
	close(connection_socket);
	delete json;
}



int UdsServer::call()
{
	connect(connection_socket, (struct sockaddr*) &address, addrlen);
}



int UdsServer::ud_send(char* msg, int msgSize)
{
	send(connection_socket, msg, msgSize, 0);
}



int UdsServer::ud_receive(char* buffer, int bufferSize)
{
	int recvSize = 0;
	while(recvSize == 0)
		recvSize = recv(connection_socket, buffer, bufferSize, 0);

	return recvSize;
}



void UdsServer::thread_listen(pthread_t parent_th, int socket)
{
	char buffer[BUFFER_SIZE];
	int recvSize = 0;

	memset(buffer, '\0', BUFFER_SIZE);

	while(listen_thread_active)
	{

		recvSize = recv( socket , buffer, BUFFER_SIZE, 0);

	}
	pthread_kill(parent_th, SIGPOLL);

}



void UdsServer::thread_accept()
{
	int new_socket = 0;
	pthread_t th_id;
	accept_thread_active = true;
	listen(connection_socket, 5);

	while(accept_thread_active)
	{
		new_socket = accept(connection_socket, (struct sockaddr*)&address, &addrlen);
		if(new_socket >= 0)
		{
			th_id = StartWorkerThread(new_socket);
			workerList.push_back(th_id);
			printf("Client verbunden.\n");
		}
	}
}


void UdsServer::thread_work(int socket)
{
	sleep(3);
	printf("Client verbunden, worker thread %d",  pthread_self());
}


void UdsServer::startCom()
{
	StartAcceptThread();
}



/*
void UdsServer::start()
{
	int recvSize = 0;
	char buffer[BUFFER_SIZE];
	string* identity;
	string* inputValue;
	string* tmp;
	char* returnValue;

	memset(buffer, '\0', BUFFER_SIZE);

	listen(connection_socket, 5);

	while(true)
	{
		working_socket = accept( connection_socket, (struct sockaddr*)&address, &addrlen);
		active = true;

		while(active)
		{
			recvSize = recv(working_socket, buffer, BUFFER_SIZE, 0);
			if(recvSize > 0)
			{


					buffer[recvSize] = '\0';
					printf("Received: %s \n", buffer);


					//send(working_socket, "Hallo Client", 12,0);
					inputValue = new string(buffer);
					identity = new string("testclient");

					if(inputValue->compare("close") != 0)
					{
						returnValue = json->handle(inputValue, identity);
						send(working_socket, returnValue, 32,0);
					}
					else
						active = false;

					memset(buffer, '\0', BUFFER_SIZE);
					delete inputValue;
					delete identity;

			}
		}
		close(working_socket);
	}



}*/



