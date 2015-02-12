
#define private public

#include "PluginAardvark.hpp"
#include "TestHarness.h"


static PluginAardvark* plugin;

static string STRING_PARSE_FAIL =
		"{\"jsonrpc\": \"2.0\", \"params\":  \"handle\": 1}, \"method\": \"aa_close\", \"id\": 3}";


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


TEST(Plugin_PluginAardvark, processMsg_FAIL)
{
	CHECK_THROWS(PluginError, plugin->processMsg(&STRING_PARSE_FAIL));
}



