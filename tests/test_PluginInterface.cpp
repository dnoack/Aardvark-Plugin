/*
 * test_PluginInterface.cpp
 *
 *  Created on: 30.01.2015
 *      Author: dnoack
 */


#include "Plugin_Interface.h"
#include "Aardvark.hpp"
#include "TestHarness.h"


static RemoteAardvark* testInterface;
static Document* dom;
static Value testValue;
static char* memberValue = "NoMember";


TEST_GROUP(Plugin_Interface)
{
	void setup()
	{
		dom = new Document();
		testInterface = new RemoteAardvark(0);
	}

	void teardown()
	{
		delete dom;
		delete testInterface;
	}

};


TEST(Plugin_Interface, findParamsMember_noObject)
{
	testValue.SetInt(3005);
	CHECK_THROWS(PluginError, testInterface->findParamsMember(testValue, memberValue));
}


TEST(Plugin_Interface, findParams_MemberFound) //is object
{
	testValue.SetObject();
	testValue.AddMember("GOOD", 0,  dom->GetAllocator());
	CHECK(testInterface->findParamsMember(testValue, "GOOD"));
}


TEST(Plugin_Interface, findParams_MemberNOTFound) //is object
{
	testValue.SetObject();
	testValue.AddMember("GOOD", 0, dom->GetAllocator());
	CHECK_THROWS(PluginError, testInterface->findParamsMember(testValue, memberValue));
}




TEST(Plugin_Interface, executeFunction_FunctionNOTfound)
{
	Value method;
	Value params;
	Value result;
	char** error;

	method.SetString("aa_NOTAFUNCTION", dom->GetAllocator());
	params.SetObject();
	params.AddMember("port", 0, dom->GetAllocator());

	CHECK_THROWS(PluginError, testInterface->executeFunction(method, params, result));
}

