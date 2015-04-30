#include <UdsServer.hpp>
#include "JsonRPC.hpp"
#include "UdsComWorker.hpp"
#include "Utils.h"


int UdsServer::connection_socket;
list<UdsComWorker*> UdsServer::workerList;
pthread_mutex_t UdsServer::wLmutex;
struct sockaddr_un UdsServer::address;
socklen_t UdsServer::addrlen;
bool UdsServer::ready;


UdsServer::UdsServer( const char* udsFile, int nameSize)
{
	optionflag = 1;
	ready = false;
	connection_socket = socket(AF_UNIX, SOCK_STREAM, 0);
	address.sun_family = AF_UNIX;
	strncpy(address.sun_path, udsFile, nameSize);
	addrlen = sizeof(address);

	pthread_mutex_init(&wLmutex, NULL);

	unlink(udsFile);
	setsockopt(connection_socket, SOL_SOCKET, SO_REUSEADDR, &optionflag, sizeof(optionflag));
	bind(connection_socket, (struct sockaddr*)&address, addrlen);

	pthread_create(&accepter, NULL, uds_accept, NULL);
	while(!isReady()){}
}


UdsServer::~UdsServer()
{
	pthread_cancel(accepter);
	pthread_join(accepter, NULL);
	deleteWorkerList();
	pthread_mutex_destroy(&wLmutex);
}


void* UdsServer::uds_accept(void* param)
{
	int new_socket = 0;
	UdsComWorker* worker = NULL;
	listen(connection_socket, MAX_CLIENTS);

	dyn_print("Accepter created\n");
	while(true)
	{
		ready = true;
		new_socket = accept(connection_socket, (struct sockaddr*)&address, &addrlen);
		if(new_socket > 0)
		{
			worker = new UdsComWorker(new_socket);
			dyn_print("Uds---> sNew UdsWorker with socket: %d \n", new_socket);
			pushWorkerList(worker);
		}
	}
	return 0;
}


void UdsServer::deleteWorkerList()
{
	list<UdsComWorker*>::iterator worker= workerList.begin();

	while(worker != workerList.end())
	{
		delete *worker;
		worker = workerList.erase(worker);
	}
}


void UdsServer::pushWorkerList(UdsComWorker* newWorker)
{
	pthread_mutex_lock(&wLmutex);
	workerList.push_back(newWorker);
	dyn_print("Uds---> Number of UdsWorker: %d \n", workerList.size());
	pthread_mutex_unlock(&wLmutex);
}


void UdsServer::checkForDeletableWorker()
{
	pthread_mutex_lock(&wLmutex);

	list<UdsComWorker*>::iterator i = workerList.begin();
	while(i != workerList.end())
	{
		if((*i)->isDeletable())
		{
			dyn_print("Uds---> Deleting UdsWorker with socket, %d. %d still left.\n", (*i)->getSocket(), workerList.size()-1);
			delete *i;
			i = workerList.erase(i);
		}
		else
			++i;
	}
	pthread_mutex_unlock(&wLmutex);
}
