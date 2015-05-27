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

		static void init();
		static void deInit();
		//valueType can be PORT or HANDLE
		RemoteAardvark* getDevice(Document* dom);

		void processMsg(string* msg);



	private:

		JsonRPC* json;
		string* result;
		int contextNumber;
		list<string*>* msgList;
		UdsComWorker* udsworker;
		RemoteAardvark* deviceLessFunctions;


		static list<RemoteAardvark*> deviceList;
		static pthread_mutex_t dLmutex;
		static void deleteDeviceList();


		void unlockAllUsedDevices();


};




#endif /* INCLUDE_PLUGINAARDVARK_HPP_ */
