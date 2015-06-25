/*
 * test_RemoteAardvark.cpp
 *
 *  Created on: 30.01.2015
 *      Author: dnoack
 */



#include "RemoteAardvark.hpp"
#include "TestHarness.h"


static RemoteAardvark* testInterface;
static Document* dom;
static Value* params;
static Value* result;

TEST_GROUP(RemoteAardvark)
{
	void setup()
	{
		dom = new Document();
		testInterface = new RemoteAardvark(0);
		params = new Value();
		params->SetObject();
		result = new Value();
	}

	void teardown()
	{
		delete dom;
		delete testInterface;
		delete params;
		delete result;
	}

};

TEST(RemoteAardvark, aa_closeOK)
{
	params->AddMember("Aardvark", 1, dom->GetAllocator());
	CHECK_EQUAL(false, testInterface->aa_close(*params, *result));
}


TEST(RemoteAardvark, aa_openOK)
{
	params->AddMember("port", 0, dom->GetAllocator());
	CHECK(testInterface->aa_open(*params, *result));
}

TEST(RemoteAardvark, aa_unique_idOK)
{
	params->AddMember("Aardvark", 1, dom->GetAllocator());
	CHECK_EQUAL(true, testInterface->aa_unique_id(*params, *result));
}

TEST(RemoteAardvark, aa_target_powerOK)
{
	params->AddMember("Aardvark", 1, dom->GetAllocator());
	params->AddMember("powerMask", 0x03, dom->GetAllocator());
	CHECK_EQUAL(true, testInterface->aa_target_power(*params, *result));
}



TEST(RemoteAardvark, aa_closeFAIL)
{
	params->AddMember("Aardvark", "notAnInteger", dom->GetAllocator());
	CHECK_THROWS(Error, testInterface->aa_close(*params, *result));
}


TEST(RemoteAardvark, aa_unique_idFAIL)
{
	params->AddMember("Aardvark", "notAnInteger", dom->GetAllocator());
	CHECK_THROWS(Error, testInterface->aa_unique_id(*params, *result));
}


TEST(RemoteAardvark, aa_target_power_handle_FAIL)
{
	params->AddMember("Aardvark", "NotAnInteger", dom->GetAllocator());
	params->AddMember("powerMask", 0x03, dom->GetAllocator());
	CHECK_THROWS(Error, testInterface->aa_target_power(*params, *result));
}


TEST(RemoteAardvark, aa_target_power_powerMask_FAIL)
{
	params->AddMember("Aardvark", 1, dom->GetAllocator());
	params->AddMember("powerMask", "NotAnUInteger", dom->GetAllocator());
	CHECK_THROWS(Error, testInterface->aa_target_power(*params, *result));
}

TEST(RemoteAardvark, aa_openFAIL)
{
	params->AddMember("port", "notAnInteger", dom->GetAllocator());
	CHECK_THROWS(Error, testInterface->aa_open(*params, *result));
}

