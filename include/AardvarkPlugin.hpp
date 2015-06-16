#ifndef INCLUDE_AARDVARKPLUGIN_H_
#define INCLUDE_AARDVARKPLUGIN_H_

#include "PluginInterface.hpp"

/*! Path to uds file for registring the plugin to RSD.*/
#define REG_PATH "/tmp/RsdRegister.uds"
/*! Path to uds file for communication (except registring), this has to be a unique path for every plugin.*/
#define COM_PATH "/tmp/AardvarkPlugin.uds"
/*! Plugin name, every rpc method for this plugin has to begin with this name, the name has to be unique.*/
#define PLUGIN_NAME "Aardvark"
/*! Unique plugin number.*/
#define PLUGIN_NUMBER 1

class AardvarkPlugin : public PluginInterface{

	public:
		AardvarkPlugin(PluginInfo* pluginInfo);
		virtual ~AardvarkPlugin();

		void thread_accept();
};

#endif /* INCLUDE_AARDVARKPLUGIN_H_ */
