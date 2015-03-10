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
	if(!deletable)
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

					request = receiveQueue.back(); // ?bug with linked list
					printf("Received: %s\n", request->c_str());

					try
					{
						response = paard->processMsg(request);
					}
					catch(PluginError &e)
					{
						response = new string(e.get());
					}
					catch(...)
					{
						printf("Unkown exception.\n");
					}

					send(currentSocket, response->c_str(), response->size(), 0);

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
			case SIGPOLL:
				printf("UdsComWorker: SIGPOLL\n");
				break;
			default:
				printf("UdsComWorker: unkown signal \n");
				break;
		}

	}
	close(currentSocket);
	WaitForListenerThreadToExit();
	//mark this whole worker object for a delete
	deletable = true;
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
			//RSD invoked shutdown
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
				pthread_kill(parent_th, SIGPOLL);
				printf("Receivsize = 0\n");
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



