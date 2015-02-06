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
#include <pthread.h>
#include "MyThreadClass.hpp"
#include "signal.h"


#define BUFFER_SIZE 1024
#define CLIENT_MODE 1
#define SERVER_MODE 2
#define ADD_WORKER true
#define DELETE_WORKER false



class UdsServer : public MyThreadClass{

	public:
		UdsServer(int mode, const char* udsFile, int nameSize);

		~UdsServer();

		void start();

		int call();


		void startCom();



	private:

		int connection_socket;
		int worker_socket;

		//list of pthread ids with all the active worker. push and pop must be protected by mutex
		vector<pthread_t> workerList;
		pthread_mutex_t wLmutex;

		int optionflag = 1;

		struct sockaddr_un address;
		socklen_t addrlen;

		Plugin::JsonRPC* json; //json rpc parser for main server thread

		sigset_t sigmask;
		struct sigaction action;
		struct sigaction pipehandler;


		virtual void thread_listen(pthread_t partent_th, int socket, char* workerBuffer);


		virtual void thread_work(int socket);

		virtual void thread_accept();


		static void dummy_handler(int){};


		//add=true -> add the worker, add=false->delete worker
		void editWorkerList(pthread_t newWorker, bool add)
		{
			pthread_mutex_lock(&wLmutex);
			if(add)
			{
				workerList.push_back(newWorker);
			}
			else
			{
				//find worker
				for(int i = 0; i < workerList.size() ; i++)
				{
					if(workerList[i] == newWorker)
					{
						workerList.erase(workerList.begin()+i);
						break;
					}
				}
			}
			pthread_mutex_unlock(&wLmutex);
		}



		void configSignals()
		{
			sigfillset(&sigmask);
			pthread_sigmask(SIG_UNBLOCK, &sigmask, (sigset_t*)0);

			action.sa_flags = 0;
			action.sa_handler = dummy_handler;
			sigaction(SIGUSR1, &action, (struct sigaction*)0);
			sigaction(SIGUSR2, &action, (struct sigaction*)0);
			sigaction(SIGPOLL, &action, (struct sigaction*)0);
			sigaction(SIGPIPE, &action, (struct sigaction*)0);
		}




};



#endif /* INCLUDE_UDSSERVER_HPP_ */
