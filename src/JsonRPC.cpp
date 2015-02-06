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
	char* responseMsg;


	try
	{
		requestDOM->Parse(request->c_str());

		if(!requestDOM->HasParseError())
		{

			if(checkJsonRpc_RequestFormat(*requestDOM))
			{
				if(isRequest(*requestDOM)) //request
					responseMsg =  processRequest((*requestDOM)["method"], (*requestDOM)["params"], (*requestDOM)["id"], identity);
				else
					responseMsg = NULL; //TODO: processNotification
			}
		}
		else
			throw PluginError("Error while parsing json rpc.");
	}
	catch(PluginError &errorMsg)
	{
		Value nullId;
		responseMsg = generateResponseError(nullId, -32700, errorMsg.get());

	}

	return responseMsg;
}


bool JsonRPC::checkJsonRpc_RequestFormat(Document &dom)
{
	if(!dom.HasMember("jsonrpc"))
	{
		throw PluginError("Inccorect Json RPC, member \"jsonrpc\" is missing.");
	}
	else
	{
		checkJsonRpcVersion(dom);
		if(!dom.HasMember("method"))
			throw PluginError("Inccorect Json RPC, member \"method\" is missing.");
	}
	return true;
}


bool JsonRPC::checkJsonRpcVersion(Document &dom)
{
	if(dom["jsonrpc"].IsString())
	{
		if(strcmp(dom["jsonrpc"].GetString(), JSON_PROTOCOL_VERSION) != 0)
			throw PluginError("Inccorect jsonrpc version. Used version is 2.0");
	}
	else
		throw PluginError("Member \"jsonrpc\" has to be a string.");

	return true;
}


bool JsonRPC::isRequest(Document &dom)
{
	if(dom.HasMember("id"))
	{
		//TODO: check: normally not NULL, no fractional pars
		return true;
	}
	else
		return false;
}



char* JsonRPC::processRequest(Value &method, Value &params, Value &id, string* identity)
{
	string* deviceIdentity = NULL;
	RemoteAardvark* currentDevice = NULL;
	char* responseMsg = NULL;
	bool lockFlag= false;
	//int devicePort = 0;

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
	try
	{
		if(deviceIdentity == NULL || deviceIdentity->compare(*identity) == 0)
		{

			if(deviceIdentity == NULL)
				currentDevice->setIdentity(identity);



				lockFlag = currentDevice->executeFunction(method, params, result);

				if(!lockFlag)
					currentDevice->setIdentity(NULL);

				//generate a normal responsemsg with result
				responseMsg = generateResponse(id);


		}
		else
		{
			throw PluginError("Hardware locked, Usage restricted.");
		}
	}
	catch(const PluginError &e)
	{
		lockFlag = false;
		responseMsg = generateResponseError(id, -32000, e.get());
	}

	return responseMsg;
}



char* JsonRPC::generateResponse(Value &id)
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
	printf("\nResponseMsg: %s\n", sBuffer.GetString());

	return (char*)sBuffer.GetString();
}


char* JsonRPC::generateResponseError(Value &id, int code, char* msg)
{
	Value data;

	data.SetString(msg, errorDOM->GetAllocator());
	sBuffer.Clear();
	jsonWriter->Reset(sBuffer);

	//e.g. parsing error, we have no id value.
	if(id.IsInt())
		(*errorDOM)["id"] = id.GetInt();
	else
		(*errorDOM)["id"] = 0;

	(*errorDOM)["error"]["code"] = code;
	(*errorDOM)["error"]["message"] = "Server error";
	(*errorDOM)["error"]["data"].Swap(data);

	errorDOM->Accept(*jsonWriter);
	printf("\nErrorMsg: %s\n", sBuffer.GetString());

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
			//devices will be deleted within destructor
			deviceList.push_back(new RemoteAardvark());
		}
	}
}



