/*
 * AardvarkPlugin.cpp
 *
 *  Created on: 25.02.2015
 *      Author: Dave
 */


#include <AardvarkPlugin.hpp>
#include "UdsServer.hpp"
#include "RemoteAardvark.hpp"


AardvarkPlugin::AardvarkPlugin()
{
	//get List of key, which are supported by the driver
	RemoteAardvark* tempDriver = new RemoteAardvark(0);
	funcList = tempDriver->getAllFunctionNames();
	delete tempDriver;

	comServer = new UdsServer(UDS_COM_PATH, sizeof(UDS_COM_PATH));
	regClient = new UdsRegClient(UDS_REGISTER_TO_RSD_PATH, sizeof(UDS_REGISTER_TO_RSD_PATH));
}



AardvarkPlugin::~AardvarkPlugin()
{
	delete comServer;
	delete regClient;
}




void AardvarkPlugin::startCommunication()
{
	comServer->startCom();
	regClient->connectToRSD();
}


#ifndef TESTMODE

int main(int argc, const char** argv)
{
	AardvarkPlugin* plugin = new AardvarkPlugin();
	plugin->startCommunication();

	while(1)
		sleep(3);


	return 0;
}

#endif
