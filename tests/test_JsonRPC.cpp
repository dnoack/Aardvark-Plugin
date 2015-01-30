/*
 * test_JsonRPC.cpp
 *
 *  Created on: 30.01.2015
 *  Author: David Noack
 */

#include "JsonRPC.hpp"
#include "TestHarness.h"

static Document* dom;
static Plugin::JsonRPC* json;



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


TEST(Plugin_JsonRPC, checkJsonRpcVersionOK)
{
	dom->AddMember("jsonrpc", "2.0", dom->GetAllocator());
	CHECK(json->checkJsonRpcVersion(dom));
}




