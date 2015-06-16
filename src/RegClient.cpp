#include "RegClient.hpp"


struct sockaddr_un RegClient::address;
socklen_t RegClient::addrlen;


RegClient::RegClient(Plugin* plugin, list<string*>* functionList, const char* regPath)
{
	this->regPath = regPath;
	this->plugin = plugin;
	this->functionList = functionList;
	globalDom = NULL;
	comPoint = NULL;
	optionflag = 1;
	currentMsgId = NULL;
	state = NOT_ACTIVE;
	address.sun_family = AF_UNIX;
	globalDom = new Document();

	strncpy(address.sun_path, regPath, strlen(regPath));
	addrlen = sizeof(address);

	connection_socket = socket(AF_UNIX, SOCK_STREAM, 0);

	if(connection_socket < 0)
		throw Error(-1100, "Could not create connection_socket");


	json = new JsonRPC();
}


RegClient::~RegClient()
{
	if(comPoint != NULL)
		delete comPoint;

	deleteFunctionList();
	delete plugin;
	delete json;
	delete globalDom;
}


void RegClient::connectToRSD()
{
	if( connect(connection_socket, (struct sockaddr*)&address, addrlen) != 0 )
		throw Error(-1101, "Could not connect to RSD.\n");
	else
		comPoint = new ComPoint(connection_socket, this, plugin->getPluginNumber());
}


void RegClient::unregisterFromRSD()
{
	//TODO: send a json rpc which tells the RSD that the plugin is going to shutdown
}


OutgoingMsg* RegClient::process(IncomingMsg* input)
{
	OutgoingMsg* output = NULL;
	try
	{
		json->parse(globalDom, input->getContent());
		currentMsgId = json->tryTogetId(globalDom);

		if(json->isError(globalDom))
			throw Error(-1102, "Received json rpc error response.");

		switch(state)
		{
			case NOT_ACTIVE:
				//check for announce ack then switch state to announced
				//and send register msg
				if(handleAnnounceACKMsg())
				{
					state = ANNOUNCED;
					output = createRegisterMsg(input);
				}
				break;
			case ANNOUNCED:
				if(handleRegisterACKMsg())
				{
					state = REGISTERED;
					output = createPluginActiveMsg(input);
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
	}
	catch(Error &e)
	{
		state = BROKEN;
		close(connection_socket);
	}
	delete input;

	return output;
}


void RegClient::sendAnnounceMsg()
{
	Value method;
	Value params;
	Value id;
	const char* announceMsg = NULL;
	Document* requestDOM = json->getRequestDOM();

	try
	{
		method.SetString("announce");
		params.SetObject();
		params.AddMember("pluginName", StringRef(plugin->getName()->c_str()), requestDOM->GetAllocator());
		params.AddMember("pluginNumber", plugin->getPluginNumber(), requestDOM->GetAllocator());
		params.AddMember("udsFilePath", StringRef(plugin->getUdsFilePath()->c_str()), requestDOM->GetAllocator());
		id.SetInt(1);

		announceMsg = json->generateRequest(method, params, id);
		comPoint->transmit(announceMsg, strlen(announceMsg));
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
		resultValue = json->tryTogetResult(globalDom);
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


OutgoingMsg* RegClient::createRegisterMsg(IncomingMsg* input)
{
	Value method;
	Value params;
	Value functionArray;

	string* functionName;
	Document* requestDOM = json->getRequestDOM();
	OutgoingMsg* output = NULL;
	const char* request = NULL;
	list<string*>::iterator function = functionList->begin();


	method.SetString("register");
	params.SetObject();
	functionArray.SetArray();

	while(function != functionList->end())
	{
		functionName = *function;
		functionArray.PushBack(StringRef(functionName->c_str()), requestDOM->GetAllocator());
		function = functionList->erase(function);
	}


	params.AddMember("functions", functionArray, requestDOM->GetAllocator());
	request = json->generateRequest(method, params, *currentMsgId);
	output = new OutgoingMsg(input->getOrigin(), request);
	return output;
}


bool RegClient::handleRegisterACKMsg()
{
	const char* resultString = NULL;
	Value* resultValue = NULL;
	bool result = false;

	try
	{
		resultValue = json->tryTogetResult(globalDom);
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


OutgoingMsg* RegClient::createPluginActiveMsg(IncomingMsg* input)
{
	Value method;
	Value* params = NULL;
	Value* id = NULL;
	const char* request = NULL;
	OutgoingMsg* output = NULL;

	method.SetString("pluginActive");
	request = json->generateRequest(method, *params, *id);
	output = new OutgoingMsg(input->getOrigin(), request);

	return output;
}

void RegClient::deleteFunctionList()
{
	list<string*>::iterator function = functionList->begin();
	while(function != functionList->end())
	{
		delete *function;
		function = functionList->erase(function);
	}
	delete functionList;
}

