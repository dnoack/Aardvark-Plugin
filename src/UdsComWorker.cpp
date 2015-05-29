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


/*
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
}*/


void UdsComWorker::thread_listen()
{

	listen_thread_active = true;
	int retval;
	pthread_t worker_thread = getWorker();
	char* msgBuffer = NULL;
	char* tempBuffer = NULL;
	int msgBufferSize = 0;
	int oldMsgBufferSize = 0;
	int messageSize = 0;
	fd_set rfds;
	struct timeval timeout;

	timeout.tv_sec = 3;
	timeout.tv_usec = 0;


	FD_ZERO(&rfds);
	FD_SET(currentSocket, &rfds);

	while(listen_thread_active)
	{
		memset(receiveBuffer, '\0', BUFFER_SIZE);

		//hier nur hin wenn nichts mehr im buffer ist und auf keine teile einer nachricht gewartet wird.
		recvSize = recv(currentSocket , receiveBuffer, BUFFER_SIZE, 0);
		if(recvSize > 0)
		{

			//msgBufferSize = receiveBufferSize;
			msgBufferSize = recvSize;

			//allocate msgBuffer with receiveBufferSize
			msgBuffer = new char[msgBufferSize];

			//copy content of receiveBuffer to msgBuffer (receiveBufferSize)
			memcpy(msgBuffer, receiveBuffer, msgBufferSize);

			//while(msgBuffer > 0)
			while(msgBufferSize > 0)
			{

				//headersize = 1 tag + 4 length
				if(msgBufferSize > HEADER_SIZE)
				{
					messageSize = readHeader(msgBuffer);
					//header ok ???
					if(messageSize > -1)
					{

						if(msgBufferSize >= messageSize+HEADER_SIZE) //es befindet sich min. eine vollständige nachricht im msgBuffer
						{
							//erzeuge std::string von msgBuffer[5] bis msgBuffer[messageSize-5]
							//füge string zu msgList hinzu (mutex geschützt)
							pushReceiveQueue(new string(&msgBuffer[HEADER_SIZE], messageSize));

							//signal an worker
							pthread_kill(worker_thread, SIGUSR1);

							if(msgBufferSize > messageSize+HEADER_SIZE)
							{
								//setze msgBufferSize auf größe von rest
								msgBufferSize = msgBufferSize - (messageSize+HEADER_SIZE);
								//kopiere restlichen inhalt in tempBuffer
								tempBuffer = new char[msgBufferSize];
								memcpy(tempBuffer, &(msgBuffer[messageSize+HEADER_SIZE]), msgBufferSize);
								//deallokiere msgBuffer
								delete[] msgBuffer;
								//msgBuffer = tempBuffer
								msgBuffer = tempBuffer;
							}
							else
							{
								delete[] msgBuffer;
								msgBufferSize = 0;
							}
						}
						else
						{
							timeout.tv_sec = 3;
							timeout.tv_usec = 0;

							//wait for more with select and timeout
							retval = select(currentSocket+1, &rfds, NULL, NULL, &timeout);
							if(retval > 0)
							{
								//recv
								recvSize = recv(currentSocket , receiveBuffer, BUFFER_SIZE, 0);
								//tempBuffer von größe recvSize + msgBufferSize ( = msgBufferSize)
								oldMsgBufferSize = msgBufferSize;
								msgBufferSize = msgBufferSize + recvSize;
								tempBuffer = new char[msgBufferSize];
								//kopiere msgBuffer und recvBuffer in tempBuffer
								memcpy(tempBuffer, msgBuffer, oldMsgBufferSize);
								memcpy(&tempBuffer[oldMsgBufferSize], receiveBuffer, recvSize);
								//deallokiere msgBuffer
								delete[] msgBuffer;
								//msgBuffer = tempBuffer
								msgBuffer = tempBuffer;
							}
							else if(retval == 0)
							{
								//timeout oder verbindung geschlossen
								//in diesem fall gehen wir von timeout aus.
								delete[] msgBuffer;
								msgBufferSize = 0;

							}
							else
							{
								//errno prüfen
								//error msg senden
							}


						}
					}
					else
					{
						delete[] msgBuffer;
						msgBufferSize = 0;
						//TODO: send msg back, that something went wrong and the message was not correctly received
					}
				}
				else
				{
					timeout.tv_sec = 5;
					timeout.tv_usec = 1;

					retval = select(currentSocket+1, &rfds, NULL, NULL, &timeout);
					if(retval > 0)
					{
						//recv
						recvSize = recv(currentSocket , receiveBuffer, BUFFER_SIZE, 0);
						//tempBuffer von größe recvSize + msgBufferSize ( = msgBufferSize)
						oldMsgBufferSize = msgBufferSize;
						msgBufferSize = msgBufferSize + recvSize;
						tempBuffer = new char[msgBufferSize];
						memset(tempBuffer, '\0', msgBufferSize);
						//kopiere msgBuffer und recvBuffer in tempBuffer
						memcpy(tempBuffer, msgBuffer, oldMsgBufferSize);
						memcpy(&tempBuffer[oldMsgBufferSize], receiveBuffer, recvSize);
						//deallokiere msgBuffer
						delete[] msgBuffer;
						//msgBuffer = tempBuffer
						msgBuffer = tempBuffer;
					}
					else if(retval == 0)
					{
						//timeout oder verbindung geschlossen
						//in diesem fall gehen wir von timeout aus.
						delete[] msgBuffer;
						msgBufferSize = 0;

					}
					else
					{
						//errno prüfen
					}
}

			}
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




int UdsComWorker::transmit(const char* data, int size)
{
	return send(currentSocket, data, size, 0);
};


int UdsComWorker::transmit(string* msg)
{
	return send(currentSocket, msg->c_str(), msg->size(), 0);
};




