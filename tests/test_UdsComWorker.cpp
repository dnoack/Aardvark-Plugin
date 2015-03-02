
#include "UdsServer.hpp"
#include "UdsComWorker.hpp"
#include "sys/socket.h"
#include <unistd.h>
#include <fcntl.h>
#include "TestHarness.h"

#define TEST_UDSFILE "/tmp/test_com.uds"
#define RECEIVE_BUFFER_SIZE 1024
#define MILLISECONDS *1000

static UdsComWorker* test_udsWorker;
static struct sockaddr_un address;
static socklen_t addrlen;

static int server_accept_socket;
static int clientSocket;
static int serverSocket;

static string OK_STRING =
		"{\"jsonrpc\": \"2.0\", \"params\": { \"handle\": 1}, \"method\": \"aa_unique_id\", \"id\": 1}";

static string FAIL_STRING =
		"{\"jsonrpc\": \"2.0\", \"params\":  \"handle\": 1}, \"method\": \"aa_close\", \"id\": 3}";


static void createServerAcceptSocket()
{
	int optionflag = 1;
	int val = 0;

	server_accept_socket = socket(AF_UNIX, SOCK_STREAM, 0);

	//manipulate fd for nonblocking mode accept socket
	val = fcntl(server_accept_socket, F_GETFL, 0);
	fcntl(server_accept_socket, F_SETFL, val|O_NONBLOCK);

	//set uds file
	address.sun_family = AF_UNIX;
	strncpy(address.sun_path, TEST_UDSFILE, sizeof(TEST_UDSFILE));
	addrlen = sizeof(address);


	unlink(TEST_UDSFILE);
	setsockopt(server_accept_socket, SOL_SOCKET, SO_REUSEADDR, &optionflag, sizeof(optionflag));
	bind(server_accept_socket, (struct sockaddr*)&address, addrlen);
	listen(server_accept_socket, 1);

}


static void createClientSocket()
{
	clientSocket = socket(AF_UNIX, SOCK_STREAM, 0);
}


static void connectClientToServer(int clientSocket, int serverAcceptSocket)
{
	bool active = true;
	int connectOK = 0;

	while(active)
	{
		serverSocket = accept(server_accept_socket, (struct sockaddr*)&address, &addrlen);
		if( serverSocket < 0)
		{
			connectOK = connect(clientSocket, (struct sockaddr*)&address, addrlen);
		}
		else
			active = false;
	}
}



TEST_GROUP(Plugin_UdsComWorker)
{
	void setup()
	{
		createServerAcceptSocket();
		createClientSocket();
		connectClientToServer(clientSocket, server_accept_socket);
		test_udsWorker = new UdsComWorker(serverSocket);
	}

	void teardown()
	{
		delete test_udsWorker;
		close(clientSocket);
		//serverSocket will be close trough test_udsWorker destructor
		close(server_accept_socket);
	}
};


IGNORE_TEST(Plugin_UdsComWorker, test1)
{
	char buffer[RECEIVE_BUFFER_SIZE];
	bool active = true;
	int received = 0;
	int sendCount = 0;
	int sleepCount = 0;
	int sleepMax = 5;

	memset(buffer, '\0', RECEIVE_BUFFER_SIZE);

	usleep(200 MILLISECONDS);
	sendCount = send(clientSocket, OK_STRING.c_str(), OK_STRING.size(), 0);

	if(sendCount < 0)
		FAIL("Could not send TestMessage");

	while(active)
	{
		received = recv(clientSocket, &buffer, RECEIVE_BUFFER_SIZE, MSG_DONTWAIT);
		if(received > 0)
		{
			printf("Empfangen: %s \n", buffer);
			active = false;
		}
		usleep(50 MILLISECONDS);
		sleepCount++;
		if(sleepCount > sleepMax)
		{
			active = false;
			FAIL("Timeout reached");
		}
	}

}




