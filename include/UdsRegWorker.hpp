
#ifndef INCLUDE_UDSREGWORKER_HPP_
#define INCLUDE_UDSREGWORKER_HPP_

#include <sys/select.h>
#include <sys/un.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cstdio>
#include <cstring>


#include "WorkerInterface.hpp"
#include "WorkerThreads.hpp"
#include "errno.h"
#include "Error.hpp"

class UdsRegClient;

class UdsRegWorker : public WorkerInterface<string>, public WorkerThreads{

	public:
		UdsRegWorker(UdsRegClient* regClient, int socket);
		~UdsRegWorker();


		int transmit(const char* data, int size);
		int transmit(string* msg);

	private:

		UdsRegClient* regClient;

		virtual void thread_listen();
		virtual void thread_work();


};

#endif /* INCLUDE_UDSREGWORKER_HPP_ */
