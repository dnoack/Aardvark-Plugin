/*
 * UdsRegWorker.hpp
 *
 *  Created on: 25.02.2015
 *      Author: Dave
 */

#ifndef UDSREGWORKER_HPP_
#define UDSREGWORKER_HPP_

#include <sys/un.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cstdio>
#include <cstring>


#include "JsonRPC.hpp"
#include "WorkerInterface.hpp"
#include "WorkerThreads.hpp"

#define BUFFER_SIZE 1024

class UdsRegWorker : public WorkerInterface, WorkerThreads{

	public:
		UdsRegWorker(int socket);
		~UdsRegWorker();

	private:
		//variables for listener
		bool listen_thread_active;
		char receiveBuffer[BUFFER_SIZE];
		int recvSize;

		//not shared, more common
		pthread_t lthread;
		int currentSocket;


		//variables for worker
		bool worker_thread_active;

		virtual void thread_listen(pthread_t partent_th, int socket, char* workerBuffer);

		virtual void thread_work(int socket);

};

#endif /* UDSREGWORKER_HPP_ */
