/*
 * PluginAardvark.cpp
 *
 *  Created on: 05.02.2015
 *      Author: dnoack
 */


#include "PluginAardvark.hpp"

vector<RemoteAardvark*>* PluginAardvark::deviceList;
pthread_mutex_t PluginAardvark::dLmutex;


PluginAardvark::PluginAardvark()
{
	pthread_mutex_init(&dLmutex, NULL);
	json = new JsonRPC();
	//deviceList = new vector<RemoteAardvark*>();
	detectDevices();
}



PluginAardvark::~PluginAardvark()
{
	pthread_mutex_destroy(&dLmutex);
	delete json;

	//delete deviceList;

	delete test;

	if(result != NULL)
		delete result;
}


RemoteAardvark* PluginAardvark::getDevice(int handle)
{
	RemoteAardvark* device;
	pthread_mutex_lock(&dLmutex);
	device = (*deviceList)[handle];
	pthread_mutex_unlock(&dLmutex);
	return device;
}


void PluginAardvark::detectDevices()
{
	u16 devices[EXPECTED_NUM_OF_DEVICES];
	u32 unique_ids[EXPECTED_NUM_OF_DEVICES];

	pthread_mutex_lock(&dLmutex);

	aa_find_devices_ext(EXPECTED_NUM_OF_DEVICES, devices, EXPECTED_NUM_OF_DEVICES, unique_ids);


	for(int i = 0; i < EXPECTED_NUM_OF_DEVICES; i++)
	{
		if(unique_ids[i] != 0)
		{
			//devices will be deleted within destructor
			//deviceList->push_back(new RemoteAardvark());
			test = new RemoteAardvark();
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

