/*
 * AardvarkPlugin.cpp
 *
 *  Created on: 25.02.2015
 *      Author: Dave
 */


#include <AardvarkPlugin.hpp>
#include "UdsServer.hpp"
#include "RemoteAardvark.hpp"
#include "AardvarkCareTaker.hpp"


list<string*>* AardvarkPlugin::funcList;


AardvarkPlugin::AardvarkPlugin()
{

	regClientReady = false;
	comServerReady = false;
	pluginActive = true;

	sigemptyset(&origmask);
	sigemptyset(&sigmask);
	sigaddset(&sigmask, SIGUSR1);
	sigaddset(&sigmask, SIGUSR2);
	pthread_sigmask(SIG_BLOCK, &sigmask, &origmask);

	//get List of key, which are supported by the driver
	RemoteAardvark* tempDriver = new RemoteAardvark(0);
	funcList = tempDriver->getAllFunctionNames();
	delete tempDriver;

	AardvarkCareTaker::init();

	regClient = new UdsRegClient(PLUGIN_NAME, PLUGIN_NUMBER, REG_PATH, sizeof(REG_PATH), COM_PATH);
	comServer = new UdsServer(COM_PATH, sizeof(COM_PATH));
}


AardvarkPlugin::~AardvarkPlugin()
{
	AardvarkCareTaker::deInit();
	delete comServer;
	delete regClient;
	deleteFuncList();
}


void AardvarkPlugin::start()
{
	try
	{
		regClient->connectToRSD();
		regClient->registerToRSD();
		pluginActive = true;
		while(pluginActive)
		{
			sleep(3);
			comServer->checkForDeletableWorker();
		}
	}
	catch(Error &e)
	{
		printf("%s \n", e.get());
	}
}


void AardvarkPlugin::deleteFuncList()
{
	list<string*>::iterator i = funcList->begin();
	while(i != funcList->end())
	{
		delete *i;
		i = funcList->erase(i);
	}
}


#ifndef TESTMODE
int main(int argc, const char** argv)
{

	AardvarkPlugin* plugin = new AardvarkPlugin();
	plugin->start();
	delete plugin;
	return 0;
}

#endif
