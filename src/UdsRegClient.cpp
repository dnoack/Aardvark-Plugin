/*
 * UdsClient.cpp
 *
 *  Created on: 11.02.2015
 *      Author: dnoack
 */


#include "UdsRegClient.hpp"

struct sockaddr_un UdsRegClient::address;
socklen_t UdsRegClient::addrlen;


UdsRegClient::UdsRegClient(const char* UDS_FILE_PATH, int size)
{
	optionflag = 1;
	currentSocket = socket(AF_UNIX, SOCK_STREAM, 0);
	address.sun_family = AF_UNIX;
	strncpy(address.sun_path, UDS_FILE_PATH, size);
	addrlen = sizeof(address);

	regWorker = new UdsRegWorker(currentSocket);
}


UdsRegClient::~UdsRegClient()
{


}

void UdsRegClient::connectToRSD()
{
	connect(currentSocket, (struct sockaddr*)&address, addrlen);
	//send a json rpc which signals "hey rsd, i want to register this plugin
	send(currentSocket, "hallo RSD",9 ,0);
}


void UdsRegClient::registerToRSD()
{
	//send a json rpc which say "im plugin x and i have y methods [method1, method2, ..., methody]

}


void UdsRegClient::unregisterFromRSD()
{
	//send a json rpc whisch tells the RSD that the plugin is going to shutdown

}


int UdsRegClient::sendData(string* data)
{
	return send(currentSocket, data->c_str(), data->size(), 0);
}




