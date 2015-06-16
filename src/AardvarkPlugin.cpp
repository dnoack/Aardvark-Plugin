#include "AardvarkPlugin.hpp"
#include "RemoteAardvark.hpp"
#include "AardvarkCareTaker.hpp"

AardvarkPlugin::AardvarkPlugin(PluginInfo* pluginInfo) : PluginInterface(pluginInfo)
{
	//get List of key, which are supported by the driver
	RemoteAardvark* tempDriver = new RemoteAardvark(0);
	list<string*>* functionList = tempDriver->getAllFunctionNames();
	delete tempDriver;

	StartAcceptThread();
	if(wait_for_accepter_up() != 0)
		throw Error("Creation of Listener/worker threads failed.");

	pluginActive = true;

	AardvarkCareTaker::init();
	regClient = new RegClient(pluginInfo, functionList, REG_PATH);
}

AardvarkPlugin::~AardvarkPlugin()
{
	delete regClient;
}


void AardvarkPlugin::thread_accept()
{
	int new_socket = 0;
	ComPoint* comPoint = NULL;
	AardvarkCareTaker* aardC = NULL;
	listen(connection_socket, MAX_CLIENTS);

	//dyn_print("Accepter created\n");
	while(true)
	{
		new_socket = accept(connection_socket, (struct sockaddr*)&address, &addrlen);
		if(new_socket > 0)
		{
			aardC = new AardvarkCareTaker();
			comPoint = new ComPoint(new_socket, aardC, pluginNumber);
			comPoint->configureLogInfo(&infoIn, &infoOut, &info);
			//dyn_print("Uds---> sNew UdsWorker with socket: %d \n", new_socket);
			pushComPointList(comPoint);
		}
	}
}


#ifndef TESTMODE
int main(int argc, const char** argv)
{
	PluginInfo* pluginInfo = new PluginInfo(PLUGIN_NAME, PLUGIN_NUMBER, COM_PATH);
	AardvarkPlugin* plugin = new AardvarkPlugin(pluginInfo);

	plugin->start();

	delete plugin;
	delete pluginInfo;
	return 0;
}

#endif

