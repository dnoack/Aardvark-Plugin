
#include "UdsRegWorker.hpp"
#include "UdsRegClient.hpp"


UdsRegWorker::UdsRegWorker(UdsRegClient* regClient, int socket)
{
	recvSize = 0;
	this->currentSocket = socket;
	this->regClient = regClient;

	StartWorkerThread();

	if(wait_for_listener_up() != 0)
		throw Error("Creation of Listener/worker threads failed.");
}



UdsRegWorker::~UdsRegWorker()
{
	worker_thread_active = false;
	listen_thread_active = false;

	pthread_cancel(getListener());
	pthread_cancel(getWorker());

	WaitForListenerThreadToExit();
	WaitForWorkerThreadToExit();

	deleteReceiveQueue();

}



void UdsRegWorker::thread_listen()
{
	listen_thread_active = true;
	fd_set rfds;
	int retval;
	pthread_t worker_thread = getWorker();


	FD_ZERO(&rfds);
	FD_SET(currentSocket, &rfds);

	while(listen_thread_active)
	{
		memset(receiveBuffer, '\0', BUFFER_SIZE);

		retval = select(currentSocket+1, &rfds, NULL, NULL, NULL);

		if(retval < 0)
		{
			deletable = true;
			listen_thread_active = false;
		}
		else if(FD_ISSET(currentSocket, &rfds))
		{
			recvSize = recv(currentSocket , receiveBuffer, BUFFER_SIZE, 0);

			if(recvSize > 0)
			{
				pushReceiveQueue(new string(receiveBuffer, recvSize));
				pthread_kill(worker_thread, SIGUSR1);
			}
			//RSD invoked shutdown
			else
			{
				deletable = true;
				listen_thread_active = false;
			}
		}
	}
	printf("UdsRegWorker: Listener beendet.\n");
}


void UdsRegWorker::thread_work()
{
	string* msg = NULL;
	worker_thread_active = true;

	//start the listenerthread and remember the theadId of it

	configSignals();
	StartListenerThread();



	while(worker_thread_active)
	{
		//wait for signals from listenerthread
		sigwait(&sigmask, &currentSig);
		switch(currentSig)
		{
			case SIGUSR1:
				msg = receiveQueue.back();
				popReceiveQueueWithoutDelete();
				regClient->processRegistration(msg);
				break;

			default:
				printf("UdsRegWorker: unkown signal \n");
				break;
		}
	}
	close(currentSocket);
}





int UdsRegWorker::transmit(const char* data, int size)
{
	return send(currentSocket, data, size, 0);
};


int UdsRegWorker::transmit(string* msg)
{
	return send(currentSocket, msg->c_str(), msg->size(), 0);
};

