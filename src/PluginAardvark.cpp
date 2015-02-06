/*
 * PluginAardvark.cpp
 *
 *  Created on: 05.02.2015
 *      Author: dnoack
 */


#include "PluginAardvark.hpp"

PluginAardvark::PluginAardvark()
{
	detectDevices();
	this->uds_reg = new UdsServer(SERVER_MODE, UDS_REGISTER_TO_RSD_PATH, sizeof(UDS_REGISTER_TO_RSD_PATH));
	uds_reg->startCom();
}



PluginAardvark::~PluginAardvark()
{
	for(unsigned int i = 0; i < deviceList.size(); i++)
		delete deviceList[i];
}




void PluginAardvark::startListening()
{
	uds_com = new UdsServer(SERVER_MODE, UDS_COM_PATH, sizeof(UDS_COM_PATH));
}



void PluginAardvark::detectDevices()
{
	u16 devices[EXPECTED_NUM_OF_DEVICES];
	u32 unique_ids[EXPECTED_NUM_OF_DEVICES];

	aa_find_devices_ext(EXPECTED_NUM_OF_DEVICES, devices, EXPECTED_NUM_OF_DEVICES, unique_ids);

	for(int i = 0; i < EXPECTED_NUM_OF_DEVICES; i++)
	{
		if(unique_ids[i] != 0)
		{
			//devices will be deleted within destructor
			deviceList.push_back(new RemoteAardvark());
		}
	}
}


int main(int argc, char** argv)
{
	PluginAardvark* test = new PluginAardvark();

	while(1)
		sleep(3);
	delete test;
}

