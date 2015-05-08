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
#include "Utils.h"



UdsComWorker::UdsComWorker(int socket)
{
	this->request = 0;
	this->response = 0;
	this->currentSocket = socket;
	this->paard = new AardvarkCareTaker(this);

	StartWorkerThread();

	if(wait_for_listener_up() != 0)
		throw Error("Creation of Listener/worker threads failed.");
}



UdsComWorker::~UdsComWorker()
{
	worker_thread_active = false;
	listen_thread_active = false;

	pthread_cancel(getListener());
	pthread_cancel(getWorker());

	WaitForListenerThreadToExit();
	WaitForWorkerThreadToExit();

	if(paard != NULL)
		delete paard;
	deleteReceiveQueue();
}



void UdsComWorker::thread_work()
{

	worker_thread_active = true;

	configSignals();
	StartListenerThread();


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
					dyn_print("Worker Received: %s\n", request->c_str());
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



void UdsComWorker::thread_listen()
{

	listen_thread_active = true;
	int retval;
	fd_set rfds;
	pthread_t worker_thread = getWorker();

	FD_ZERO(&rfds);
	FD_SET(currentSocket, &rfds);

	while(listen_thread_active)
	{
		memset(receiveBuffer, '\0', BUFFER_SIZE);

		retval = pselect(currentSocket+1, &rfds, NULL, NULL, NULL, &origmask);

		if(retval < 0)
		{
			deletable = true;
			listen_thread_active = false;
		}
		else if(FD_ISSET(currentSocket, &rfds))
		{
			recvSize = recv(currentSocket , receiveBuffer, BUFFER_SIZE, MSG_DONTWAIT);

			if(recvSize > 0)
			{
				//add received data in buffer to queue
				pushReceiveQueue(new string(receiveBuffer, recvSize));
				dyn_print("Listener Received: %s\n", receiveBuffer);
				pthread_kill(worker_thread, SIGUSR1);
			}
			//RSD invoked shutdown
			else
			{
				dyn_print("Error receiveSize < 0 , Errno: %s\n", strerror(errno));
				deletable = true;
				listen_thread_active = false;
				delete paard;
				paard = NULL;
			}

		}
	}
}



int UdsComWorker::transmit(const char* data, int size)
{
	return send(currentSocket, data, size, 0);
};


int UdsComWorker::transmit(string* msg)
{
	return send(currentSocket, msg->c_str(), msg->size(), 0);
};




