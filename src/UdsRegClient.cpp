#include "UdsRegClient.hpp"
#include "errno.h"

struct sockaddr_un UdsRegClient::address;
socklen_t UdsRegClient::addrlen;


UdsRegClient::UdsRegClient(const char* pluginName, int pluginNumber, const char* regPath, int size, const char* comPath)
{
	this->regWorker = NULL;
	this->pluginName = pluginName;
	this->pluginNumber = pluginNumber;
	this->pluginPath = comPath;
	optionflag = 1;

	ready = false;
	currentSocket = socket(AF_UNIX, SOCK_STREAM, 0);
	address.sun_family = AF_UNIX;
	strncpy(address.sun_path, regPath, size);
	addrlen = sizeof(address);
}


UdsRegClient::~UdsRegClient()
{
	if(regWorker != NULL)
		delete regWorker;
}


void UdsRegClient::connectToRSD()
{
	int status = connect(currentSocket, (struct sockaddr*)&address, addrlen);

	if(status > -1)
		regWorker = new UdsRegWorker(currentSocket);
	else
		throw Error("Fehler beim Verbinden zu RSD.\n");
}


void UdsRegClient::registerToRSD()
{
	regWorker->sendAnnounceMsg(pluginName, pluginNumber, pluginPath);
}


void UdsRegClient::unregisterFromRSD()
{
	//TODO: send a json rpc which tells the RSD that the plugin is going to shutdown
}

