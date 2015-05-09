
#ifndef INCLUDE_AARDVARKPLUGIN_HPP_
#define INCLUDE_AARDVARKPLUGIN_HPP_

#define REG_PATH "/tmp/RsdRegister.uds"
#define COM_PATH "/tmp/AardvarkPlugin.uds"
#define PLUGIN_NAME "Aardvark"
#define PLUGIN_NUMBER 1
#define WAIT_TIME 3

#include "UdsServer.hpp"
#include "UdsRegClient.hpp"
#include "RemoteAardvark.hpp"
#include "AardvarkCareTaker.hpp"



class AardvarkPlugin {

	public:
		AardvarkPlugin();
		~AardvarkPlugin();

		static list<string*>* getFuncList(){return funcList;}

		void start();


	private:

		static list<string*>* funcList;

		UdsServer* comServer;
		UdsRegClient* regClient;
		sigset_t sigmask;
		sigset_t origmask;

		bool pluginActive;

		void deleteFuncList();

};

#endif /* INCLUDE_AARDVARKPLUGIN_HPP_ */
