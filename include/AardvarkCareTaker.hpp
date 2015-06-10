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
#include "ProcessInterface.hpp"
#include "JsonRPC.hpp"
#include "OutgoingMsg.hpp"
#include "RemoteAardvark.hpp"

class UdsComWorker;

class AardvarkCareTaker : public ProcessInterface{

	public:
		AardvarkCareTaker();
		~AardvarkCareTaker();

		static void init();
		static void deInit();
		//valueType can be PORT or HANDLE
		RemoteAardvark* getDevice(int value, int valueType);

		OutgoingMsg* process(RPCMsg* msg);

	private:

		JsonRPC* json;
		//for generating json rpc error responses
		const char* error;
		Document* currentDom;
		//saving json rpc id of incomming msg
		Value* id;
		const char* result;
		int contextNumber;
		RemoteAardvark* deviceLessFunctions;


		static list<RemoteAardvark*> deviceList;
		static pthread_mutex_t dLmutex;
		static void deleteDeviceList();

		void unlockAllUsedDevices();
};


#endif /* INCLUDE_PLUGINAARDVARK_HPP_ */
