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
	detectDevices();
}



PluginAardvark::~PluginAardvark()
{
	pthread_mutex_destroy(&dLmutex);
	int size = deviceList.size();
	delete json;

	if(result != NULL)
		delete result;


	for(int i = 0; i < size; i++)
		delete deviceList[i];

	deviceList.clear();
	vector<RemoteAardvark*>().swap(deviceList);
}


RemoteAardvark* PluginAardvark::getDevice(int handle)
{
	RemoteAardvark* device;
	pthread_mutex_lock(&dLmutex);
	device = deviceList[handle];
	pthread_mutex_unlock(&dLmutex);
	return device;
}


void PluginAardvark::detectDevices()
{
	u16 devices[EXPECTED_NUM_OF_DEVICES];
	u32 unique_ids[EXPECTED_NUM_OF_DEVICES];
	int found = 0;

	pthread_mutex_lock(&dLmutex);

	found = aa_find_devices_ext(EXPECTED_NUM_OF_DEVICES, devices, EXPECTED_NUM_OF_DEVICES, unique_ids);


	for(int i = 0; i < found; i++)
	{
		if(unique_ids[i] != 0)
		{
			//devices will be deleted within destructor
			deviceList.push_back(new RemoteAardvark());

		}
	}

	pthread_mutex_unlock(&dLmutex);
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
					device = getDevice((*dom)["params"]["port"].GetInt());
					device->executeFunction((*dom)["method"], (*dom)["params"], responseValue);
					result = new string(json->generateResponse((*dom)["id"], responseValue));
				}
				else
				{
					if((*dom)["params"].HasMember("handle"))
					{
						device = getDevice((*dom)["params"]["handle"].GetInt()-1);
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
		result = new string(json->getResponseError());
		throw;
	}

	return result;
}

