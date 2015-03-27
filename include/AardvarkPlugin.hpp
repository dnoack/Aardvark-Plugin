/*
 * AardvarkPlugin.hpp
 *
 *  Created on: 25.02.2015
 *      Author: Dave
 */

#ifndef AARDVARKPLUGIN_HPP_
#define AARDVARKPLUGIN_HPP_

#define REG_PATH "/tmp/RsdRegister.uds"
#define COM_PATH "/tmp/AardvarkPlugin.uds"
#define PLUGIN_NAME "Aardvark"
#define PLUGIN_NUMBER 1

#include "UdsServer.hpp"
#include "UdsRegClient.hpp"


class AardvarkPlugin {

	public:
		AardvarkPlugin();
		~AardvarkPlugin();


		void startCommunication();
		void registerToRSD();

		static list<string*>* getFuncList(){return funcList;}


	private:
		UdsServer* comServer;
		UdsRegClient* regClient;

		bool regClientReady;
		bool comServerReady;
		bool pluginActive;

		static list<string*>* funcList;



};

#endif /* AARDVARKPLUGIN_HPP_ */
