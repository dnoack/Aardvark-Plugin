/*
 * test_JsonRPC.cpp
 *
 *  Created on: 30.01.2015
 *  Author: David Noack
 */

//getting access to private members for tests
#define private public

#include "JsonRPC.hpp"
#include "TestHarness.h"

static Document* dom;
static Plugin::JsonRPC* json;


static void generateRequest(Document &requestMsg)
{
	requestMsg.SetObject();
	requestMsg.AddMember("jsonrpc", "2.0", requestMsg.GetAllocator());
	requestMsg.AddMember("method", "func", requestMsg.GetAllocator());
	requestMsg.AddMember("params", "", requestMsg.GetAllocator());
	requestMsg["params"].SetObject();
	requestMsg["params"].AddMember("param1", 3005, requestMsg.GetAllocator());
	requestMsg.AddMember("id", 1234, requestMsg.GetAllocator());
}


TEST_GROUP(Plugin_JsonRPC)
{
	void setup()
	{
		json = new Plugin::JsonRPC();
		dom = new Document();
		dom->SetObject();
	}

	void teardown()
	{
		delete json;
		delete dom;
	}
};



TEST(Plugin_JsonRPC, checkJsonRpcVersion_FAIL)
{
	dom->AddMember("jsonrpc", "3.2", dom->GetAllocator());
	CHECK_THROWS(char*, json->checkJsonRpcVersion(*dom));
}



TEST(Plugin_JsonRPC, checkJsonRpcVersionOK)
{
	dom->AddMember("jsonrpc", "2.0", dom->GetAllocator());
	CHECK(json->checkJsonRpcVersion(*dom));
}


TEST(Plugin_JsonRPC, checkJsonRpc_RequestFormat_OK)
{
	dom->AddMember("jsonrpc", "2.0", dom->GetAllocator());
	dom->AddMember("method" , "func", dom->GetAllocator());
	CHECK(json->checkJsonRpc_RequestFormat(*dom));
}


TEST(Plugin_JsonRPC, checkJsonRpc_RequestFormat_FAIL)
{
	dom->AddMember("jsonrpc", "2.0", dom->GetAllocator());
	dom->AddMember("NOTaMethod", "foo", dom->GetAllocator());
	CHECK_THROWS(char*, json->checkJsonRpc_RequestFormat(*dom));
}


TEST(Plugin_JsonRPC, isRequest_no)
{
	dom->AddMember("id", "5", dom->GetAllocator());
	CHECK(json->isRequest(*dom));
	CHECK(json->isRequest(*dom));
}


TEST(Plugin_JsonRPC, responseDOM_ok)
{
	Value* id;
	Document* temp;

	//see begin of this file
	generateRequest(*json->requestDOM);

	temp = (json->requestDOM);
	id = &(*temp)["id"];
	json->response(*id);

	CHECK(json->responseDOM->HasMember("jsonrpc"));
	CHECK(json->responseDOM->HasMember("result"));
	LONGS_EQUAL((*json->responseDOM)["id"].GetInt(), 1234);
}


TEST(Plugin_JsonRPC, responseError_ok)
{
	Value* id;
	Value* error;
	Document* tempRequest;
	Document* tempResponse;
	char* message = "Test";


	//see begin of this file
	generateRequest(*json->requestDOM);

	tempRequest = (json->requestDOM);
	id = &(*tempRequest)["id"];

	json->responseError(*id, -32000, message);

	tempResponse = json->errorDOM;
	CHECK(tempResponse->HasMember("jsonrpc"));
	CHECK_EQUAL(false, tempResponse->HasMember("result"));

	CHECK(tempResponse->HasMember("error"));

	error = &(*tempResponse)["error"];
	CHECK(error->HasMember("message"));
	CHECK(error->HasMember("data"));

	LONGS_EQUAL((*json->errorDOM)["id"].GetInt(), 1234);

}





