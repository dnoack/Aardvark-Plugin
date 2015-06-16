#include "AardvarkPlugin.hpp"



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
	list<string*>* functionList = tempDriver->getAllFunctionNames();
	delete tempDriver;

	AardvarkCareTaker::init();

	regClient = new RegClient(new Plugin(PLUGIN_NAME, PLUGIN_NUMBER, COM_PATH), functionList, REG_PATH);
	comServer = new ComServer(COM_PATH, sizeof(COM_PATH), PLUGIN_NUMBER);
}


AardvarkPlugin::~AardvarkPlugin()
{
	AardvarkCareTaker::deInit();
	delete comServer;
	delete regClient;
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


#ifndef TESTMODE
int main(int argc, const char** argv)
{

	AardvarkPlugin* plugin = new AardvarkPlugin();
	plugin->start();
	delete plugin;
	return 0;
}

#endif
