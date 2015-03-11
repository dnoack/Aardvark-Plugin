
#include "UdsServer.hpp"
#include "UdsComWorker.hpp"
#include "sys/socket.h"
#include <sys/select.h>
#include <unistd.h>
#include <fcntl.h>
#include "errno.h"
#include "TestHarness.h"

#define TEST_UDSFILE "/tmp/test_com.uds"
#define RECEIVE_BUFFER_SIZE 1024


static UdsComWorker* test_udsWorker;
static struct sockaddr_un address;
static socklen_t addrlen;

static int server_accept_socket;
static int clientSocket;
static int serverSocket;
static int lfd;

static string OK_STRING =
		"{\"jsonrpc\": \"2.0\", \"params\": { \"handle\": 1}, \"method\": \"aa_unique_id\", \"id\": 1}";

static string FAIL_STRING =
		"{\"jsonrpc\": \"2.0\", \"params\":  \"handle\": 1}, \"method\": \"aa_close\", \"id\": 3}";


static int createServerAcceptSocket()
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

	return server_accept_socket;

}


static int createClientSocket()
{
	clientSocket = socket(AF_UNIX, SOCK_STREAM, 0);
	return clientSocket;
}


static void connectClientToServer(int clientSocket, int serverAcceptSocket)
{
	bool active = true;

	while(active)
	{
		serverSocket = accept(server_accept_socket, (struct sockaddr*)&address, &addrlen);
		if( serverSocket < 0)
		{
			connect(clientSocket, (struct sockaddr*)&address, addrlen);
		}
		else
			active = false;

	}
}



TEST_GROUP(Plugin_UdsComWorker)
{
	void setup()
	{
		createClientSocket();
		createServerAcceptSocket();
		connectClientToServer(clientSocket, server_accept_socket);
		test_udsWorker = new UdsComWorker(serverSocket);
		while(!test_udsWorker->isReady()){};
	}

	void teardown()
	{
		close(clientSocket);
		//close(server_accept_socket);
		while(!test_udsWorker->isDeletable()){}
		delete test_udsWorker;
	}
};


TEST(Plugin_UdsComWorker, sendCorrectMsg_and_getAnswer)
{
	char buffer[RECEIVE_BUFFER_SIZE];
	int received = 0;
	int retval = 0;
	int sendCount = 0;
	struct timeval timeout;

	timeout.tv_sec = 0;
	timeout.tv_usec = 300000;

	fd_set fds;
	FD_ZERO(&fds);
	FD_SET(clientSocket, &fds);


	memset(buffer, '\0', RECEIVE_BUFFER_SIZE);

	sendCount = send(clientSocket, OK_STRING.c_str(), OK_STRING.size(), 0);

	if(sendCount < 0)
		FAIL("Could not send TestMessage");

	retval = select(clientSocket+1, &fds, NULL, NULL, &timeout);
	if(retval > 0)
	{
		if(FD_ISSET(clientSocket, &fds))
		{
			received = recv(clientSocket, &buffer, RECEIVE_BUFFER_SIZE, 0);
			if(received > 0)
			{
				printf("Empfangen: %s \n", buffer);
			}
		}
	}
	else if(retval == 0)
	{
		printf("Timeout.\n");
	}
	else if(retval < 0)
	{
		switch(errno)
		{
			case EBADF:
				printf("errno: EBADF\n");
				break;
			case EINTR:
				printf("errno: EINTR\n");
				break;
			case EINVAL:
				printf("errno: EINVAL\n");
				break;
			case ENOMEM:
				printf("errno: ENOMEM\n");
				break;
		}

	}


}



TEST(Plugin_UdsComWorker, correctMsg_x1000)
{
	char buffer[RECEIVE_BUFFER_SIZE];
	bool active = true;
	int received = 0;
	int sendCount = 0;
	int repeats = 50;
	int retval = 0;

	struct timeval timeout;

	timeout.tv_sec = 0;
	timeout.tv_usec = 300000;

	fd_set fds;
	FD_ZERO(&fds);
	FD_SET(clientSocket, &fds);

	memset(buffer, '\0', RECEIVE_BUFFER_SIZE);

	for(int i = 0; i < repeats; i++)
	{

		active = true;
		sendCount = send(clientSocket, OK_STRING.c_str(), OK_STRING.size(), 0);

		if(sendCount < 0)
			FAIL("Could not send TestMessage");


		while(active)
		{

			retval = select(clientSocket+1, &fds, NULL, NULL, &timeout);

			if(retval > 0)
			{
				received = recv(clientSocket, &buffer, RECEIVE_BUFFER_SIZE, 0);
				if(received > 0)
				{
					printf("Empfangen: %s \n", buffer);

				}
			}
			else
				FAIL("Socket error or timeout.\n");

			active = false;
		}
	}

}




