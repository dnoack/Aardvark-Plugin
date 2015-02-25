
#define private public

#include "AardvarkCareTaker.hpp"
#include "TestHarness.h"

static AardvarkCareTaker* plugin;



TEST_GROUP(Plugin_AardvarkCareTaker)
{
	void setup()
	{
		plugin = new AardvarkCareTaker();
	}

	void teardown()
	{
		delete plugin;
	}
};


TEST(Plugin_AardvarkCareTaker, processMsg_OK)
{
	string testString = "{\"jsonrpc\": \"2.0\", \"params\": { \"port\": 0}, \"method\": \"aa_open\", \"id\": 3}";
	string* response = plugin->processMsg(&testString);
	delete response;
}

TEST(Plugin_AardvarkCareTaker, processMsg_FAIL)
{
	string testString = "{\"jsonrpc\": \"2.0\", \"params\":  \"handle\": 1}, \"method\": \"aa_close\", \"id\": 3}";

	CHECK_THROWS(PluginError,plugin->processMsg(&testString));
	//plugin->result is normally private and will be deleted by UdsWorker
	//delete plugin->result;
}



