#define private public
#define protected public

#include "AardvarkCareTaker.hpp"
#include "TestHarness.h"


static AardvarkCareTaker* aardC = NULL;
static AardvarkCareTaker* aardC2 = NULL;
static JsonRPC* json = NULL;
static IncomingMsg* request = NULL;


OutgoingMsg* openDevice(int contextNumber)
{
	Value method;
	Value params;
	Value id;
	Value* nullId = NULL;
	const char* message = NULL;
	OutgoingMsg* output = NULL;
	Document* requestDOM = json->getRequestDOM();

	//first we set our contextNumber because with the help of the contextNumber a device will be locked
	method.SetString("setIdentification", requestDOM->GetAllocator());
	params.SetObject();
	params.AddMember("contextNumber", contextNumber, requestDOM->GetAllocator());
	message = json->generateRequest(method, params, *nullId);
	request = new IncomingMsg(aardC->comPoint, message);
	aardC->processMsg(request);

	method.SetString("Aardvark.aa_open", requestDOM->GetAllocator());
	params.SetObject();
	params.AddMember("port", 0, requestDOM->GetAllocator());
	id.SetInt(1);
	message = json->generateRequest(method, params, id);
	request = new IncomingMsg(aardC->comPoint, message);
	return output = aardC->processMsg(request);
}

TEST_GROUP(AardvarkCareTaker)
{
	void setup()
	{
		aardC = new AardvarkCareTaker();
		aardC->init();
		json = new JsonRPC();

	}

	void teardown()
	{
		delete aardC;
		delete json;
		aardC->deInit();
	}

};


TEST_GROUP(Concurent_Access)
{
	void setup()
	{
		aardC = new AardvarkCareTaker();
		aardC2 = new AardvarkCareTaker();
		aardC->init();
		json = new JsonRPC();
	}

	void teardown()
	{

		delete aardC;
		delete aardC2;
		delete json;
		aardC->deInit();
	}
};


TEST(Concurent_Access, tryToAccesDeviceInUse)
{
	OutgoingMsg* output = openDevice(1);
	STRCMP_EQUAL("{\"jsonrpc\":\"2.0\",\"result\":{\"Aardvark\":1},\"id\":1}", output->getContent()->c_str());
	delete output;
	output = openDevice(2);
	STRCMP_EQUAL("{\"jsonrpc\":\"2.0\",\"error\":{\"code\":-99998,\"message\":\"Server error\",\"data\":\"Another user is using the requested hardware.\"},\"id\":1}", output->getContent()->c_str());
	delete output;
}


TEST(AardvarkCareTaker, makeARequest)
{
	OutgoingMsg* output = openDevice(1);
	STRCMP_EQUAL("{\"jsonrpc\":\"2.0\",\"result\":{\"Aardvark\":1},\"id\":1}", output->getContent()->c_str());
	delete output;
}



TEST(AardvarkCareTaker, memoryTest)
{

}
