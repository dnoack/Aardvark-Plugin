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

	//start the listenerthread and remember the theadId of it
	StartListenerThread(pthread_self(), currentSocket, receiveBuffer);

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


int UdsRegWorker::uds_send(char* msg)
{
	int wrote = 0;

	wrote = send(currentSocket, msg, strlen(msg), 0);
	if(wrote == -1)
	{
		error = json->generateResponseError(*currentMsgId, -31013, "Could not send Data over UdsSocket.");
		throw PluginError(error);
	}
	return wrote;
}


void UdsRegWorker::processRegistration()
{
	string* request = receiveQueue.back();
	const char* response = NULL;

	printf("Received: %s \n", request->c_str());

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
		send(currentSocket, error, strlen(error), 0);
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

