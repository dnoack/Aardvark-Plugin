/*
 * AardvarkPlugin.cpp
 *
 *  Created on: 25.02.2015
 *      Author: Dave
 */


#include <AardvarkPlugin.hpp>
#include "UdsServer.hpp"
#include "RemoteAardvark.hpp"


list<string*>* AardvarkPlugin::funcList;


AardvarkPlugin::AardvarkPlugin()
{

	regClientReady = false;
	comServerReady = false;
	pluginActive = true;

	//get List of key, which are supported by the driver
	RemoteAardvark* tempDriver = new RemoteAardvark(0);
	funcList = tempDriver->getAllFunctionNames();
	delete tempDriver;

	regClient = new UdsRegClient(PLUGIN_NAME, PLUGIN_NUMBER, REG_PATH, sizeof(REG_PATH), COM_PATH);
	comServer = new UdsServer(COM_PATH, sizeof(COM_PATH));

}



AardvarkPlugin::~AardvarkPlugin()
{
	delete comServer;
	delete regClient;
}




void AardvarkPlugin::startCommunication()
{
	pluginActive = regClient->connectToRSD();


	while(pluginActive)
	{
		sleep(3);
		comServer->checkForDeletableWorker();
	}

}


#ifndef TESTMODE

int main(int argc, const char** argv)
{

	AardvarkPlugin* plugin = new AardvarkPlugin();
	plugin->startCommunication();
	delete plugin;
	return 0;
}

#endif
