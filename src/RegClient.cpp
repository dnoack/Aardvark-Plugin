#include "RegClient.hpp"


struct sockaddr_un RegClient::address;
socklen_t RegClient::addrlen;


RegClient::RegClient(const char* pluginName, int pluginNumber, const char* regPath, const char* comPath)
{
	this->regPath = regPath;
	dom = NULL;
	workerInterface = NULL;
	optionflag = 1;
	currentMsgId = NULL;
	state = NOT_ACTIVE;
	address.sun_family = AF_UNIX;
	strncpy(address.sun_path, regPath, strlen(regPath));
	addrlen = sizeof(address);

	connection_socket = socket(AF_UNIX, SOCK_STREAM, 0);

	if(connection_socket < 0)
		throw Error(-1100, "Could not create connection_socket");


	plugin = new Plugin(pluginName, pluginNumber ,comPath);
	json = new JsonRPC();
}


RegClient::~RegClient()
{
	if(workerInterface != NULL)
		delete workerInterface;

	delete plugin;
	delete json;
}


void RegClient::connectToRSD()
{
	if( connect(connection_socket, (struct sockaddr*)&address, addrlen) != 0 )
		throw Error(-1101, "Could not connect to RSD.\n");
	else
		workerInterface = new ComPoint(connection_socket, this, plugin->getPluginNumber());
}


void RegClient::unregisterFromRSD()
{
	//TODO: send a json rpc which tells the RSD that the plugin is going to shutdown
}


void RegClient::process(RPCMsg* msg)
{

	const char* response = NULL;

	try
	{
		dom = new Document();
		json->parse(dom, msg->getContent());
		currentMsgId = json->tryTogetId(dom);

		if(json->isError(dom))
			throw Error(-1102, "Received json rpc error response.");

		switch(state)
		{
			case NOT_ACTIVE:
				//check for announce ack then switch state to announced
				//and send register msg
				if(handleAnnounceACKMsg())
				{
					state = ANNOUNCED;
					response = createRegisterMsg();
					workerInterface->transmit(response, strlen(response));
				}
				break;
			case ANNOUNCED:
				if(handleRegisterACKMsg())
				{
					state = REGISTERED;
					//TODO: check if Plugin com part is ready, if yes -> state = active
					//create pluginActive msg
					response = createPluginActiveMsg();
					workerInterface->transmit(response, strlen(response));
				}
				//check for register ack then switch state to active
				break;
			case ACTIVE:
				//maybe heartbeat check
				break;
			case BROKEN:
				//clean up an set state to NOT_ACTIVE
				state = NOT_ACTIVE;
				break;
			default:
				//something went completely wrong
				state = BROKEN;
				break;
		}
		delete msg;
	}
	catch(Error &e)
	{
		delete msg;
		state = BROKEN;
		close(connection_socket);
	}
}


void RegClient::sendAnnounceMsg()
{
	Value method;
	Value params;
	Value id;
	const char* announceMsg = NULL;
	Document* dom = json->getRequestDOM();

	try
	{
		method.SetString("announce");
		params.SetObject();
		params.AddMember("pluginName", StringRef(plugin->getName()->c_str()), dom->GetAllocator());
		params.AddMember("pluginNumber", plugin->getPluginNumber(), dom->GetAllocator());
		params.AddMember("udsFilePath", StringRef(plugin->getUdsFilePath()->c_str()), dom->GetAllocator());
		id.SetInt(1);

		announceMsg = json->generateRequest(method, params, id);
		workerInterface->transmit(announceMsg, strlen(announceMsg));
	}
	catch(Error &e)
	{
		throw;
	}
}



bool RegClient::handleAnnounceACKMsg()
{
	Value* resultValue = NULL;
	const char* resultString = NULL;
	bool result = false;

	try
	{
		resultValue = json->tryTogetResult(dom);
		if(resultValue->IsString())
		{
			resultString = resultValue->GetString();
			if(strcmp(resultString, "announceACK") == 0)
				result = true;
		}
		else
		{
			throw Error(-1103, "Awaited announceACK.");
		}
	}
	catch(Error &e)
	{
		throw;
	}
	return result;
}


const char* RegClient::createRegisterMsg()
{
	Document dom;
	Value method;
	Value params;
	Value functionArray;


	list<string*>* funcList;
	string* functionName;


	//get methods from plugin
	funcList = AardvarkPlugin::getFuncList();
	method.SetString("register");
	params.SetObject();
	functionArray.SetArray();

	for(list<string*>::iterator ifName = funcList->begin(); ifName != funcList->end(); )
	{
		functionName = *ifName;
		functionArray.PushBack(StringRef(functionName->c_str()), dom.GetAllocator());
		ifName = funcList->erase(ifName);
	}

	params.AddMember("functions", functionArray, dom.GetAllocator());

	return json->generateRequest(method, params, *currentMsgId);
}


bool RegClient::handleRegisterACKMsg()
{
	const char* resultString = NULL;
	Value* resultValue = NULL;
	bool result = false;

	try
	{
		resultValue = json->tryTogetResult(dom);
		if(resultValue->IsString())
		{
			resultString = resultValue->GetString();
			if(strcmp(resultString, "registerACK") == 0)
				result = true;
		}
		else
		{
			throw Error(-1104, "Awaited registerACK.");
		}
	}
	catch(Error &e)
	{
		throw;
	}
	return result;
}


const char* RegClient::createPluginActiveMsg()
{
	Value method;
	Value* params = NULL;
	Value* id = NULL;
	const char* msg = NULL;

	method.SetString("pluginActive");
	msg = json->generateRequest(method, *params, *id);

	return msg;
}

