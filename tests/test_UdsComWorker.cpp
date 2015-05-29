#define protected public

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


static string OK_STRING =
		"{\"jsonrpc\": \"2.0\", \"params\": { \"handle\": 1}, \"method\": \"aa_unique_id\", \"id\": 1}";

static string OK_STRING2 =
		"{\"jsonrpc\": \"2.0\", \"params\": { \"handle\": 1}, \"method\": \"aa_close\", \"id\": 30}";

static string FAIL_STRING =
		"{\"jsonrpc\": \"2.0\", \"params\":  \"handle\": 1}, \"method\": \"aa_close\", \"id\": 3}";

static string SPLIT1_MSG =
		"{\"jsonrpc\": \"2.0\", \"params\": { \"handle\": 1}, \"met";

static string SPLIT2_MSG =
		"hod\": \"aa_unique_id\", \"id\": 1}";


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


TEST_GROUP(WorkerInterface)
{
	void setup()
	{
		test_udsWorker = new UdsComWorker(serverSocket);
	}

	void teardown()
	{
		while(!test_udsWorker->isDeletable()){}
		delete test_udsWorker;
	}
};



TEST_GROUP(Plugin_UdsComWorker)
{
	void setup()
	{
		createClientSocket();
		createServerAcceptSocket();
		connectClientToServer(clientSocket, server_accept_socket);
		test_udsWorker = new UdsComWorker(serverSocket);
	}

	void teardown()
	{
		close(clientSocket);
		while(!test_udsWorker->isDeletable()){}
		delete test_udsWorker;
	}
};


TEST(WorkerInterface, makeHeader_negativeValue)
{
	int testValue = -30001;
	char* header = new char[HEADER_SIZE];

	test_udsWorker->createHeader(header, testValue);

	CHECK_EQUAL(0, test_udsWorker->readHeader(header));

	delete[] header;

}



TEST(WorkerInterface, makeHeader_and_convertBack)
{
	int testValue = 27654;
	char* header = new char[HEADER_SIZE];

	test_udsWorker->createHeader(header, testValue);

	CHECK_EQUAL(testValue, test_udsWorker->readHeader(header));

	delete[] header;

}


TEST(WorkerInterface, convertInt_to_binaryCharArray)
{
	char* header = new char[HEADER_SIZE];
	test_udsWorker->createHeader(header, 1024);

	delete[] header;
}


TEST(Plugin_UdsComWorker, send_splitted_Msg_with_splitted_Header)
{
	char buffer[RECEIVE_BUFFER_SIZE];
	char* header = NULL;
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

	header = new char[HEADER_SIZE];
	test_udsWorker->createHeader(header, SPLIT1_MSG.size() + SPLIT2_MSG.size());

	sendCount = send(clientSocket, header, 2, 0);
	sleep(1);
	sendCount = send(clientSocket, &header[2], 3, 0);


	sendCount = send(clientSocket, SPLIT1_MSG.c_str(), SPLIT1_MSG.size(), 0);
	sleep(1);
	sendCount = send(clientSocket, SPLIT2_MSG.c_str(), SPLIT2_MSG.size(), 0);



	delete[] header;

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
			FAIL("TIMEOUT");
		}

}




TEST(Plugin_UdsComWorker, send_Msg_with_splittedHeader)
{
	char buffer[RECEIVE_BUFFER_SIZE];
	char* header = NULL;
	int received = 0;
	int retval = 0;
	int sendCount = 0;
	struct timeval timeout;

	timeout.tv_sec = 3;
	timeout.tv_usec = 0;

	fd_set fds;
	FD_ZERO(&fds);
	FD_SET(clientSocket, &fds);


	memset(buffer, '\0', RECEIVE_BUFFER_SIZE);

	header = new char[HEADER_SIZE];
	test_udsWorker->createHeader(header, OK_STRING.size());


	sendCount = send(clientSocket, header, 2, 0);
	sleep(1);
	sendCount = send(clientSocket, &header[2], 3, 0);

	sendCount = send(clientSocket, OK_STRING.c_str(), OK_STRING.size(), 0);



	delete[] header;

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
			FAIL("TIMEOUT");
		}

}




TEST(Plugin_UdsComWorker, send_splitted_Msg)
{
	char buffer[RECEIVE_BUFFER_SIZE];
	char* header = NULL;
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

	header = new char[HEADER_SIZE];
	test_udsWorker->createHeader(header, SPLIT1_MSG.size() + SPLIT2_MSG.size());


	sendCount = send(clientSocket, header, HEADER_SIZE, 0);
	sendCount = send(clientSocket, SPLIT1_MSG.c_str(), SPLIT1_MSG.size(), 0);
	sleep(1);
	sendCount = send(clientSocket, SPLIT2_MSG.c_str(), SPLIT2_MSG.size(), 0);



	delete[] header;

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
			FAIL("TIMEOUT");
		}

}




TEST(Plugin_UdsComWorker, sendMultipleMessages)
{
	char buffer[RECEIVE_BUFFER_SIZE];
	char* header = NULL;
	char* header2 = NULL;
	int received = 0;
	int retval = 0;
	int sendCount = 0;
	struct timeval timeout;
	int messageCount = 2;

	timeout.tv_sec = 0;
	timeout.tv_usec = 300000;

	fd_set fds;
	FD_ZERO(&fds);
	FD_SET(clientSocket, &fds);


	memset(buffer, '\0', RECEIVE_BUFFER_SIZE);

	header = new char[HEADER_SIZE];
	test_udsWorker->createHeader(header, OK_STRING.size());
	header2 = new char[HEADER_SIZE];
	test_udsWorker->createHeader(header2, OK_STRING2.size());


	sendCount = send(clientSocket, header, 5, 0);
	sendCount = send(clientSocket, OK_STRING.c_str(), OK_STRING.size(), 0);


	sendCount = send(clientSocket, header2, 5, 0);
	sendCount = send(clientSocket, OK_STRING2.c_str(), OK_STRING2.size(), 0);


	delete[] header;
	delete[] header2;

	if(sendCount < 0)
		FAIL("Could not send TestMessage");

	while(messageCount != 0)
	{
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
		messageCount--;
	}

}




TEST(Plugin_UdsComWorker, sendCorrectMsg_and_getAnswer)
{
	char buffer[RECEIVE_BUFFER_SIZE];
	char* header = NULL;
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

	header = new char[5];
	test_udsWorker->createHeader(header, OK_STRING.size());


	sendCount = send(clientSocket, header, 5, 0);
	sendCount = send(clientSocket, OK_STRING.c_str(), OK_STRING.size(), 0);


	delete[] header;

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



IGNORE_TEST(Plugin_UdsComWorker, correctMsg_x50)
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




