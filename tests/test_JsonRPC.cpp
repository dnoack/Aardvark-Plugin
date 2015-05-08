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
		dom = json->inputDOM;
		dom->SetObject();
	}

	void teardown()
	{
		delete json;
	}
};


TEST(Plugin_JsonRPC, checkParse_FAIL)
{
	CHECK_THROWS(Error, json->parse(&PARSE_ERROR_STRING));
}


TEST(Plugin_JsonRPC, checkJsonRpc_RequestFormat_OK)
{
	json->parse(&OK_STRING);
	CHECK(json->checkJsonRpc_RequestFormat());
}


TEST(Plugin_JsonRPC, checkJsonRpc_RequestFormat_FAIL)
{
	json->parse(&REQ_FROMAT_ERROR_STRING);
	CHECK_THROWS(Error, json->checkJsonRpc_RequestFormat());
}


TEST(Plugin_JsonRPC, hasJsonRpcVersion_OK)
{
	json->parse(&OK_STRING);
	CHECK(json->hasJsonRPCVersion());
}


TEST(Plugin_JsonRPC, hasJsonRpcVersion_FAIL)
{
	string noVersion = "{\"params\": { \"handle\": 1}, \"method\": \"aa_close\", \"id\": 3}";
	string noString = "{\"jsonrpc\": 2.0, \"params\": { \"handle\": 1}, \"method\": \"aa_close\", \"id\": 3}";
	json->parse(&noVersion);
	CHECK_THROWS(Error, json->hasJsonRPCVersion());
	json->parse(&noString);
	CHECK_THROWS(Error, json->hasJsonRPCVersion());
}


TEST(Plugin_JsonRPC, checkJsonRpcVersion_FAIL)
{
	string wrongVersion = "{\"jsonrpc\": \"3.0\", \"params\": { \"handle\": 1}, \"method\": \"aa_close\", \"id\": 3}";
	json->parse(&wrongVersion);
	CHECK_THROWS(Error, json->checkJsonRpcVersion());
}



TEST(Plugin_JsonRPC, checkJsonRpcVersionOK)
{
	string rightVersion = "{\"jsonrpc\": \"2.0\", \"params\": { \"handle\": 1}, \"method\": \"aa_close\", \"id\": 3}";
	json->parse(&rightVersion);
	CHECK(json->checkJsonRpcVersion());
}


TEST(Plugin_JsonRPC, hasMemberId_OK)
{
	string idAvailable = "{\"jsonrpc\": \"2.0\", \"params\": { \"handle\": 1}, \"method\": \"aa_close\", \"id\": 3}";
	json->parse(&idAvailable);
	CHECK(json->hasId());
}


TEST(Plugin_JsonRPC, hasMemberId_FAIL)
{
	string idNOTAvailable = "{\"jsonrpc\": \"2.0\", \"params\": { \"handle\": 1}, \"method\": \"aa_close\"}";
	json->parse(&idNOTAvailable);
	CHECK_THROWS(Error, json->hasId());
}


TEST(Plugin_JsonRPC, hasMemberMethod_OK)
{
	string methodAvailable = "{\"jsonrpc\": \"2.0\", \"params\": { \"handle\": 1}, \"method\": \"aa_close\", \"id\": 1}";
	json->parse(&methodAvailable);
	CHECK(json->hasMethod());
}


TEST(Plugin_JsonRPC, hasMemberMethod_FAIL)
{
	string methodNOTAvailable = "{\"jsonrpc\": \"2.0\", \"params\": { \"handle\": 1}, \"id\": 1}";
	string methodNOString = "{\"jsonrpc\": \"2.0\", \"params\": { \"handle\": 1}, \"method\": 4563456 , \"id\": 1}";
	json->parse(&methodNOTAvailable);
	CHECK_THROWS(Error, json->hasMethod());
	json->parse(&methodNOString);
	CHECK_THROWS(Error, json->hasMethod());

}


TEST(Plugin_JsonRPC, hasParams_OK)
{
	string paramsAvailable = "{\"jsonrpc\": \"2.0\", \"params\": { \"handle\": 1}, \"method\": \"aa_close\", \"id\": 1}";
	json->parse(&paramsAvailable);
	CHECK(json->hasParams());
}


TEST(Plugin_JsonRPC, hasParams_FAIL)
{
	string paramsNOTAvailable = "{\"jsonrpc\": \"2.0\", \"method\": \"aa_close\", \"id\": 1}";
	string paramsIsString = "{\"jsonrpc\": \"2.0\", \"params\":  \"handle : 1\", \"method\": \"aa_close\", \"id\": 1}";
	string paramsIsInt = "{\"jsonrpc\": \"2.0\", \"params\": 3425345, \"method\": \"aa_close\", \"id\": 1}";
	json->parse(&paramsNOTAvailable);
	CHECK_THROWS(Error, json->hasParams());
	json->parse(&paramsIsString);
	CHECK_THROWS(Error, json->hasParams());
	json->parse(&paramsIsInt);
	CHECK_THROWS(Error, json->hasParams());

}

//hasResult
TEST(Plugin_JsonRPC, hasResult_OK)
{
	string resultAvailable = "{\"jsonrpc\": \"2.0\", \"result\" : \"hallo welt\", \"id\": 1}";
	json->parse(&resultAvailable);
	CHECK(json->hasResult());

}


TEST(Plugin_JsonRPC, hasResult_FAIL)
{
	string resultNOTAvailable = "{\"jsonrpc\": \"2.0\", \"id\": 1}";
	json->parse(&resultNOTAvailable);
	CHECK_THROWS(Error, json->hasResult());

}


//hasError
TEST(Plugin_JsonRPC, hasError_OK)
{
	string errorAvailable = "{\"jsonrpc\": \"2.0\", \"error\": {\"code\" : -32000, \"message\": \"This is an error\"}, \"id\": 1}";
	json->parse(&errorAvailable);
	CHECK(json->hasError());
}


TEST(Plugin_JsonRPC, hasError_FAIL)
{
	string errorNOTAvailable = "{\"jsonrpc\": \"2.0\", \"id\": 1}";
	json->parse(&errorNOTAvailable);
	CHECK_THROWS(Error, json->hasError());

}


TEST(Plugin_JsonRPC, tryTogetParam_OK)
{
	Value* result;
	string paramAvailable = "{\"jsonrpc\": \"2.0\", \"params\": { \"handle\": 1, \"totalRandomNumber\": 3085}, \"method\": \"aa_close\", \"id\": 3}";
	json->parse(&paramAvailable);
	result = json->tryTogetParam("totalRandomNumber");
	LONGS_EQUAL(3085, result->GetInt());

}


TEST(Plugin_JsonRPC, tryTogetParam_FAIL)
{

	string paramNOTAvailable = "{\"jsonrpc\": \"2.0\", \"params\": { \"handle\": 1}, \"method\": \"aa_close\", \"id\": 3}";
	json->parse(&paramNOTAvailable);
	CHECK_THROWS(Error, json->tryTogetParam("totalRandomNumber"));

}


TEST(Plugin_JsonRPC, tryTogetResult_OK)
{
	string resultAvailable = "{\"jsonrpc\": \"2.0\", \"result\" : \"hallo welt\", \"id\": 1}";
	json->parse(&resultAvailable);
	STRCMP_EQUAL("hallo welt", json->tryTogetResult()->GetString());
}


TEST(Plugin_JsonRPC, tryTogetResult_FAIL)
{
	string resultAvailable = "{\"jsonrpc\": \"2.0\", \"id\": 1}";
	json->parse(&resultAvailable);
	CHECK_THROWS(Error, json->tryTogetResult());

}


TEST(Plugin_JsonRPC, tryTogetMethod_OK)
{
	string methodAvailable = "{\"jsonrpc\": \"2.0\", \"params\": { \"handle\": 1}, \"method\": \"aa_close\", \"id\": 1}";
	json->parse(&methodAvailable);
	STRCMP_EQUAL("aa_close", json->tryTogetMethod()->GetString());
}


TEST(Plugin_JsonRPC, tryTogetMethod_FAIL)
{
	string methodNOTAvailable = "{\"jsonrpc\": \"2.0\", \"params\": { \"handle\": 1}, \"id\": 1}";
	json->parse(&methodNOTAvailable);
	CHECK_THROWS(Error, json->tryTogetMethod());

}



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






