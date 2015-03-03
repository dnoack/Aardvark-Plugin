/*
 * UdsRegWorker.cpp
 *
 *  Created on: 25.02.2015
 *      Author: Dave
 */

#include "AardvarkPlugin.hpp"
#include <UdsRegWorker.hpp>
#include <cstring>

UdsRegWorker::UdsRegWorker(int socket)
{
	this->listen_thread_active = false;
	this->worker_thread_active = false;
	this->recvSize = 0;
	this->lthread = 0;
	this->currentSocket = socket;
	this->json = new JsonRPC();
	this->state = NOT_ACTIVE;
	memset(receiveBuffer, '\0', BUFFER_SIZE);

	StartWorkerThread(currentSocket);
}



UdsRegWorker::~UdsRegWorker()
{
	delete json;
}



void UdsRegWorker::thread_listen(pthread_t parent_th, int socket, char* workerBuffer)
{
	listen_thread_active = true;

	while(listen_thread_active)
	{
		memset(receiveBuffer, '\0', BUFFER_SIZE);

		//TODO:msg_dontwait lets us cancel the client with listen_thread_active flag but not with sigpipe (disco at clientside)
		recvSize = recv( socket , receiveBuffer, BUFFER_SIZE, 0);
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
		if(recvSize == 0)
			listen_thread_active = false;
	}
	printf("Listener beendet.\n");
	pthread_kill(parent_th, SIGPOLL);


}


void UdsRegWorker::thread_work(int socket)
{
	memset(receiveBuffer, '\0', BUFFER_SIZE);
	worker_thread_active = true;
	string* request = NULL;
	char* response = NULL;

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
					request = receiveQueue.back();
					printf("Received: %s \n", request->c_str());
					//sigusr1 = there is data for work e.g. parsing json rpc
					switch(state)
					{
						case NOT_ACTIVE:
							//check for announce ack then switch state to announced
							//and send register msg
							if(handleAnnounceACKMsg(request))
							{
								state = ANNOUNCED;
								response = createRegisterMsg();
								send(currentSocket, response, strlen(response), 0);
							}
							break;
						case ANNOUNCED:
							if(handleRegisterACKMsg(request))
							{
								state = REGISTERED;
								//check if Plugin com part is ready, if yes -> state = active


								//create pluginActive msg
								response = createPluginActiveMsg();
								send(currentSocket, response, strlen(response), 0);
							}

							listen_thread_active = false;
							//check for register ack then switch state to active
							break;
						case ACTIVE:
							//maybe heartbeat check
							break;
						case BROKEN:
							//should not occur
							break;
						default:
							//something went completely wrong
							state = BROKEN;
							break;
					}
					editReceiveQueue(NULL, false);
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


bool UdsRegWorker::handleAnnounceACKMsg(string* msg)
{
	const char* rpcResult = NULL;
	bool result = false;

	json->parse(msg);

	//check if it is really announceACK (maybe it is announceNACK)
	rpcResult = json->getResult();
	if(strcmp(rpcResult, "announceACK") == 0)
		result = true;

	return result;
}


char* UdsRegWorker::createRegisterMsg()
{
	Document dom;
	Value method;
	Value params;
	Value f;
	Value id;
	Value fNumber;
	int count = 1;
	char* buffer = NULL;
	memset(buffer, '\0', 0);
	list<string*>* funcList;
	string* tempString;


	//get methods from plugin
	funcList = AardvarkPlugin::getFuncList();
	method.SetString("register");
	params.SetObject();
	for(list<string*>::const_iterator i = funcList->begin(); i != funcList->end(); ++i)
	{
		tempString = *i;
		buffer = new char[10];
		sprintf(buffer, "f%d\0", count);
		fNumber.SetString(buffer, strlen(buffer));
		f.SetString(tempString->c_str(), tempString->size());
		params.AddMember(fNumber,f, dom.GetAllocator());
		count++;
	}

	id.SetInt(2);
	return json->generateRequest(method, params, id);

	//create request json rpc with methodnames as parameters
}



bool UdsRegWorker::handleRegisterACKMsg(string* msg)
{
	const char* rpcResult = NULL;
	bool result = false;

	json->parse(msg);

	//check if it is really announceACK (maybe it is announceNACK)
	rpcResult = json->getResult();
	if(strcmp(rpcResult, "registerACK") == 0)
		result = true;

	return result;
}



char* UdsRegWorker::createPluginActiveMsg()
{
	Value method;
	Value* params = NULL;
	Value* id = NULL;
	Document dom;
	char* msg = NULL;

	method.SetString("pluginActive");

	msg = json->generateRequest(method, *params, *id);

	return msg;
}

