#include <UdsServer.hpp>
#include "JsonRPC.hpp"
#include "UdsComWorker.hpp"


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

	printf("Accepter created\n");
	while(true)
	{
		ready = true;
		new_socket = accept(connection_socket, (struct sockaddr*)&address, &addrlen);
		if(new_socket > 0)
		{
			worker = new UdsComWorker(new_socket);
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
	printf("New UdsWorker : %d\n", workerList.size());
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
			delete *i;
			i = workerList.erase(i);
			printf("UdsWorker deleted from list, %d left.\n", workerList.size());
		}
		else
			++i;
	}
	pthread_mutex_unlock(&wLmutex);
}
