

#include "PluginAardvark.hpp"
#include "TestHarness.h"



TEST_GROUP(Plugin_PluginAardvark)
{
	void setup()
	{


	}

	void teardown()
	{

	}
};


TEST(Plugin_PluginAardvark, processMsg_FAIL)
{

	PluginAardvark* plugin = new PluginAardvark();
	string testString = "{\"jsonrpc\": \"2.0\", \"params\":  \"handle\": 1}, \"method\": \"aa_close\", \"id\": 3}";
	//there will be one leak because of static vector (which is deallocated after main)
	CHECK_THROWS(PluginError, plugin->processMsg(&testString));
	delete plugin;

}



