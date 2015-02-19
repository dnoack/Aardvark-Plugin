
#define private public

#include "PluginAardvark.hpp"
#include "TestHarness.h"

static PluginAardvark* plugin;



TEST_GROUP(Plugin_PluginAardvark)
{
	void setup()
	{
		plugin = new PluginAardvark();
	}

	void teardown()
	{
		delete plugin;
	}
};


TEST(Plugin_PluginAardvark, processMsg_OK)
{
	string testString = "{\"jsonrpc\": \"2.0\", \"params\": { \"handle\": 1}, \"method\": \"aa_close\", \"id\": 3}";
	string* response = plugin->processMsg(&testString);
	delete response;
}

TEST(Plugin_PluginAardvark, processMsg_FAIL)
{
	string testString = "{\"jsonrpc\": \"2.0\", \"params\":  \"handle\": 1}, \"method\": \"aa_close\", \"id\": 3}";

	CHECK_THROWS(string*,plugin->processMsg(&testString));
	//plugin->result is normally private and will be deleted by UdsWorker
	delete plugin->result;
}



