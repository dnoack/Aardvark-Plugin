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


#define BUFFER_SIZE 1024
#define ADD_WORKER true
#define DELETE_WORKER false



class UdsComWorker : public WorkerInterface<string>, public WorkerThreads{

	public:
		UdsComWorker(int socket);
		~UdsComWorker();

		int uds_send(string* data);
		int uds_send(const char* data);

	private:

		virtual void thread_listen(pthread_t partent_th, int socket, char* workerBuffer);

		virtual void thread_work(int socket);

		AardvarkCareTaker* paard;
		string* request;
		string* response;
		int currentSocket;

};



#endif /* INCLUDE_UDSWORKER_HPP_ */
