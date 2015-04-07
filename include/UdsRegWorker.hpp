/*
 * UdsRegWorker.hpp
 *
 *  Created on: 	25.02.2015
 *  Last edited:	20.03.2015
 *  Author: 		dnoack
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



class UdsRegWorker : public WorkerInterface<string>, public WorkerThreads{

	public:
		UdsRegWorker(int socket);
		~UdsRegWorker();

		bool isReady(){return ready;}

	private:
		JsonRPC* json;
		const char* error;
		Value* currentMsgId;


		int currentSocket;
		enum REG_STATE{NOT_ACTIVE, ANNOUNCED, REGISTERED, ACTIVE, BROKEN};
		unsigned int state;

		int uds_send(char* msg);
		void processRegistration();
		//createAnnounceMsg will be created by UdsRegClient
		bool handleAnnounceACKMsg(string* msg);
		const char* createRegisterMsg();
		bool handleRegisterACKMsg(string* msg);
		const char* createPluginActiveMsg();


		virtual void thread_listen(pthread_t partent_th, int socket, char* workerBuffer);
		virtual void thread_work(int socket);
};

#endif /* UDSREGWORKER_HPP_ */
