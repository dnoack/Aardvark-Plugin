/*
 * UdsRegWorker.cpp
 *
 *  Created on: 25.02.2015
 *      Author: Dave
 */


#include <sys/select.h>
#include <UdsRegWorker.hpp>
#include "errno.h"

#include "AardvarkPlugin.hpp"
#include "Plugin_Error.h"


UdsRegWorker::UdsRegWorker(int socket)
{
	memset(receiveBuffer, '\0', BUFFER_SIZE);

	this->listen_thread_active = false;
	this->worker_thread_active = false;
	this->recvSize = 0;
	this->currentMsgId = NULL;
	this->error = NULL;
	this->currentSocket = socket;
	this->json = new JsonRPC();
	this->state = NOT_ACTIVE;

	StartWorkerThread();
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



void UdsRegWorker::thread_listen()
{
	listen_thread_active = true;
	fd_set rfds;
	int retval;

	pthread_t worker_thread = getWorker();
	configSignals();

	FD_ZERO(&rfds);
	FD_SET(currentSocket, &rfds);

	while(listen_thread_active)
	{
		memset(receiveBuffer, '\0', BUFFER_SIZE);
		ready = true;

		retval = pselect(currentSocket+1, &rfds, NULL, NULL, NULL, &origmask);

		if(retval < 0)
		{
			//error
		}
		else if(FD_ISSET(currentSocket, &rfds))
		{
			recvSize = recv(currentSocket , receiveBuffer, BUFFER_SIZE, 0);

			if(recvSize > 0)
			{
				printf("Received: %s", receiveBuffer);
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
	memset(receiveBuffer, '\0', BUFFER_SIZE);
	worker_thread_active = true;

	//start the listenerthread and remember the theadId of it
	StartListenerThread();

	configSignals();

	while(worker_thread_active)
	{
		//wait for signals from listenerthread
		sigwait(&sigmask, &currentSig);
		switch(currentSig)
		{
			case SIGUSR1:
				processRegistration();
				break;

			default:
				printf("UdsRegWorker: unkown signal \n");
				break;
		}
	}
	close(currentSocket);
}


void UdsRegWorker::processRegistration()
{
	string* request = receiveQueue.back();
	const char* response = NULL;

	try{

		json->parse(request);
		currentMsgId = json->tryTogetId();
		currentMsgId->SetInt(currentMsgId->GetInt()+1);

		switch(state)
		{
			case NOT_ACTIVE:
				//check for announce ack then switch state to announced
				//and send register msg
				if(handleAnnounceACKMsg(request))
				{
					state = ANNOUNCED;
					response = createRegisterMsg();
					transmit(response, strlen(response));
				}
				break;
			case ANNOUNCED:
				if(handleRegisterACKMsg(request))
				{
					state = REGISTERED;
					//TODO: check if Plugin com part is ready, if yes -> state = active
					//create pluginActive msg
					response = createPluginActiveMsg();
					transmit(response, strlen(response));
				}
				//check for register ack then switch state to active
				break;
			case ACTIVE:
				//maybe heartbeat check
				break;
			case BROKEN:
				//clean up an set state to NOT_ACTIVE
				state = NOT_ACTIVE;
				break;
			default:
				//something went completely wrong
				state = BROKEN;
				break;
		}
	}
	catch(PluginError &e)
	{
		state = BROKEN;
		error = e.get();
		transmit(e.get(), strlen(e.get()));
	}
	popReceiveQueue();
}



bool UdsRegWorker::handleAnnounceACKMsg(string* msg)
{
	Value* resultValue = NULL;
	const char* resultString = NULL;
	bool result = false;


	resultValue = json->tryTogetResult();
	if(resultValue->IsString())
	{
		resultString = resultValue->GetString();
		if(strcmp(resultString, "announceACK") == 0)
			result = true;
	}
	else
	{
		error = json->generateResponseError(*currentMsgId, -31010, "Awaited \"announceACK\" but didn't receive it.");
		throw PluginError(error);
	}

	return result;
}


const char* UdsRegWorker::createRegisterMsg()
{
	Document dom;
	Value method;
	Value params;
	Value f;
	Value fNumber;
	int count = 1;
	char* buffer = NULL;

	list<string*>* funcList;
	string* tempString;


	//get methods from plugin
	funcList = AardvarkPlugin::getFuncList();
	method.SetString("register");
	params.SetObject();
	for(list<string*>::iterator i = funcList->begin(); i != funcList->end(); )
	{
		memset(buffer, '\0', 0);
		buffer = new char[10];
		tempString = *i;

		sprintf(buffer, "f%d", count);
		fNumber.SetString(buffer, dom.GetAllocator());
		f.SetString(tempString->c_str(), tempString->size());
		params.AddMember(fNumber,f, dom.GetAllocator());

		delete[] buffer;
		i = funcList->erase(i);
		count++;
	}
	delete funcList;

	return json->generateRequest(method, params, *currentMsgId);
}



bool UdsRegWorker::handleRegisterACKMsg(string* msg)
{
	const char* resultString = NULL;
	Value* resultValue = NULL;
	bool result = false;


	resultValue = json->tryTogetResult();
	if(resultValue->IsString())
	{
		resultString = resultValue->GetString();
		if(strcmp(resultString, "registerACK") == 0)
			result = true;
	}
	else
	{
		error = json->generateResponseError(*currentMsgId, -31011, "Awaited \"registerACK\" but didn't receive it.");
		throw PluginError(error);
	}
	return result;
}



const char* UdsRegWorker::createPluginActiveMsg()
{
	Value method;
	Value* params = NULL;
	Value* id = NULL;
	const char* msg = NULL;

	method.SetString("pluginActive");
	msg = json->generateRequest(method, *params, *id);

	return msg;
}

int UdsRegWorker::transmit(char* data, int size)
{
	return send(currentSocket, data, size, 0);
};


int UdsRegWorker::transmit(const char* data, int size)
{
	return send(currentSocket, data, size, 0);
};


int UdsRegWorker::transmit(string* msg)
{
	return send(currentSocket, msg->c_str(), msg->size(), 0);
};

