/*
 * UdsRegWorker.cpp
 *
 *  Created on: 25.02.2015
 *      Author: Dave
 */

#include <cstring>
#include <sys/select.h>
#include "errno.h"

#include "AardvarkPlugin.hpp"
#include <UdsRegWorker.hpp>
#include "Plugin_Error.h"


UdsRegWorker::UdsRegWorker(int socket)
{
	memset(receiveBuffer, '\0', BUFFER_SIZE);

	this->listen_thread_active = false;
	this->worker_thread_active = false;
	this->recvSize = 0;
	this->lthread = 0;
	this->error = NULL;
	this->currentSocket = socket;
	this->json = new JsonRPC();
	this->state = NOT_ACTIVE;

	StartWorkerThread(currentSocket);
}



UdsRegWorker::~UdsRegWorker()
{
	worker_thread_active = false;
	listen_thread_active = false;

	pthread_cancel(getListener());
	pthread_cancel(getWorker());

	WaitForListenerThreadToExit();
	WaitForWorkerThreadToExit();

	delete json;
}



void UdsRegWorker::thread_listen(pthread_t parent_th, int socket, char* workerBuffer)
{
	listen_thread_active = true;
	fd_set rfds;
	int retval;

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
			//error
		}
		else if(FD_ISSET(socket, &rfds))
		{
			recvSize = recv( socket , receiveBuffer, BUFFER_SIZE, 0);

			if(recvSize > 0)
			{
				printf("Received: %s", receiveBuffer);
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
	printf("UdsRegWorker: Listener beendet.\n");
}


void UdsRegWorker::thread_work(int socket)
{
	memset(receiveBuffer, '\0', BUFFER_SIZE);
	worker_thread_active = true;
	string* request = NULL;
	char* response = NULL;

	//start the listenerthread and remember the theadId of it
	lthread = StartListenerThread(pthread_self(), currentSocket, receiveBuffer);

	configSignals();

	while(worker_thread_active)
	{
		//wait for signals from listenerthread

		sigwait(&sigmask, &currentSig);
		switch(currentSig)
		{
			case SIGUSR1:
				while(getReceiveQueueSize( )> 0)
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
								//TODO: check if Plugin com part is ready, if yes -> state = active

								//create pluginActive msg
								response = createPluginActiveMsg();
								send(currentSocket, response, strlen(response), 0);
							}
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
					popReceiveQueue();
				}
				break;

			case SIGUSR2:
				printf("UdsRegWorker: SIGUSR2\n");
				break;

			default:
				printf("UdsRegWorker: unkown signal \n");
				break;
		}
	}
	close(currentSocket);
}



bool UdsRegWorker::handleAnnounceACKMsg(string* msg)
{
	Value* resultValue = NULL;
	Value nullid;
	const char* resultString = NULL;
	bool result = false;

	json->parse(msg);

	resultValue = json->tryTogetResult();
	if(resultValue->IsString())
	{
		resultString = resultValue->GetString();
		if(strcmp(resultString, "announceACK") == 0)
			result = true;
	}
	else
	{
		error = json->generateResponseError(nullid, -31010, "Awaited \"announceACK\" but didn't receive it.");
		throw PluginError(error);
	}

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

	list<string*>* funcList;
	string* tempString;


	//get methods from plugin
	funcList = AardvarkPlugin::getFuncList();
	method.SetString("register");
	params.SetObject();
	for(list<string*>::const_iterator i = funcList->begin(); i != funcList->end(); ++i)
	{
		memset(buffer, '\0', 0);
		buffer = new char[10];
		tempString = *i;

		sprintf(buffer, "f%d", count);
		fNumber.SetString(buffer, dom.GetAllocator());
		f.SetString(tempString->c_str(), tempString->size());
		params.AddMember(fNumber,f, dom.GetAllocator());

		delete[] buffer;
		count++;
	}

	id.SetInt(2);
	return json->generateRequest(method, params, id);
}



bool UdsRegWorker::handleRegisterACKMsg(string* msg)
{
	const char* resultString = NULL;
	Value* resultValue = NULL;
	Value nullid;
	bool result = false;

	json->parse(msg);

	resultValue = json->tryTogetResult();
	if(resultValue->IsString())
	{
		resultString = resultValue->GetString();
		if(strcmp(resultString, "registerACK") == 0)
			result = true;
	}
	else
	{
		error = json->generateResponseError(nullid, -31011, "Awaited \"registerACK\" but didn't receive it.");
		throw PluginError(error);
	}
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

