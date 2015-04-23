/*
 * UdsWorker.hpp
 *
 *  Created on: 09.02.2015
 *      Author: dnoack
 */

#ifndef INCLUDE_UDSCOMWORKER_HPP_
#define INCLUDE_UDSCOMWORKER_HPP_

//unix domain socket definition
#include <sys/un.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cstdio>
#include <cstring>


#include "JsonRPC.hpp"
#include "AardvarkCareTaker.hpp"
#include "WorkerInterface.hpp"
#include "WorkerThreads.hpp"


class UdsServer;


class UdsComWorker : public WorkerInterface<string>, public WorkerThreads{

	public:
		UdsComWorker(int socket);
		~UdsComWorker();

		int uds_send(string* data);
		int uds_send(const char* data);

		int transmit(char* data, int size);
		int transmit(const char* data, int size);
		int transmit(string* msg);

	private:

		virtual void thread_listen();

		virtual void thread_work();

		AardvarkCareTaker* paard;
		string* request;
		string* response;
		int currentSocket;

};



#endif /* INCLUDE_UDSWORKER_HPP_ */
