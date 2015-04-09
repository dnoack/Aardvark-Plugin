/*
 * PlugingAardvark.hpp
 *
 *  Created on: 05.02.2015
 *      Author: dnoack
 */

#ifndef INCLUDE_PLUGINAARDVARK_HPP_
#define INCLUDE_PLUGINAARDVARK_HPP_

#define EXPECTED_NUM_OF_DEVICES 1
#define PORT 0
#define HANDLE 1

#include <vector>
#include "JsonRPC.hpp"
#include "RemoteAardvark.hpp"

class UdsComWorker;

class AardvarkCareTaker{

	public:
		AardvarkCareTaker();
		AardvarkCareTaker(UdsComWorker* udsWorker);
		~AardvarkCareTaker();

		//valueType can be PORT or HANDLE
		RemoteAardvark* getDevice(int value, int valueType);

		string* processMsg(string* msg);


	private:

		JsonRPC* json;
		string* result;
		int contextNumber;
		list<string*>* msgList;
		UdsComWorker* udsworker;
		RemoteAardvark* deviceLessFunctions;

		static int instanceCount;
		static list<RemoteAardvark*> deviceList;
		static pthread_mutex_t dLmutex;
		static pthread_mutex_t instanceCountMutex;


		void deleteDeviceList();
		void unlockAllUsedDevices();


		static void increaseInstanceCount()
		{
			pthread_mutex_lock(&instanceCountMutex);
				++instanceCount;
			pthread_mutex_unlock(&instanceCountMutex);
		}


		static void decreaseInstanceCount()
		{
			pthread_mutex_lock(&instanceCountMutex);
				--instanceCount;
			pthread_mutex_unlock(&instanceCountMutex);

		}



		static int getInstanceCount()
		{
			int result = 0;
			pthread_mutex_lock(&instanceCountMutex);
				result = instanceCount;
			pthread_mutex_unlock(&instanceCountMutex);
			return result;
		}



};




#endif /* INCLUDE_PLUGINAARDVARK_HPP_ */
