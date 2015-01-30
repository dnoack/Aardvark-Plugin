/*
 * JsonRPC.cpp
 *
 *  Created on: 16.01.2015
 *      Author: dnoack
 */

#include <JsonRPC.hpp>
#include "Plugin_Interface.h"
#include "Aardvark.hpp"
#include "stdio.h"


using namespace Plugin;


char* JsonRPC::handle(string* request, string* identity)
{

	requestDOM->Parse(request->c_str());


	if(!requestDOM->HasParseError())
	{
		try
		{
			if(checkJsonRpcFormat(requestDOM))
			{
				if(requestDOM->HasMember("id")) //request
					return process((*requestDOM)["method"], (*requestDOM)["params"], (*requestDOM)["id"], identity);
				else
					return NULL; //TODO: notification
			}
		}
		catch(char* e)
		{
			return e;
		}
	}
	else
		return "Parsing error";

}


bool JsonRPC::checkJsonRpcFormat(Document* dom)
{
	if(!requestDOM->HasMember("jsonrpc"))
	{
		throw "Inccoret Json RPC, member \"jsonrpc\" is missing.";
	}
	else
	{
		checkJsonRpcVersion(dom);
		if(!requestDOM->HasMember("method"))
			throw "Inccoret Json RPC, member \"method\" is missing.";
		else
			if(!requestDOM->HasMember("params"))
				throw "Inccoret Json RPC, member \"params\" is missing.";
	}
	return true;
}


bool JsonRPC::checkJsonRpcVersion(Document* dom)
{


	if((*dom)["jsonrpc"].IsString())
	{
		if(strcmp((*dom)["jsonrpc"].GetString(), "2.0") != 0)
			throw "Inccorect jsonrpc version. Used version is 2.0";
	}
	else
		throw "Member \"jsonrpc\" has to be a string.";

	return true;
}




char* JsonRPC::process(Value &method, Value &params, Value &id, string* identity)
{
	string* deviceIdentity = NULL;
	RemoteAardvark* currentDevice = NULL;
	char* responseMsg = NULL;
	bool lockFlag= false;
	int devicePort = 0;

	//lookup the function !
	printf("Funktionaufruf: %s ", method.GetString());

	/*
	//identify hw, for aardvark we can identify the device by it s port_number (first parameter)
	if(params.IsArray())
	{
		devicePort = params[0].GetInt();
	}
	else
		devicePort = params.GetInt();

	//TODO: we need a better way to distinguish hw port and hw handle
	if(strcmp(method.GetString(), "aa_open") == 0)
		currentDevice = deviceList[devicePort];
	else
		currentDevice = deviceList[devicePort-1];*/

	currentDevice = deviceList[0];
	//process the function and create result
	deviceIdentity = currentDevice->getIdentity();

	//Hardware is free or it s the same user
	if(deviceIdentity == NULL || deviceIdentity->compare(*identity) == 0)
	{

		if(deviceIdentity == NULL)
			currentDevice->setIdentity(identity);

		try{

			lockFlag = currentDevice->executeFunction(method, params, result);

			if(!lockFlag)
				currentDevice->setIdentity(NULL);

			//generate a normal responsemsg with result
			responseMsg = response(id);

		}
		catch(const PluginError &e)
		{
			responseMsg = responseError(id, -32000, e.get());
		}
	}
	else
	{
		responseMsg = responseError(id, -32000, "Hardware locked, Usage restrictedt.\n");
	}
	return responseMsg;
}



char* JsonRPC::response(Value &id)
{
	//clear buffer
	Value* oldResult;
	sBuffer.Clear();
	jsonWriter->Reset(sBuffer);

	//swap current result value with the old one and get the corresponding id
	oldResult = &((*responseDOM)["result"]);
	oldResult->Swap(result);
	(*responseDOM)["id"] = id.GetInt();

	//write DOM to sBuffer
	responseDOM->Accept(*jsonWriter);
	printf("ResponseMsg: %s\n", sBuffer.GetString());

	return (char*)sBuffer.GetString();
}


char* JsonRPC::responseError(Value &id, int code, char* msg)
{
	Value data;

	data.SetString(msg, errorDOM->GetAllocator());
	delete[] msg;

	sBuffer.Clear();
	jsonWriter->Reset(sBuffer);

	(*errorDOM)["id"] = id.GetInt();
	(*errorDOM)["error"]["code"] = -32000;
	(*errorDOM)["error"]["message"] = "Server error";
	(*errorDOM)["error"]["data"].Swap(data);

	errorDOM->Accept(*jsonWriter);
	printf("ErrorMsg: %s\n", sBuffer.GetString());

	return (char*)sBuffer.GetString();
}


void JsonRPC::generateResponseDOM(Document &dom)
{
	Value id;
	id.SetInt(0);

	dom.SetObject();
	dom.AddMember("jsonrpc", JSON_PROTOCOL_VERSION, dom.GetAllocator());
	dom.AddMember("result", "", dom.GetAllocator());
	dom.AddMember("id", id, dom.GetAllocator());

}


void JsonRPC::generateErrorDOM(Document &dom)
{
	Value id;
	Value error;

	//An error msg is a response with error value but without result value, where error is an object
	dom.SetObject();
	dom.AddMember("jsonrpc", JSON_PROTOCOL_VERSION, dom.GetAllocator());


	error.SetObject();
	error.AddMember("code", 0, dom.GetAllocator());
	error.AddMember("message", "", dom.GetAllocator());
	error.AddMember("data", "", dom.GetAllocator());
	dom.AddMember("error", error, dom.GetAllocator());

	id.SetInt(0);
	dom.AddMember("id", id, dom.GetAllocator());

}


void JsonRPC::detectDevices()
{
	int num_devices = 1;
	u16 devices[num_devices];
	int num_ids = num_devices;
	u32 unique_ids[num_devices];

	aa_find_devices_ext(num_devices, devices, num_ids, unique_ids);

	for(int i = 0; i < num_devices; i++)
	{
		if(unique_ids[i] != 0)
		{
			//printf("Device found: Index: %u UniqueID: %u\n", (unsigned int)devices[i], (unsigned int)unique_ids[i]);
			deviceList.push_back(new RemoteAardvark());
		}
	}

}






