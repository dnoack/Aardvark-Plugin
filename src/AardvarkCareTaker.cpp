/*
 * AardvarkCareTaker.cpp
 *
 *  Created on: 05.02.2015
 *      Author: dnoack
 */


#include "AardvarkCareTaker.hpp"

//list<RemoteAardvark*> AardvarkCareTaker::deviceList;
pthread_mutex_t AardvarkCareTaker::dLmutex;


AardvarkCareTaker::AardvarkCareTaker()
{
	pthread_mutex_init(&dLmutex, NULL);
	json = new JsonRPC();
	user = new string();
	result = NULL;
}



AardvarkCareTaker::~AardvarkCareTaker()
{
	delete json;
	delete user;
	deleteDeviceList();
	pthread_mutex_destroy(&dLmutex);
}


RemoteAardvark* AardvarkCareTaker::getDevice(int value, int valueType)
{

	RemoteAardvark* device = NULL;
	bool found = false;
	char* error = NULL;
	list<RemoteAardvark*>::iterator i = deviceList.begin();

	pthread_mutex_lock(&dLmutex);
	while(i != deviceList.end() && !found)
	{
		if(valueType == HANDLE)
		{
			if((*i)->getHandle() == value)
			{
				device = *i;
				found = true;
			}
		}
		else if(valueType == PORT)
		{
			if((*i)->getPort() == value)
			{
				device = *i;
				found = true;
			}
		}
		++i;
	}

	//we found a device, but is no one else using it ?
	if(found)
	{
		//TODO: compare user string from PluginAardvark with user string of RemoteAardvark
		printf("Found device\n");
	}
	else //didnt found the device
	{
		if(valueType == PORT) //create a new device instance with value as port
		{
			device = new RemoteAardvark(value);
			deviceList.push_back(device);
		}
		else // cant create new devices with handle as value
		{
			pthread_mutex_unlock(&dLmutex);
			error = json->generateResponseError(*(json->getId()), -99999, "No device with this handle available.");
			throw PluginError(error);
		}
	}

	pthread_mutex_unlock(&dLmutex);
	return device;
}





//main method for processing new json rpc msgs
string* AardvarkCareTaker::processMsg(string* msg)
{
	Document* dom;
	Value responseValue;
	RemoteAardvark* device;

	try
	{
		//parses the msg string into a internal dom object
		dom = json->parse(msg);


		if(json->isRequest())
		{
			if(json->hasParams())
			{
				if((*dom)["params"].HasMember("port"))
				{

					device = getDevice((*dom)["params"]["port"].GetInt(), PORT);
					device->executeFunction((*dom)["method"], (*dom)["params"], responseValue);
					result = new string(json->generateResponse((*dom)["id"], responseValue));
				}
				else
				{
					if((*dom)["params"].HasMember("handle"))
					{
						device = getDevice((*dom)["params"]["handle"].GetInt(), HANDLE);
						device->executeFunction((*dom)["method"], (*dom)["params"], responseValue);
						result = new string(json->generateResponse((*dom)["id"], responseValue));
					}
				}
			}

		}
		else
		{
			throw PluginError("Aardvark-Plugin received: NO Request.\n");
		}

	}
	catch(PluginError &e)
	{
		throw;
	}

	return result;
}


void AardvarkCareTaker::deleteDeviceList()
{
	list<RemoteAardvark*>::iterator i = deviceList.begin();

	while(i != deviceList.end())
	{
		delete *i;
		i = deviceList.erase(i);
	}
}

