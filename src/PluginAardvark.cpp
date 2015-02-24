/*
 * PluginAardvark.cpp
 *
 *  Created on: 05.02.2015
 *      Author: dnoack
 */


#include "PluginAardvark.hpp"

vector<RemoteAardvark*> PluginAardvark::deviceList;
pthread_mutex_t PluginAardvark::dLmutex;


PluginAardvark::PluginAardvark()
{
	pthread_mutex_init(&dLmutex, NULL);
	json = new JsonRPC();
	result = NULL;
	user = new string();
}



PluginAardvark::~PluginAardvark()
{
	pthread_mutex_destroy(&dLmutex);
	int size = deviceList.size();
	delete json;
	delete user;

	for(int i = 0; i < size; i++)
		delete deviceList[i];

	deviceList.clear();
	vector<RemoteAardvark*>().swap(deviceList);
}


RemoteAardvark* PluginAardvark::getDevice(int value, int valueType)
{
	RemoteAardvark* device = NULL;
	bool found = false;

	pthread_mutex_lock(&dLmutex);
	for(unsigned int i = 0; i < deviceList.size() && !found ; i++)
	{
		if(valueType == HANDLE)
		{
			if(deviceList[i]->getHandle() == value)
			{
				device = deviceList[i];
				found = true;
			}
		}
		if(valueType == PORT)
		{
			if(deviceList[i]->getPort() == value)
			{
				device = deviceList[i];
				found = true;
			}
		}
	}

	//we found a device, but is no one else using it ?
	if(found)
	{
		//TODO: compare user string from PluginAardvark with user string of RemoteAardvark
	}
	else //didnt found the device
	{
		if(valueType == PORT) //create a new device instance
		{
			device = new RemoteAardvark(value);
			deviceList.push_back(device);
		}
		else
		{
			pthread_mutex_unlock(&dLmutex);
			throw PluginError("No device with this handle available.");
		}
	}

	pthread_mutex_unlock(&dLmutex);
	return device;
}





//main method for processing new json rpc msgs
string* PluginAardvark::processMsg(string* msg)
{
	Document* dom;
	Value responseValue;
	RemoteAardvark* device;

	try
	{
		//parses the msg string into a internal dom object
		dom = json->parse(msg);


		if(json->checkJsonRpc_RequestFormat())
		{
			if(json->isRequest())
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
			else
			{
				//TODO: notification
			}
		}
	}
	catch(PluginError &e)
	{
		throw;
	}

	return result;
}

