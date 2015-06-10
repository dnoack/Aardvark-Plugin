/*
 * AardvarkCareTaker.cpp
 *
 *  Created on: 05.02.2015
 *      Author: dnoack
 */


#include "AardvarkCareTaker.hpp"
#include "ProcessInterface.hpp"


list<RemoteAardvark*> AardvarkCareTaker::deviceList;
pthread_mutex_t AardvarkCareTaker::dLmutex;



AardvarkCareTaker::AardvarkCareTaker()
{
	json = new JsonRPC();
	id = NULL;
	error = NULL;
	currentDom = NULL;
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


RemoteAardvark* AardvarkCareTaker::getDevice(int value, int valueType)
{

	RemoteAardvark* device = NULL;
	bool found = false;
	const char* error = NULL;

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
			//dyn_print("Found device and got access to it. context: %d \n", contextNumber);
			if(device->getContextNumber() == 0)
				device->setContextNumber(contextNumber);
		}
		else
		{
			pthread_mutex_unlock(&dLmutex);
			//dyn_print("Requesting context %d.  using context: %d \n", contextNumber, device->getContextNumber());
			error = json->generateResponseError(*(json->getId(currentDom)), -99998, "Another user is using the requested hardware.");
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
			error = json->generateResponseError(*(json->getId(currentDom)), -99999, "No device with this handle available.");
			throw Error(error);
		}
	}

	pthread_mutex_unlock(&dLmutex);
	return device;
}





//main method for processing new json rpc msgs
void AardvarkCareTaker::process(RPCMsg* msg)
{

	Value responseValue;
	Value* paramValue;
	RemoteAardvark* device;

	currentDom = new Document();

	try
	{
		json->parse(currentDom, msg->getContent());

		if(json->isRequest(currentDom))
		{
			id = json->getId(currentDom);
			if(json->hasParams(currentDom))
			{
				//dyn_print("Request incomming, gettin device...\n");

				if((*currentDom)["params"].HasMember("port"))
				{
					device = getDevice((*currentDom)["params"]["port"].GetInt(), PORT);

				}
				else if((*currentDom)["params"].HasMember("handle") )
				{
					device = getDevice((*currentDom)["params"]["handle"].GetInt(), HANDLE);
				}
				else if((*currentDom)["params"].HasMember("Aardvark") )
				{
					device = getDevice((*currentDom)["params"]["Aardvark"].GetInt(), HANDLE);
				}
				else
				{
					device = deviceLessFunctions;
				}
				device->executeFunction((*currentDom)["method"], (*currentDom)["params"], responseValue);
				result = new string(json->generateResponse((*currentDom)["id"], responseValue));
				comPoint->transmit(result->c_str(), result->size());
			}

		}
		else if(json->isNotification(currentDom))
		{
			paramValue = json->tryTogetParam(currentDom, "contextNumber");
			contextNumber = paramValue->GetInt();
			//dyn_print("Received Notification for context: %d", contextNumber);
		}
		else
		{
			throw Error("Aardvark-Plugin received: NO Request.\n");
		}
	}
	catch(Error &e)
	{
		error = json->generateResponseError(*id, e.getErrorCode(), e.get());
		comPoint->transmit(error, strlen(error));
	}
	delete currentDom;
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
