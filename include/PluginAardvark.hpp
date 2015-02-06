/*
 * PlugingAardvark.hpp
 *
 *  Created on: 05.02.2015
 *      Author: dnoack
 */

#ifndef INCLUDE_PLUGINAARDVARK_HPP_
#define INCLUDE_PLUGINAARDVARK_HPP_


#define UDS_REGISTER_TO_RSD_PATH "/tmp/RsdRegister.uds"
#define UDS_COM_PATH "/tmp/AardvarkPlugin.uds"
#define EXPECTED_NUM_OF_DEVICES 1 //TODO: Read from textfile or argument

#include <vector>
#include "UdsServer.hpp"
#include "Aardvark.hpp"


class PluginAardvark{

	public:
		PluginAardvark();
		~PluginAardvark();

		void startListening();

	private:
		UdsServer* uds_reg;
		UdsServer* uds_com;
		vector<RemoteAardvark*> deviceList;
		char buffer[BUFFER_SIZE];
		void detectDevices();


};




#endif /* INCLUDE_PLUGINAARDVARK_HPP_ */
