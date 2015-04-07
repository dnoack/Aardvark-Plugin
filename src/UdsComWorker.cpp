/*
 * UdsComWorker.cpp
 *
 *  Created on: 09.02.2015
 *      Author: dnoack
 */


#include "UdsComWorker.hpp"
#include "UdsServer.hpp"
#include <sys/select.h>
#include "errno.h"



UdsComWorker::UdsComWorker(int socket)
{
	this->request = 0;
	this->response = 0;
	this->currentSocket = socket;
	this->paard = new AardvarkCareTaker(this);

	StartWorkerThread(currentSocket);
}



UdsComWorker::~UdsComWorker()
{

	pthread_cancel(getListener());
	pthread_cancel(getWorker());

	WaitForListenerThreadToExit();
	WaitForWorkerThreadToExit();

	delete paard;
	deleteReceiveQueue();
}


int UdsComWorker::uds_send(string* data)
{
	return send(currentSocket, data->c_str(), data->size(), 0);
}

int UdsComWorker::uds_send(const char* data)
{
	return send(currentSocket, data, strlen(data), 0);
}



void UdsComWorker::thread_work(int socket)
{

	worker_thread_active = true;

	//start the listenerthread and remember the theadId of it
	StartListenerThread(pthread_self(), currentSocket, receiveBuffer);

	configSignals();

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

					request = receiveQueue.back();
					printf("Received: %s\n", request->c_str());

					paard->processMsg(request);

					popReceiveQueue();
				}
				break;

			default:
				printf("UdsComWorker: unkown signal \n");
				break;
		}

	}
	close(currentSocket);
}



void UdsComWorker::thread_listen(pthread_t parent_th, int socket, char* workerBuffer)
{

	listen_thread_active = true;
	int retval;
	fd_set rfds;

	configSignals();

	FD_ZERO(&rfds);
	FD_SET(socket, &rfds);

	while(listen_thread_active)
	{
		memset(receiveBuffer, '\0', BUFFER_SIZE);
		ready = true;

		retval = pselect(socket+1, &rfds, NULL, NULL, NULL, &origmask);

		if(retval < 0)
		{
			deletable = true;
			listen_thread_active = false;
		}
		else if(FD_ISSET(socket, &rfds))
		{
			recvSize = recv( socket , receiveBuffer, BUFFER_SIZE, MSG_DONTWAIT);

			if(recvSize > 0)
			{
				//add received data in buffer to queue
				pushReceiveQueue(new string(receiveBuffer, recvSize));
				pthread_kill(parent_th, SIGUSR1);
			}
			//RSD invoked shutdown
			else
			{
				deletable = true;
				listen_thread_active = false;
			}

		}
	}
}



