
#include "UdsServer.hpp"
#include "UdsWorker.hpp"
#include "TestHarness.h"

#define TEST_UDSFILE "/tmp/test_com.uds"

static UdsWorker* test_udsWorker;
static int test_socket;

static int bindTestSocket()
{
	int optionflag = 1;
	test_socket = socket(AF_UNIX, SOCK_STREAM, 0);
	struct sockaddr_un address;
	socklen_t addrlen;

	address.sun_family = AF_UNIX;
	strncpy(address.sun_path, TEST_UDSFILE, sizeof(TEST_UDSFILE));
	addrlen = sizeof(address);

	unlink(TEST_UDSFILE);
	setsockopt(test_socket, SOL_SOCKET, SO_REUSEADDR, &optionflag, sizeof(optionflag));
	bind(test_socket, (struct sockaddr*)&address, addrlen);
	return test_socket;
}



TEST_GROUP(Plugin_UdsWorker)
{
	void setup()
	{
		bindTestSocket();
		test_udsWorker = new UdsWorker(test_socket);
	}

	void teardown()
	{
		delete test_udsWorker;
		close(test_socket);
	}
};


TEST(Plugin_UdsWorker, test1)
{


}

TEST(Plugin_UdsWorker, test2)
{


}





