/*
 * AardvarkCareTaker.cpp
 *
 *  Created on: 05.02.2015
 *      Author: dnoack
 */

#include "UdsComWorker.hpp"
#include "AardvarkCareTaker.hpp"
#include "Utils.h"


list<RemoteAardvark*> AardvarkCareTaker::deviceList;
pthread_mutex_t AardvarkCareTaker::dLmutex;



AardvarkCareTaker::AardvarkCareTaker(UdsComWorker* udsWorker)
{
	msgList = NULL;
	this->udsworker = udsWorker;
	json = new JsonRPC();
	contextNumber = 0;
	result = NULL;
	deviceLessFunctions = new RemoteAardvark(-1);
}


AardvarkCareTaker::~AardvarkCareTaker()
{
	delete json;
	unlockAllUsedDevices();
	delete deviceLessFunctions;
}


void AardvarkCareTaker::init()
{
	pthread_mutex_init(&dLmutex, NULL);
}


void AardvarkCareTaker::deInit()
{
	deleteDeviceList();
	pthread_mutex_destroy(&dLmutex);
}


RemoteAardvark* AardvarkCareTaker::getDevice(Document* dom)
{
	RemoteAardvark* device = NULL;
	bool found = false;
	const char* error = NULL;
	int value = 0;
	int valueType = -1;

	if((*dom)["params"].HasMember("port"))
	{
		valueType = PORT;
		value = (*dom)["params"]["port"].GetInt();
	}
	else if((*dom)["params"].HasMember("handle") )
	{
		valueType = HANDLE;
		value = (*dom)["params"]["handle"].GetInt();
	}
	else if((*dom)["params"].HasMember("Aardvark") )
	{
		valueType = HANDLE;
		value = (*dom)["params"]["Aardvark"].GetInt();
	}
	else
	{
		device = deviceLessFunctions;
	}

	pthread_mutex_lock(&dLmutex);
	list<RemoteAardvark*>::iterator tempDevice = deviceList.begin();

	while(tempDevice != deviceList.end() && !found)
	{
		if(valueType == HANDLE)
		{
			if((*tempDevice)->getHandle() == value)
			{
				device = *tempDevice;
				found = true;
			}
		}
		else if(valueType == PORT)
		{
			if((*tempDevice)->getPort() == value)
			{
				device = *tempDevice;
				found = true;
			}
		}
		++tempDevice;
	}

	//we found a device, but is no one else using it ?
	if(found)
	{
		if(device->getContextNumber() == 0 || device->getContextNumber() == contextNumber)
		{
			dyn_print("Found device and got access to it. context: %d \n", contextNumber);
			if(device->getContextNumber() == 0)
				device->setContextNumber(contextNumber);
		}
		else
		{
			pthread_mutex_unlock(&dLmutex);
			dyn_print("Requesting context %d.  using context: %d \n", contextNumber, device->getContextNumber());
			error = json->generateResponseError(*(json->getId(dom)), -99998, "Another user is using the requested hardware.");
			throw Error(error);
		}
	}
	else //didnt found the device
	{
		if(valueType == PORT) //create a new device instance with value as port
		{
			device = new RemoteAardvark(value);
			device->setContextNumber(contextNumber);
			deviceList.push_back(device);
		}
		else // cant create new devices with handle as value
		{
			pthread_mutex_unlock(&dLmutex);
			error = json->generateResponseError(*(json->getId(dom)), -99999, "No device with this handle available.");
			throw Error(error);
		}
	}

	pthread_mutex_unlock(&dLmutex);
	return device;
}





//main method for processing new json rpc msgs
void AardvarkCareTaker::processMsg(string* msg)
{
	Document* dom = new Document();
	Value responseValue;
	Value* paramValue;
	RemoteAardvark* device;
	list<string*>::iterator currentMsg;


	msgList = json->splitMsg(dom, msg);
	currentMsg = msgList->begin();


	while(currentMsg != msgList->end())
	{
		try
		{
			dom = json->parse(*currentMsg);

			if(json->isRequest(dom))
			{
				if(json->hasParams(dom))
				{
					dyn_print("Request incomming, gettin device...\n");

					device = getDevice(dom);
					device->executeFunction((*dom)["method"], (*dom)["params"], responseValue);
					result = new string(json->generateResponse((*dom)["id"], responseValue));
					udsworker->transmit(result);
					delete result;
				}

			}
			else if(json->isNotification(dom))
			{
				paramValue = json->tryTogetParam(dom, "contextNumber");
				contextNumber = paramValue->GetInt();
				dyn_print("Received Notification for context: %d", contextNumber);
			}
			else
			{
				throw Error("Aardvark-Plugin received: NO Request.\n");
			}
			delete *currentMsg;
			currentMsg = msgList->erase(currentMsg);

		}
		catch(Error &e)
		{
			udsworker->transmit(e.get(), strlen(e.get()));
			delete *currentMsg;
			currentMsg = msgList->erase(currentMsg);

		}

	}
	delete msgList;
	delete dom;

}


void AardvarkCareTaker::deleteDeviceList()
{
	pthread_mutex_lock(&dLmutex);
	list<RemoteAardvark*>::iterator device = deviceList.begin();

	while(device != deviceList.end())
	{
		delete *device;
		device = deviceList.erase(device);
	}
	pthread_mutex_unlock(&dLmutex);
}


void AardvarkCareTaker::unlockAllUsedDevices()
{
	pthread_mutex_lock(&dLmutex);
	list<RemoteAardvark*>::iterator device = deviceList.begin();

	while(device != deviceList.end())
	{
		if((*device)->getContextNumber() == contextNumber)
		{
			(*device)->setContextNumber(0);
			(*device)->close();
		}
		++device;
	}

	pthread_mutex_unlock(&dLmutex);
}


