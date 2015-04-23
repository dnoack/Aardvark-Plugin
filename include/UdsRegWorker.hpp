
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

		int transmit(char* data, int size);
		int transmit(const char* data, int size);
		int transmit(string* msg);

	private:

		virtual void thread_listen();
		virtual void thread_work();

		JsonRPC* json;
		const char* error;
		Value* currentMsgId;
		int currentSocket;
		enum REG_STATE{NOT_ACTIVE, ANNOUNCED, REGISTERED, ACTIVE, BROKEN};
		unsigned int state;


		void processRegistration();
		//createAnnounceMsg will be created by UdsRegClient
		bool handleAnnounceACKMsg(string* msg);
		const char* createRegisterMsg();
		bool handleRegisterACKMsg(string* msg);
		const char* createPluginActiveMsg();

};

#endif /* UDSREGWORKER_HPP_ */
