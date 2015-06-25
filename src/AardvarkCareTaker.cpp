#include "AardvarkCareTaker.hpp"
#include "ProcessInterface.hpp"


list<RemoteAardvark*> AardvarkCareTaker::deviceList;
pthread_mutex_t AardvarkCareTaker::dLmutex;


AardvarkCareTaker::AardvarkCareTaker()
{
	json = new JsonRPC();
	id = NULL;
	currentDom = NULL;
	contextNumber = 0;
	deviceLessFunctions = new RemoteAardvark(-1);
}


AardvarkCareTaker::~AardvarkCareTaker()
{
	delete json;
	unlockAllUsedDevices();
	delete deviceLessFunctions;
}


void AardvarkCareTaker::init()
{
	pthread_mutex_init(&dLmutex, NULL);
}


void AardvarkCareTaker::deInit()
{
	deleteDeviceList();
	pthread_mutex_destroy(&dLmutex);
}


RemoteAardvark* AardvarkCareTaker::getDevice(int value, int valueType)
{

	RemoteAardvark* device = NULL;
	bool found = false;

	pthread_mutex_lock(&dLmutex);
	list<RemoteAardvark*>::iterator tempDevice = deviceList.begin();

	while(tempDevice != deviceList.end() && !found)
	{
		if(valueType == HANDLE)
		{
			if((*tempDevice)->getHandle() == value)
			{
				device = *tempDevice;
				found = true;
			}
		}
		else if(valueType == PORT)
		{
			if((*tempDevice)->getPort() == value)
			{
				device = *tempDevice;
				found = true;
			}
		}
		++tempDevice;
	}

	//we found a device, but is no one else using it ?
	if(found)
	{
		if(device->getContextNumber() == 0 || device->getContextNumber() == contextNumber)
		{
			//dyn_print("Found device and got access to it. context: %d \n", contextNumber);
			if(device->getContextNumber() == 0)
				device->setContextNumber(contextNumber);
		}
		else
		{
			pthread_mutex_unlock(&dLmutex);
			//dyn_print("Requesting context %d.  using context: %d \n", contextNumber, device->getContextNumber());
			throw Error(-99998, "Another user is using the requested hardware.");
		}
	}
	else //didnt found the device
	{
		if(valueType == PORT) //create a new device instance with value as port
		{
			device = new RemoteAardvark(value);
			device->setContextNumber(contextNumber);
			deviceList.push_back(device);
		}
		else // cant create new devices with handle as value
		{
			pthread_mutex_unlock(&dLmutex);
			throw Error(-99999, "No device with this handle available.");
		}
	}

	pthread_mutex_unlock(&dLmutex);
	return device;
}





//main method for processing new json rpc msgs
OutgoingMsg* AardvarkCareTaker::process(IncomingMsg* input)
{

	Value responseValue;
	Value* params = NULL;
	Value* paramValue = NULL;
	RemoteAardvark* device = NULL;
	OutgoingMsg* output = NULL;
	const char* error = NULL;
	const char* result = NULL;

	currentDom = new Document();

	try
	{
		json->parse(currentDom, input->getContent());

		if(json->isRequest(currentDom))
		{
			id = json->getId(currentDom);
			params = json->tryTogetParams(currentDom);


			if(params->HasMember("port"))
			{
				device = getDevice((*params)["port"].GetInt(), PORT);

			}
			else if(params->HasMember("handle") )
			{
				device = getDevice((*params)["handle"].GetInt(), HANDLE);
			}
			else if(params->HasMember("Aardvark") )
			{
				device = getDevice((*params)["Aardvark"].GetInt(), HANDLE);
			}
			else
			{
				device = deviceLessFunctions;
			}
			device->executeFunction((*currentDom)["method"], *params, responseValue);
			result = json->generateResponse(*id, responseValue);
			output = new OutgoingMsg(input->getOrigin(), result);

		}
		else if(json->isNotification(currentDom))
		{
			paramValue = json->tryTogetParam(currentDom, "contextNumber");
			contextNumber = paramValue->GetInt();
			//dyn_print("Received Notification for context: %d", contextNumber);
		}
		else
		{
			throw Error("Aardvark-Plugin received: NO Request.\n");
		}
	}
	catch(Error &e)
	{
		error = json->generateResponseError(*id, e.getErrorCode(), e.get());
		output = new OutgoingMsg(input->getOrigin(), error);
	}
	delete currentDom;
	delete input;
	return output;
}


void AardvarkCareTaker::deleteDeviceList()
{
	pthread_mutex_lock(&dLmutex);
	list<RemoteAardvark*>::iterator device = deviceList.begin();

	while(device != deviceList.end())
	{
		delete *device;
		device = deviceList.erase(device);
	}
	pthread_mutex_unlock(&dLmutex);
}


void AardvarkCareTaker::unlockAllUsedDevices()
{
	pthread_mutex_lock(&dLmutex);
	list<RemoteAardvark*>::iterator device = deviceList.begin();

	while(device != deviceList.end())
	{
		if((*device)->getContextNumber() == contextNumber)
		{
			(*device)->setContextNumber(0);
			(*device)->close();
		}
		++device;
	}

	pthread_mutex_unlock(&dLmutex);
}
