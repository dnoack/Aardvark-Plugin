/*
 * Uds.cpp
 *
 *  Created on: 04.02.2015
 *      Author: dnoack
 */

#include <UdsServer.hpp>
#include "JsonRPC.hpp"
#include "UdsComWorker.hpp"


#define EXPECTED_NUM_OF_DEVICES 1

//static symbols
int UdsServer::connection_socket;

vector<UdsComWorker*> UdsServer::workerList;
pthread_mutex_t UdsServer::wLmutex;

struct sockaddr_un UdsServer::address;
socklen_t UdsServer::addrlen;



UdsServer::UdsServer( const char* udsFile, int nameSize)
{

	optionflag = 1;
	connection_socket = socket(AF_UNIX, SOCK_STREAM, 0);
	address.sun_family = AF_UNIX;
	strncpy(address.sun_path, udsFile, nameSize);
	addrlen = sizeof(address);

	pthread_mutex_init(&wLmutex, NULL);

	unlink(udsFile);
	setsockopt(connection_socket, SOL_SOCKET, SO_REUSEADDR, &optionflag, sizeof(optionflag));
	bind(connection_socket, (struct sockaddr*)&address, addrlen);


}



UdsServer::~UdsServer()
{
	close(connection_socket);
	workerList.erase(workerList.begin(), workerList.end());
	pthread_mutex_destroy(&wLmutex);
}



int UdsServer::call()
{
	connect(connection_socket, (struct sockaddr*) &address, addrlen);
	return 0;
}




void* UdsServer::uds_accept(void* param)
{
	int new_socket = 0;
	UdsComWorker* worker = NULL;
	listen(connection_socket, 5);
	bool accept_thread_active = true;


	printf("Accepter created\n");
	while(accept_thread_active)
	{
		new_socket = accept(connection_socket, (struct sockaddr*)&address, &addrlen);
		if(new_socket > 0)
		{
			worker = new UdsComWorker(new_socket);
			pushWorkerList(worker);
		}

	}
	return 0;
}




void UdsServer::pushWorkerList(UdsComWorker* newWorker)
{
	pthread_mutex_lock(&wLmutex);
		workerList.push_back(newWorker);
	pthread_mutex_unlock(&wLmutex);
}



void UdsServer::checkForDeletableWorker()
{

	pthread_mutex_lock(&wLmutex);
	for(unsigned int i = 0; i < workerList.size() ; i++)
	{
		if(workerList[i]->isDeletable())
		{
			delete workerList[i];
			workerList.erase(workerList.begin()+i);
		}
	}
	pthread_mutex_unlock(&wLmutex);
}





void UdsServer::startCom()
{
	pthread_t accepter;
	pthread_create(&accepter, NULL, uds_accept, NULL);
}



