/*
 * PlugingAardvark.hpp
 *
 *  Created on: 05.02.2015
 *      Author: dnoack
 */

#ifndef INCLUDE_PLUGINAARDVARK_HPP_
#define INCLUDE_PLUGINAARDVARK_HPP_

#define EXPECTED_NUM_OF_DEVICES 1


#include <vector>
#include "JsonRPC.hpp"
#include "Aardvark.hpp"


class PluginAardvark{

	public:
		PluginAardvark();
		~PluginAardvark();

		RemoteAardvark* getDevice(int handle);
		void detectDevices();

		string* processMsg(string* msg);

	private:

		vector<RemoteAardvark*> deviceList;
		pthread_mutex_t dLmutex;
		JsonRPC* json;
		string* result;


};




#endif /* INCLUDE_PLUGINAARDVARK_HPP_ */
