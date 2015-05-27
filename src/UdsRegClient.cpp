#include "UdsRegClient.hpp"


struct sockaddr_un UdsRegClient::address;
socklen_t UdsRegClient::addrlen;


UdsRegClient::UdsRegClient(const char* pluginName, int pluginNumber, const char* regPath, const char* comPath)
{
	this->regPath = regPath;
	regWorker = NULL;
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


UdsRegClient::~UdsRegClient()
{
	if(regWorker != NULL)
		delete regWorker;

	delete plugin;
	delete json;
}


void UdsRegClient::connectToRSD()
{
	if( connect(connection_socket, (struct sockaddr*)&address, addrlen) != 0 )
		throw Error(-1101, "Could not connect to RSD.\n");
	else
		regWorker = new UdsRegWorker(this, connection_socket);
}


void UdsRegClient::unregisterFromRSD()
{
	//TODO: send a json rpc which tells the RSD that the plugin is going to shutdown
}


void UdsRegClient::processRegistration(string* msg)
{

	const char* response = NULL;

	try
	{
		dom = json->parse(msg);
		currentMsgId = json->tryTogetId(dom);

		if(json->isError(dom))
			throw Error(-1102, "Received json rpc error response.");

		switch(state)
		{
			case NOT_ACTIVE:
				//check for announce ack then switch state to announced
				//and send register msg
				if(handleAnnounceACKMsg(msg))
				{
					state = ANNOUNCED;
					response = createRegisterMsg();
					regWorker->transmit(response, strlen(response));
				}
				break;
			case ANNOUNCED:
				if(handleRegisterACKMsg(msg))
				{
					state = REGISTERED;
					//TODO: check if Plugin com part is ready, if yes -> state = active
					//create pluginActive msg
					response = createPluginActiveMsg();
					regWorker->transmit(response, strlen(response));
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


void UdsRegClient::sendAnnounceMsg()
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
		regWorker->transmit(announceMsg, strlen(announceMsg));
	}
	catch(Error &e)
	{
		throw;
	}
}



bool UdsRegClient::handleAnnounceACKMsg(string* msg)
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


const char* UdsRegClient::createRegisterMsg()
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


bool UdsRegClient::handleRegisterACKMsg(string* msg)
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


const char* UdsRegClient::createPluginActiveMsg()
{
	Value method;
	Value* params = NULL;
	Value* id = NULL;
	const char* msg = NULL;

	method.SetString("pluginActive");
	msg = json->generateRequest(method, *params, *id);

	return msg;
}

