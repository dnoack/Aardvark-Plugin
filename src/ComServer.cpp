#include <ComServer.hpp>
#include "JsonRPC.hpp"



ComServer::ComServer( const char* udsFile, int nameSize, int pluginNumber)
{
	optionflag = 1;
	this->pluginNumber = pluginNumber;
	connection_socket = socket(AF_UNIX, SOCK_STREAM, 0);
	address.sun_family = AF_UNIX;
	strncpy(address.sun_path, udsFile, nameSize);
	addrlen = sizeof(address);
	infoIn.logLevel = LOG_INPUT;
	infoIn.logName = "IPC IN:";
	infoOut.logLevel = LOG_OUTPUT;
	infoOut.logName = "IPC OUT:";
	info.logLevel = LOG_INFO;
	info.logName = "ComPoint for RPC:";

	pthread_mutex_init(&wLmutex, NULL);

	unlink(udsFile);
	setsockopt(connection_socket, SOL_SOCKET, SO_REUSEADDR, &optionflag, sizeof(optionflag));
	bind(connection_socket, (struct sockaddr*)&address, addrlen);


	StartAcceptThread();

	if(wait_for_accepter_up() != 0)
		throw Error("Creation of Listener/worker threads failed.");

}


ComServer::~ComServer()
{
	pthread_t accepter = getAccepter();
	if(accepter != 0)
		pthread_cancel(accepter);

	WaitForAcceptThreadToExit();

	deleteComPointList();
	close(connection_socket);
	pthread_mutex_destroy(&wLmutex);
}


void ComServer::thread_accept()
{
	int new_socket = 0;
	ComPoint* comPoint = NULL;
	AardvarkCareTaker* aardC = NULL;
	listen(connection_socket, MAX_CLIENTS);

	//dyn_print("Accepter created\n");
	while(true)
	{
		new_socket = accept(connection_socket, (struct sockaddr*)&address, &addrlen);
		if(new_socket > 0)
		{
			aardC = new AardvarkCareTaker();
			comPoint = new ComPoint(new_socket, aardC, pluginNumber);
			comPoint->configureLogInfo(&infoIn, &infoOut, &info);
			//dyn_print("Uds---> sNew UdsWorker with socket: %d \n", new_socket);
			pushComPointList(comPoint);
		}
	}
}


void ComServer::deleteComPointList()
{
	list<ComPoint*>::iterator worker= comPointList.begin();

	while(worker != comPointList.end())
	{
		delete *worker;
		worker = comPointList.erase(worker);
	}
}


void ComServer::pushComPointList(ComPoint* comPoint)
{
	pthread_mutex_lock(&wLmutex);
	comPointList.push_back(comPoint);
	//dyn_print("Uds---> Number of UdsWorker: %d \n", comPointList.size());
	pthread_mutex_unlock(&wLmutex);
}


void ComServer::checkForDeletableWorker()
{
	pthread_mutex_lock(&wLmutex);

	list<ComPoint*>::iterator i = comPointList.begin();
	while(i != comPointList.end())
	{
		if((*i)->isDeletable())
		{
			//dyn_print("Uds---> Deleting UdsWorker. %d still left.\n", comPointList.size()-1);
			delete (*i)->getProcessInterface(); //delete AardvarkCareTarker
			//TODO: find a better way, maybe a more generic way
			delete *i;
			i = comPointList.erase(i);
		}
		else
			++i;
	}
	pthread_mutex_unlock(&wLmutex);
}
