
#include "AardvarkPlugin.hpp"


list<string*>* AardvarkPlugin::funcList;
int LogUnit::globalLogLevel = 4;


AardvarkPlugin::AardvarkPlugin()
{
	pluginActive = true;
	logInfo.logName = "AardvarkPlugin: ";

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

	regClient = new RegClient(PLUGIN_NAME, PLUGIN_NUMBER, REG_PATH, COM_PATH);
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
		regClient->sendAnnounceMsg();

		pluginActive = true;
		while(pluginActive)
		{
			sleep(3);
			comServer->checkForDeletableWorker();
			if(regClient->isDeletable())
				pluginActive = false;
		}
	}
	catch(Error &e)
	{
		log(logInfo, e.get());
		pluginActive = false;
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
	delete funcList;
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
