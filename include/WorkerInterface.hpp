/*
 * WorkerInterface.hpp
 *
 *  Created on: 25.02.2015
 *      Author: Dave
 */

#ifndef WORKERINTERFACE_HPP_
#define WORKERINTERFACE_HPP_

#include <list>
#include <string>
#include <pthread.h>
#include "signal.h"


using namespace std;


class WorkerInterface{

	public:
		WorkerInterface()
		{
			pthread_mutex_init(&rQmutex, NULL);

			this->currentSig = 0;
			this->deletable = false;
			this->ready = false;
			configSignals();

		};


		~WorkerInterface()
		{
			pthread_mutex_destroy(&rQmutex);
		};

		bool isDeletable(){return deletable;}
		bool isReady(){return ready;}


	protected:

		//receivequeue
		list<string*> receiveQueue;
		pthread_mutex_t rQmutex;


		//signal variables
		struct sigaction action;
		sigset_t sigmask;
		int currentSig;


		bool deletable;
		bool ready;


		void popReceiveQueue()
		{
			string* lastElement = NULL;
			pthread_mutex_lock(&rQmutex);
				lastElement = receiveQueue.back();
				receiveQueue.pop_back();
				delete lastElement;
			pthread_mutex_unlock(&rQmutex);
		}


		void pushReceiveQueue(string* data)
		{
			pthread_mutex_lock(&rQmutex);
				receiveQueue.push_front(data);
			pthread_mutex_unlock(&rQmutex);
		}


		int getReceiveQueueSize()
		{
			int result = 0;
			pthread_mutex_lock(&rQmutex);
				result = receiveQueue.size();
			pthread_mutex_unlock(&rQmutex);

			return result;
		}



		void configSignals()
		{
			sigemptyset(&sigmask);
			sigaddset(&sigmask, SIGUSR1);
			sigaddset(&sigmask, SIGUSR2);
			sigaddset(&sigmask, SIGPOLL);
			sigaddset(&sigmask, SIGPIPE);
			pthread_sigmask(SIG_BLOCK, &sigmask, NULL);

		}


};



#endif /* WORKERINTERFACE_HPP_ */
