
#include "UdsWorker.hpp"
#include "TestHarness.h"



/*
static int bindTestSocket()
{
	int optionflag = 1;
	static int connection_socket = socket(AF_UNIX, SOCK_STREAM, 0);
	struct sockaddr_un address;
	socklen_t addrlen;

	address.sun_family = AF_UNIX;
	strncpy(address.sun_path, TEST_UDSFILE, sizeof(TEST_UDSFILE));
	addrlen = sizeof(address);

	unlink(TEST_UDSFILE);
	setsockopt(connection_socket, SOL_SOCKET, SO_REUSEADDR, &optionflag, sizeof(optionflag));
	bind(connection_socket, (struct sockaddr*)&address, addrlen);
	return connection_socket;
}*/



TEST_GROUP(Plugin_UdsWorker)
{
	void setup()
	{

	}

	void teardown()
	{

	}
};


TEST(Plugin_UdsWorker, test1)
{
	UdsWorker* test_udsWorker = new UdsWorker(1);
	//there will be one leak because of static vector (which is deallocated after main)
	MemoryLeakWarningPlugin::getFirstPlugin()->expectLeaksInTest(1);
	FAIL("test failed");
	delete test_udsWorker;
}



