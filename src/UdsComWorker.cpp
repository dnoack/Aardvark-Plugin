/*
 * UdsComWorker.cpp
 *
 *  Created on: 09.02.2015
 *      Author: dnoack
 */


#include "UdsComWorker.hpp"
#include "UdsServer.hpp"
#include "errno.h"



UdsComWorker::UdsComWorker(int socket)
{
	memset(receiveBuffer, '\0', BUFFER_SIZE);
	this->listen_thread_active = false;
	this->worker_thread_active = false;
	this->recvSize = 0;
	this->lthread = 0;
	this->request = 0;
	this->response = 0;
	this->currentSocket = socket;
	this->paard = new AardvarkCareTaker();

	StartWorkerThread(currentSocket);
}



UdsComWorker::~UdsComWorker()
{
	delete paard;
	worker_thread_active = false;
	listen_thread_active = false;
	pthread_kill(lthread, SIGUSR2);

	WaitForWorkerThreadToExit();
}




void UdsComWorker::thread_work(int socket)
{

	worker_thread_active = true;

	//start the listenerthread and remember the theadId of it
	lthread = StartListenerThread(pthread_self(), currentSocket, receiveBuffer);


	while(worker_thread_active)
	{
		request = NULL;
		//wait for signals from listenerthread

		sigwait(&sigmask, &currentSig);
		switch(currentSig)
		{
			case SIGUSR1:
				while(getReceiveQueueSize() > 0)
				{
					//sigusr1 = there is data for work e.g. parsing json rpc
					//printf("We received something and the worker got a signal.\n");

					//1 get data from queue
					request = receiveQueue.back();
					printf("Received: %s\n", request->c_str());

					try
					{
						//TODO: BUG wenn dieser teil drin ist, passiert was undefiniertes / programm endet mit 5 threads.
						response = paard->processMsg(request);
					}
					catch(PluginError &e)
					{
						response = new string(e.get());
					}

					send(currentSocket, response->c_str(), response->size(), 0);
					//send(currentSocket, "OK", 2, 0);
					//3 remove data from queue
					popReceiveQueue();

					delete response;

				}
				break;

			case SIGUSR2:
				printf("UdsComWorker: SIGUSR2\n");

				break;
			case SIGPIPE:
				printf("UdsComWorker: SIGPIPE\n");
				break;
			default:
				printf("UdsComWorker: unkown signal \n");
				break;
		}

	}
	close(currentSocket);
	WaitForListenerThreadToExit();
	//destroy this UdsWorker and delete it from workerList in Uds::Server
	UdsServer::editWorkerList(this, DELETE_WORKER);
	printf("UdsComWorker: Worker Thread beendet.\n");

}



void UdsComWorker::thread_listen(pthread_t parent_th, int socket, char* workerBuffer)
{

	listen_thread_active = true;

	while(listen_thread_active)
	{
		memset(receiveBuffer, '\0', BUFFER_SIZE);


		recvSize = recv( socket , receiveBuffer, BUFFER_SIZE, 0);
		if(recvSize > 0)
		{
			//add received data in buffer to queue
			pushReceiveQueue(new string(receiveBuffer, recvSize));


			pthread_kill(parent_th, SIGUSR1);

		}
		//no data, either udsComClient or plugin invoked a shutdown of this UdsComWorker
		else
		{
			//udsComClient invoked shutdown
			if(errno == EINTR)
			{
				worker_thread_active = false;
				listen_thread_active = false;
				pthread_kill(parent_th, SIGUSR2);
			}
			//plugin invoked shutdown
			else
			{
				worker_thread_active = false;
				listen_thread_active = false;
				pthread_kill(parent_th, SIGUSR2);
			}
			listenerDown = true;
		}

	}
	if(!listenerDown)
	{
		worker_thread_active = false;
		listen_thread_active = false;
		pthread_kill(parent_th, SIGUSR2);
	}
	printf("UdsComWorker: Listener beendet.\n");
}



