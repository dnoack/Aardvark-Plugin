/*
 * test_JsonRPC.cpp
 *
 *  Created on: 30.01.2015
 *  Author: David Noack
 */

//getting access to private members for tests
#define private public

#include "JsonRPC.hpp"
#include "Error.hpp"
#include "TestHarness.h"

static Document* dom;
static JsonRPC* json;

static string OK_STRING =
		"{\"jsonrpc\": \"2.0\", \"params\": { \"handle\": 1}, \"method\": \"aa_close\", \"id\": 3}";

static string PARSE_ERROR_STRING =
		"{\"jsonrpc\": \"2.0\", \"params\": { \"handle\": 1 , \"method\": \"aa_close\", \"id\": 3}";

static string REQ_FROMAT_ERROR_STRING =
		"{\"jsonrpc\": \"2.0\", \"params\": { \"handle\": 1} , \"notAMethod\": \"aa_close\", \"id\": 3}";

static string REQUEST = OK_STRING;
static string RESPONSE =
		"{\"jsonrpc\": \"2.0\", \"result\": 3085, \"id\": 1}";

static string ERROR_RESPONSE =
		"{\"jsonrpc\": \"2.0\", \"error\": {\"code\" : -32000, \"message\": \"This is an error\"}, \"id\": 1}";

static string NOTIFICATION =
		"{\"jsonrpc\": \"2.0\", \"params\": { \"handle\": 1}, \"method\": \"aa_close\"}";

static string THREE_REQUESTS =
		"{\"jsonrpc\": \"2.0\", \"params\": { \"handle\": 1}, \"method\": \"aa_close\", \"id\": 1}{\"jsonrpc\": \"2.0\", \"params\": { \"handle\": 3005}, \"method\": \"blub\", \"id\": 2}{\"jsonrpc\": \"2.0\", \"params\": { \"handle\": 1}, \"method\": \"aa_open\", \"id\": 3}";

static string REQUESTS_WITH_ERROR =
		"{\"jsonrpc\": \"2.0\", \"params\": { \"handle\": 1}, \"method\": \"aa_close\", \"id\": 1}{\"jsonrpc\": \"2.0\", \"params\":  \"handle\": 3005}, \"method\": \"blub\", \"id\": 2}{\"jsonrpc\": \"2.0\", \"params\": { \"handle\": 1}, \"method\": \"aa_open\", \"id\": 3}";


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
		json = new JsonRPC();
		dom = new Document();
	}

	void teardown()
	{
		delete json;
		delete dom;
	}
};













TEST(Plugin_JsonRPC, responseDOM_ok)
{
	Value* id;
	Value response;
	Document* temp;

	//see begin of this file
	generateRequest(*json->requestDOM);

	temp = (json->requestDOM);
	id = &(*temp)["id"];
	json->generateResponse(*id, response);

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
	const char* message = "Test";


	//see begin of this file
	generateRequest(*json->requestDOM);

	tempRequest = (json->requestDOM);
	id = &(*tempRequest)["id"];

	json->generateResponseError(*id, -32000, message);

	tempResponse = json->errorDOM;
	CHECK(tempResponse->HasMember("jsonrpc"));
	CHECK_EQUAL(false, tempResponse->HasMember("result"));

	CHECK(tempResponse->HasMember("error"));

	error = &(*tempResponse)["error"];
	CHECK(error->HasMember("message"));
	CHECK(error->HasMember("data"));

	LONGS_EQUAL((*json->errorDOM)["id"].GetInt(), 1234);

}


TEST(Plugin_JsonRPC, hastResultOrError_FAIL)
{
	json->parse(dom, &REQUEST);
	CHECK_THROWS(Error, json->hasResultOrError(dom));
}


TEST(Plugin_JsonRPC, hasResultOrError_OK)
{
	json->parse(dom, &RESPONSE);
	CHECK(json->hasResultOrError(dom));
	json->parse(dom, &ERROR_RESPONSE);
	CHECK(json->hasResultOrError(dom));
}


TEST(Plugin_JsonRPC, hasError_FAIL)
{
	string errorNOTAvailable = "{\"jsonrpc\": \"2.0\", \"id\": 1}";
	json->parse(dom, &errorNOTAvailable);
	CHECK_THROWS(Error, json->hasError(dom));
}


TEST(Plugin_JsonRPC, hasError_OK)
{
	json->parse(dom, &ERROR_RESPONSE);
	CHECK(json->hasError(dom));
}


TEST(Plugin_JsonRPC, hasResult_FAIL)
{
	string resultNOTAvailable = "{\"jsonrpc\": \"2.0\", \"id\": 1}";
	json->parse(dom, &resultNOTAvailable);
	CHECK_THROWS(Error, json->hasResult(dom));
}


TEST(Plugin_JsonRPC, hasResult_OK)
{
	string resultAvailable = "{\"jsonrpc\": \"2.0\", \"result\" : \"hallo welt\", \"id\": 1}";
	json->parse(dom, &resultAvailable);
	CHECK(json->hasResult(dom));

}


TEST(Plugin_JsonRPC, hasNoMemberId_FAIL)
{
	json->parse(dom, &REQUEST);
	CHECK_THROWS(Error, json->hasNoId(dom));
}


TEST(Plugin_JsonRPC, hasNoMemberId_OK)
{
	json->parse(dom, &NOTIFICATION);
	CHECK(json->hasNoId(dom));
}


TEST(Plugin_JsonRPC, hasMemberId_FAIL)
{
	string idNOTAvailable = "{\"jsonrpc\": \"2.0\", \"params\": { \"handle\": 1}, \"method\": \"aa_close\"}";
	json->parse(dom, &idNOTAvailable);
	CHECK_THROWS(Error, json->hasId(dom));

}


TEST(Plugin_JsonRPC, hasMemberId_OK)
{
	string idAvailable = "{\"jsonrpc\": \"2.0\", \"params\": { \"handle\": 1}, \"method\": \"aa_close\", \"id\": 3}";
	json->parse(dom, &idAvailable);
	CHECK(json->hasId(dom));

}


TEST(Plugin_JsonRPC, hasParams_FAIL)
{
	string paramsNOTAvailable = "{\"jsonrpc\": \"2.0\", \"method\": \"aa_close\", \"id\": 1}";
	string paramsIsString = "{\"jsonrpc\": \"2.0\", \"params\":  \"handle : 1\", \"method\": \"aa_close\", \"id\": 1}";
	string paramsIsInt = "{\"jsonrpc\": \"2.0\", \"params\": 3425345, \"method\": \"aa_close\", \"id\": 1}";

	json->parse(dom, &paramsNOTAvailable);
	CHECK_THROWS(Error, json->hasParams(dom));

	json->parse(dom, &paramsIsString);
	CHECK_THROWS(Error, json->hasParams(dom));

	json->parse(dom, &paramsIsInt);
	CHECK_THROWS(Error, json->hasParams(dom));
}


TEST(Plugin_JsonRPC, hasParams_OK)
{
	string paramsAvailable = "{\"jsonrpc\": \"2.0\", \"params\": { \"handle\": 1}, \"method\": \"aa_close\", \"id\": 1}";
	json->parse(dom, &paramsAvailable);
	CHECK(json->hasParams(dom));
}


TEST(Plugin_JsonRPC, hasMemberMethod_FAIL)
{
	string methodNOTAvailable = "{\"jsonrpc\": \"2.0\", \"params\": { \"handle\": 1}, \"id\": 1}";
	string methodNOString = "{\"jsonrpc\": \"2.0\", \"params\": { \"handle\": 1}, \"method\": 4563456 , \"id\": 1}";
	json->parse(dom, &methodNOTAvailable);
	CHECK_THROWS(Error, json->hasMethod(dom));
	json->parse(dom, &methodNOString);
	CHECK_THROWS(Error, json->hasMethod(dom));
}


TEST(Plugin_JsonRPC, hasMemberMethod_OK)
{
	string methodAvailable = "{\"jsonrpc\": \"2.0\", \"params\": { \"handle\": 1}, \"method\": \"aa_close\", \"id\": 1}";
	json->parse(dom, &methodAvailable);
	CHECK(json->hasMethod(dom));
}


TEST(Plugin_JsonRPC, hasJsonRPCVersion_FAIL)
{
	string noVersion = "{\"params\": { \"handle\": 1}, \"method\": \"aa_close\", \"id\": 3}";
	json->parse(dom, &noVersion);
	CHECK_THROWS(Error, json->hasJsonRPCVersion(dom));
}


TEST(Plugin_JsonRPC, hasJsonRPCVersion_OK)
{
	json->parse(dom, &REQUEST);
	CHECK(json->hasJsonRPCVersion(dom));
}


TEST(Plugin_JsonRPC, tryTogetId_FAIL)
{
	json->parse(dom, &NOTIFICATION);
	CHECK_THROWS(Error, json->tryTogetId(dom));
}


TEST(Plugin_JsonRPC, tryTogetId_OK)
{
	json->parse(dom, &REQUEST);
	json->tryTogetId(dom);
	json->parse(dom, &RESPONSE);
	json->tryTogetId(dom);

}


TEST(Plugin_JsonRPC, tryTogetMethod_FAIL)
{
	string methodNOTAvailable = "{\"jsonrpc\": \"2.0\", \"params\": { \"handle\": 1}, \"id\": 1}";
	json->parse(dom, &methodNOTAvailable);
	CHECK_THROWS(Error, json->tryTogetMethod(dom));
}


TEST(Plugin_JsonRPC, tryTogetMethod_OK)
{
	string methodAvailable = "{\"jsonrpc\": \"2.0\", \"params\": { \"handle\": 1}, \"method\": \"aa_close\", \"id\": 1}";
	json->parse(dom, &methodAvailable);
	STRCMP_EQUAL("aa_close", json->tryTogetMethod(dom)->GetString());
}


TEST(Plugin_JsonRPC, tryTogetResult_FAIL)
{
	string resultAvailable = "{\"jsonrpc\": \"2.0\", \"id\": 1}";
	json->parse(dom, &resultAvailable);
	CHECK_THROWS(Error, json->tryTogetResult(dom));
}


TEST(Plugin_JsonRPC, tryTogetResult_OK)
{
	string resultAvailable = "{\"jsonrpc\": \"2.0\", \"result\" : \"hallo welt\", \"id\": 1}";
	json->parse(dom, &resultAvailable);
	STRCMP_EQUAL("hallo welt", json->tryTogetResult(dom)->GetString());
}


TEST(Plugin_JsonRPC, tryTogetParam_FAIL)
{

	string paramNOTAvailable = "{\"jsonrpc\": \"2.0\", \"params\": { \"handle\": 1}, \"method\": \"aa_close\", \"id\": 3}";
	json->parse(dom, &paramNOTAvailable);
	CHECK_THROWS(Error, json->tryTogetParam(dom, "totalRandomNumber"));
}


TEST(Plugin_JsonRPC, tryTogetParam_OK)
{
	Value* result;
	string paramAvailable = "{\"jsonrpc\": \"2.0\", \"params\": { \"handle\": 1, \"totalRandomNumber\": 3085}, \"method\": \"aa_close\", \"id\": 3}";
	json->parse(dom, &paramAvailable);
	result = json->tryTogetParam(dom, "totalRandomNumber");
	LONGS_EQUAL(3085, result->GetInt());
}


TEST(Plugin_JsonRPC, tryTogetParams)
{
	json->parse(dom, &REQUEST);
	json->tryTogetParams(dom);

	json->parse(dom, &RESPONSE);
	CHECK_THROWS(Error, json->tryTogetParams(dom));

}


TEST(Plugin_JsonRPC, splitMsg_FAIL)
{
	list<string*>* messages = json->splitMsg(dom, &REQUESTS_WITH_ERROR);

	CHECK_EQUAL(2, messages->size());

	list<string*>::iterator i = messages->begin();
	while(i != messages->end())
	{
		delete *i;
		i = messages->erase(i);
	}

	delete messages;
}


TEST(Plugin_JsonRPC, splitMsg_OK)
{

	list<string*>* messages = json->splitMsg(dom, &THREE_REQUESTS);

	CHECK_EQUAL(3, messages->size());

	list<string*>::iterator i = messages->begin();
	while(i != messages->end())
	{
		delete *i;
		i = messages->erase(i);
	}

	delete messages;

}


TEST(Plugin_JsonRPC, parse_FAIL)
{
	CHECK_THROWS(Error, json->parse(dom, &PARSE_ERROR_STRING));
}


TEST(Plugin_JsonRPC, isNoNotification)
{
	json->parse(dom, &REQUEST);
	CHECK_EQUAL(false, json->isNotification(dom));
}


TEST(Plugin_JsonRPC, isNotification)
{
	json->parse(dom, &NOTIFICATION);
	CHECK_EQUAL(true, json->isNotification(dom));
}


TEST(Plugin_JsonRPC, isNoError)
{
	json->parse(dom, &RESPONSE);
	CHECK_EQUAL(false, json->isError(dom));
}


TEST(Plugin_JsonRPC, isError)
{
	json->parse(dom, &ERROR_RESPONSE);
	CHECK_EQUAL(true, json->isError(dom));
}


TEST(Plugin_JsonRPC, isNoResponse)
{
	json->parse(dom, &REQUEST);
	CHECK_EQUAL(false, json->isResponse(dom));
}


TEST(Plugin_JsonRPC, isResponse)
{
	json->parse(dom, &RESPONSE);
	CHECK_EQUAL(true, json->isResponse(dom));
}


TEST(Plugin_JsonRPC, isNoRequest)
{
	json->parse(dom, &RESPONSE);
	CHECK_EQUAL(false, json->isRequest(dom));
}


TEST(Plugin_JsonRPC, isRequest)
{
	json->parse(dom, &REQUEST);
	CHECK_EQUAL(true, json->isRequest(dom));
}


TEST(Plugin_JsonRPC, checkJsonRpcVersion_FAIL)
{
	string wrongVersion = "{\"jsonrpc\": \"3.0\", \"params\": { \"handle\": 1}, \"method\": \"aa_close\", \"id\": 3}";
	json->parse(dom, &wrongVersion);
	CHECK_THROWS(Error, json->checkJsonRpcVersion(dom));
}


TEST(Plugin_JsonRPC, checkJsonRpcVersionOK)
{
	string rightVersion = "{\"jsonrpc\": \"2.0\", \"params\": { \"handle\": 1}, \"method\": \"aa_close\", \"id\": 3}";
	json->parse(dom, &rightVersion);
	CHECK(json->checkJsonRpcVersion(dom));
}

