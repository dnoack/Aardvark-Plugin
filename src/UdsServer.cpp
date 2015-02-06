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

	pthread_mutex_init(&wLmutex, NULL);
	configSignals();

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
	pthread_mutex_destroy(&wLmutex);
	close(connection_socket);
	delete json;
}



int UdsServer::call()
{
	connect(connection_socket, (struct sockaddr*) &address, addrlen);
	return 0;
}





void UdsServer::thread_listen(pthread_t parent_th, int socket, char* workerBuffer, bool* workerBusy)
{
	char buffer[BUFFER_SIZE];
	int recvSize = 0;
	bool listen_thread_active = true;
	*workerBusy = false;
	memset(buffer, '\0', BUFFER_SIZE);


	while(listen_thread_active)
	{

		recvSize = recv( socket , buffer, BUFFER_SIZE, 0);
		if(recvSize > 0)
		{
			if(!*workerBusy)
			{
				*workerBusy = true;
				//copy buffer / add copy off buffer to a queue
				strncpy(workerBuffer, buffer, BUFFER_SIZE);
				//signal the worker
				pthread_kill(parent_th, SIGUSR1);
				memset(buffer, '\0', BUFFER_SIZE);
			}
			else
			{
				//queue data
				printf("\n#####Nachricht verworfen, zuviele Nachrichten.####\n");
			}
		}
		else
			listen_thread_active = false;

	}
	printf("Listener beendet.\n");
	pthread_kill(parent_th, SIGPOLL);

}



void UdsServer::thread_accept()
{
	int new_socket = 0;
	pthread_t th_id;
	bool accept_thread_active = true;
	listen(connection_socket, 5);

	while(accept_thread_active)
	{
		new_socket = accept(connection_socket, (struct sockaddr*)&address, &addrlen);
		if(new_socket >= 0)
		{
			th_id = StartWorkerThread(new_socket);
			editWorkerList(th_id, ADD_WORKER);
			printf("Client verbunden.\n");
		}

	}
}


void UdsServer::thread_work(int socket)
{
	Plugin::JsonRPC json;
	pthread_t lthread = 0;
	char buffer[BUFFER_SIZE];
	char* bufferOut;
	string* jsonInput;
	string* identity = new string("fake");
	string* jsonReturn;
	int currentsocket = socket;
	bool worker_thread_active = true;
	int currentSig = 0;
	bool workerBusy = false;

	memset(buffer, '\0', BUFFER_SIZE);

	//start the listenerthread and remember the theadId of it
	lthread = StartListenerThread(pthread_self(), currentsocket, buffer, &workerBusy);


	while(worker_thread_active)
	{
		//wait for signals from listenerthread

		sigwait(&sigmask, &currentSig);
		switch(currentSig)
		{
			case SIGUSR1:

				//sigusr1 = there is data for work e.g. parsing json rpc
				printf("We received something and the worker got a signal.\n");
				jsonInput = new string(buffer, BUFFER_SIZE);
				bufferOut = json.handle(jsonInput, identity);
				jsonReturn = new string(bufferOut);
				send(currentsocket, jsonReturn->c_str(), jsonReturn->size(), 0);

				delete jsonInput;
				memset(buffer, '\0', BUFFER_SIZE);
				delete jsonReturn;
				break;

			case SIGUSR2:
				//sigusr2 = time to exit
				worker_thread_active = false;
				break;

			case SIGPIPE:
				break;
			default:
				worker_thread_active = false;
				break;
		}
		workerBusy = false;
	}
	editWorkerList(pthread_self(), DELETE_WORKER);
	close(currentsocket);
	printf("Worker Thread beendet.\n");
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



