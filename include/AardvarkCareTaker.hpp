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


class AardvarkCareTaker{

	public:
		AardvarkCareTaker();
		~AardvarkCareTaker();

		//valueType can be PORT or HANDLE
		RemoteAardvark* getDevice(int value, int valueType);

		string* processMsg(string* msg);

	private:

		static vector<RemoteAardvark*> deviceList;
		static pthread_mutex_t dLmutex;
		JsonRPC* json;
		string* result;
		string* user;


};




#endif /* INCLUDE_PLUGINAARDVARK_HPP_ */
