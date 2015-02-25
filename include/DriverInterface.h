/*
 * Plugin_Interface.h
 *
 *  Created on: 21.01.2015
 *      Author: dnoack
 */

#ifndef DRIVERINTERFACE_H_
#define DRIVERINTERFACE_H_


#include <map>
#include <cstring>
#include "document.h"
#include "writer.h"
#include "Plugin_Error.h"

using namespace std;
using namespace rapidjson;



template <class TDriver, class TPointer>
class DriverInterface{

	public:
		DriverInterface(TDriver derivedClass)
		{
			driver = derivedClass;
		};

		~DriverInterface()
		{
			funcMap.clear();
		};

		bool executeFunction(Value &method, Value &params, Value &result)
		{
			try{
				funcP = funcMap[(char*)method.GetString()];
				if(funcP == NULL)
					throw PluginError("Function not found.",  __FILE__, __LINE__);
				else
					return (driver->*funcP)(params, result);
			}
			catch(const PluginError &e)
			{
				throw;
			}
		}



		bool findParamsMember(Value &object, char* memberName)
		{
			Type paramsType;

			paramsType = object.GetType();
			if(paramsType == kObjectType)
			{
				if(object.HasMember(memberName))
				{
					return true;
				}
				else
					throw PluginError("Needed member not found.",  __FILE__, __LINE__);
			}
			else
				throw PluginError("Params is not an Object.",  __FILE__, __LINE__);

			return false;
		}

	protected:

		struct cmp_keys
		{
			bool operator()(char const* input, char const* key)
			{
				return strcmp(input, key) < 0;
			}
		};


		//use char* for keys, because rapidjson gives us char* and we save a lot of allocating new strings
		map<char*, TPointer, DriverInterface::cmp_keys> funcMap;
		TPointer funcP;
		TDriver driver;

};



#endif /* PLUGIN_ORIGIN_INCLUDE_PLUGIN_INTERFACE_H_ */
