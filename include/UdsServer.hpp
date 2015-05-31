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

#include "AcceptThread.hpp"
#include "ComPoint.hpp"
#include "AardvarkCareTaker.hpp"
#include "JsonRPC.hpp"

#define MAX_CLIENTS 20

class UdsComWorker;


class UdsServer : public AcceptThread{

	public:

		UdsServer(const char* udsFile, int nameSize);
		~UdsServer();


		void checkForDeletableWorker();

	private:


		 int connection_socket;

		list<ComPoint*> comPointList;
		pthread_mutex_t wLmutex;

		struct sockaddr_un address;
		socklen_t addrlen;

		void pushComPointList(ComPoint* newWorker);
		void deleteComPointList();


		virtual void thread_accept();

		int optionflag;
};

#endif /* AARDVARK_PLUGIN_INCLUDE_UDSSERVER_HPP_ */
