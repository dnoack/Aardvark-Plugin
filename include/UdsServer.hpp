#ifndef AARDVARK_PLUGIN_INCLUDE_UDSSERVER_HPP_
#define AARDVARK_PLUGIN_INCLUDE_UDSSERVER_HPP_


#include <sys/un.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cstdio>
#include <list>
#include <pthread.h>

#include "signal.h"
#include "JsonRPC.hpp"

#define MAX_CLIENTS 20

class UdsComWorker;


class UdsServer{

	public:

		UdsServer(const char* udsFile, int nameSize);
		~UdsServer();

		bool isReady(){return ready;}
		void checkForDeletableWorker();

	private:

		static bool ready;
		static int connection_socket;

		static list<UdsComWorker*> workerList;
		static pthread_mutex_t wLmutex;

		static struct sockaddr_un address;
		static socklen_t addrlen;

		static void* uds_accept(void*);
		static void pushWorkerList(UdsComWorker* newWorker);
		static void deleteWorkerList();

		int optionflag;
		pthread_t accepter;

};

#endif /* AARDVARK_PLUGIN_INCLUDE_UDSSERVER_HPP_ */
