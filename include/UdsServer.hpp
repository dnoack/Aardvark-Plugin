/*
 * UDS.h
 *
 *  Created on: 04.02.2015
 *      Author: dnoack
 */

#ifndef INCLUDE_UDSSERVER_HPP_
#define INCLUDE_UDSSERVER_HPP_

//unix domain socket definition
#include <sys/un.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cstdio>



#include "JsonRPC.hpp"
#include "Aardvark.hpp"
#include "MyThreadClass.hpp"
#include "signal.h"


#define BUFFER_SIZE 1024
#define CLIENT_MODE 1
#define SERVER_MODE 2


class UdsServer : public MyThreadClass{

	public:
		UdsServer(int mode, const char* udsFile, int nameSize);

		~UdsServer();

		void start();

		int call();

		int ud_send(char* msg, int msgSize);

		int ud_receive(char* buffer, int bufferSize);

		int ud_listen();

		void startCom();



	private:

		int connection_socket;
		int worker_socket;

		vector<pthread_t> workerList;

		int optionflag = 1;

		struct sockaddr_un address;
		socklen_t addrlen;

		Plugin::JsonRPC* json;



		/** Status var of the listen_thread*/
		bool listen_thread_active;
		/** Status var ot the work_thread*/
		bool work_thread_active;
		bool accept_thread_active;


		virtual void thread_listen(pthread_t partent_th, int socket);

		virtual void thread_work(int socket);

		virtual void thread_accept();




};



#endif /* INCLUDE_UDSSERVER_HPP_ */
