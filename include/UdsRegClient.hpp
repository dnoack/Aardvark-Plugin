/*
 * UdsRegClient.hpp
 *
 *  Created on: 25.02.2015
 *      Author: dnoack
 */

#ifndef INCLUDE_UDSREGCLIENT_HPP_
#define INCLUDE_UDSREGCLIENT_HPP_

#include <sys/un.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cstdio>
#include <list>
#include <pthread.h>
#include "signal.h"
#include <string>

#include "UdsRegClient.hpp"
#include "UdsRegWorker.hpp"

using namespace std;


class UdsRegClient{


	public:
		UdsRegClient(const char* UDS_FILE_PATH, int size);
		~UdsRegClient();


		void connectToRSD();
		void registerToRSD();
		void unregisterFromRSD();



	private:

	UdsRegWorker* regWorker;

	static struct sockaddr_un address;
	static socklen_t addrlen;

	int optionflag;
	int currentSocket;

	int sendData(string* data);
};




#endif /* INCLUDE_UDSREGCLIENT_HPP_ */