/*
 * test_PluginInterface.cpp
 *
 *  Created on: 30.01.2015
 *      Author: dnoack
 */


#include "DriverInterface.h"
#include "RemoteAardvark.hpp"
#include "TestHarness.h"


static RemoteAardvark* testInterface;
static Document* dom;
static Value testValue;
static const char* memberValue = "NoMember";


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


TEST(Plugin_Interface, findObjectMember_noObject)
{
	testValue.SetInt(3005);
	CHECK_THROWS(PluginError, testInterface->findObjectMember(testValue, memberValue));
}


TEST(Plugin_Interface, findParams_MemberFound) //is object
{
	testValue.SetObject();
	testValue.AddMember("GOOD", 0,  dom->GetAllocator());
	CHECK(testInterface->findObjectMember(testValue, "GOOD"));
}


TEST(Plugin_Interface, findParams_MemberNOTFound) //is object
{
	testValue.SetObject();
	testValue.AddMember("GOOD", 0, dom->GetAllocator());
	CHECK_THROWS(PluginError, testInterface->findObjectMember(testValue, memberValue));
}




TEST(Plugin_Interface, executeFunction_FunctionNOTfound)
{
	Value method;
	Value params;
	Value result;

	method.SetString("aa_NOTAFUNCTION", dom->GetAllocator());
	params.SetObject();
	params.AddMember("port", 0, dom->GetAllocator());

	CHECK_THROWS(PluginError, testInterface->executeFunction(method, params, result));
}

