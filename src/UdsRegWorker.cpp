/*
 * UdsRegWorker.cpp
 *
 *  Created on: 25.02.2015
 *      Author: Dave
 */

#include <UdsRegWorker.hpp>
#include <cstring>

UdsRegWorker::UdsRegWorker(int socket)
{
	this->listen_thread_active = false;
	this->worker_thread_active = false;
	this->recvSize = 0;
	this->lthread = 0;
	this->currentSocket = socket;
	memset(receiveBuffer, '\0', BUFFER_SIZE);
}



UdsRegWorker::~UdsRegWorker()
{
	// TODO Auto-generated destructor stub
}



void UdsRegWorker::thread_listen(pthread_t parent_th, int socket, char* workerBuffer)
{
	listen_thread_active = true;

	while(listen_thread_active)
	{
		memset(receiveBuffer, '\0', BUFFER_SIZE);

		//TODO:msg_dontwait lets us cancel the client with listen_thread_active flag but not with sigpipe (disco at clientside)
		recvSize = recv( socket , receiveBuffer, BUFFER_SIZE, MSG_DONTWAIT);
		if(recvSize > 0)
		{
			//add received data in buffer to queue
			editReceiveQueue(new string(receiveBuffer, recvSize), true);

			//worker is doing nothing, wake him up
			if(!workerStatus(WORKER_GETSTATUS))
			{
				workerStatus(WORKER_BUSY);
				//signal the worker
				pthread_kill(parent_th, SIGUSR1);
			}
		}

	}
	printf("Listener beendet.\n");
	pthread_kill(parent_th, SIGPOLL);


}


void UdsRegWorker::thread_work(int socket)
{
	memset(receiveBuffer, '\0', BUFFER_SIZE);
	worker_thread_active = true;

	//start the listenerthread and remember the theadId of it
	lthread = StartListenerThread(pthread_self(), currentSocket, receiveBuffer);


	while(worker_thread_active)
	{
		//wait for signals from listenerthread

		sigwait(&sigmask, &currentSig);
		switch(currentSig)
		{
			case SIGUSR1:
				while(receiveQueue.size() > 0)
				{
					//sigusr1 = there is data for work e.g. parsing json rpc
					printf("We received something and the worker got a signal.\n");


				}
				break;

			case SIGUSR2:
				//sigusr2 = time to exit
				worker_thread_active = false;
				listen_thread_active = false;
				break;

			case SIGPIPE:
				listen_thread_active = false;
				worker_thread_active = false;
				break;
			default:
				worker_thread_active = false;
				listen_thread_active = false;
				break;
		}
		workerStatus(WORKER_FREE);
	}
	close(currentSocket);
	printf("Worker Thread beendet.\n");
	WaitForListenerThreadToExit();

}
