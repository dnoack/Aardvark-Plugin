/*
 * UdsClient.cpp
 *
 *  Created on: 11.02.2015
 *      Author: dnoack
 */


#include "UdsRegClient.hpp"
#include "errno.h"

struct sockaddr_un UdsRegClient::address;
socklen_t UdsRegClient::addrlen;


UdsRegClient::UdsRegClient(const char* UDS_FILE_PATH, int size)
{
	optionflag = 1;
	ready = false;
	currentSocket = socket(AF_UNIX, SOCK_STREAM, 0);
	address.sun_family = AF_UNIX;
	strncpy(address.sun_path, UDS_FILE_PATH, size);
	addrlen = sizeof(address);

	json = new JsonRPC();
	regWorker = new UdsRegWorker(currentSocket);
}



UdsRegClient::~UdsRegClient()
{
	delete json;
	delete regWorker;
}



bool UdsRegClient::connectToRSD()
{
	Value method;
	Value params;
	Value id;
	Document dom;
	char* msg;
	int status = 0;
	bool result = false;

	//maybe connect will be better separated ?
	status = connect(currentSocket, (struct sockaddr*)&address, addrlen);

	if(status != -1)
	{
		//send a json rpc which signals "hey rsd, I want to register this plugin"
		method.SetString("announce");
		params.SetObject();
		params.AddMember("pluginName", "Aardvark", dom.GetAllocator());
		params.AddMember("udsFilePath", "/tmp/AardvarkPlugin.uds", dom.GetAllocator());
		id.SetInt(1);

		msg = json->generateRequest(method, params, id);

		status = send(currentSocket, msg , strlen(msg) ,0);
		if(status == -1)
		{
			printf("Fehler beim senden.\n");
			result = false;
			delete regWorker;
		}
		else
		{
			result = true;
		}
	}
	else
	{
		printf("Fehler beim Verbinden zu RSD.\n");
		result = false;
		delete regWorker;

	}
	return result;
}



void UdsRegClient::unregisterFromRSD()
{
	//send a json rpc whisch tells the RSD that the plugin is going to shutdown

}



int UdsRegClient::sendData(string* data)
{
	return send(currentSocket, data->c_str(), data->size(), 0);
}


