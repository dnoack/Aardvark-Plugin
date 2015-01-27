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
	requestDOM.Parse(request->c_str());


	if(!requestDOM.HasParseError())
	{
		currentValue = requestDOM["jsonrpc"];
		if(strcmp(currentValue.GetString(), "2.0") == 0)
		{
			return process(requestDOM["method"], requestDOM["params"], requestDOM["id"], identity);
		}
		else return NULL;
	}
	else
		return NULL;//TODO: error handling
}




char* JsonRPC::process(Value &method, Value &params, Value &id, string* identity)
{
	string* deviceIdentity;
	RemoteAardvark* currentDevice;
	bool lockFlag= false;
	int devicePort;

	//lookup the function !
	printf("Funktionaufruf: %s ", method.GetString());

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
		currentDevice = deviceList[devicePort-1];

	//process the function and create result
	deviceIdentity = currentDevice->getIdentity();

	//Hardware is free or it s the same user
	if(deviceIdentity == NULL || deviceIdentity->compare(*identity) == 0)
	{

		if(deviceIdentity == NULL)
			currentDevice->setIdentity(identity);


		lockFlag = currentDevice->executeFunction(method, params, result);


		if(!lockFlag)
			currentDevice->setIdentity(NULL);
		//send the result back to RSD
		return response(id);
	}
	else
	{
		printf("Hardware locked, Usage restrictedt.\n");
		return responseError(id, 0, "Hardware gesperrt");
	}
}



char* JsonRPC::response(Value &id)
{
	//clear buffer
	Value* oldResult;
	sBuffer.Clear();
	jsonWriter->Reset(sBuffer);

	//swap current result value with the old one and get the corresponding id
	oldResult = &(responseDOM["result"]);
	oldResult->Swap(result);
	responseDOM["id"] = id.GetInt();

	//write DOM to sBuffer
	responseDOM.Accept(*jsonWriter);
	printf("ResponseMsg: %s\n", sBuffer.GetString());

	return (char*)sBuffer.GetString();
}


char* JsonRPC::responseError(Value &id, int code, char* msg)
{
	Value data;

	data.SetString(msg, errorDOM.GetAllocator());

	sBuffer.Clear();
	jsonWriter->Reset(sBuffer);

	errorDOM["id"] = id.GetInt();
	errorDOM["error"]["code"] = -32000;
	errorDOM["error"]["message"] = "Server error";
	errorDOM["error"]["data"].Swap(data);

	errorDOM.Accept(*jsonWriter);
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
	Value error;
	Value id;

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
			printf("Device found: Index: %u UniqueID: %u\n", (unsigned int)devices[i], (unsigned int)unique_ids[i]);
			deviceList.push_back(new RemoteAardvark());
		}
	}

}






