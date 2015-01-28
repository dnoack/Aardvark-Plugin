/*
 * Plugin_Interface.h
 *
 *  Created on: 21.01.2015
 *      Author: dnoack
 */

#ifndef PLUGIN_ORIGIN_INCLUDE_PLUGIN_INTERFACE_H_
#define PLUGIN_ORIGIN_INCLUDE_PLUGIN_INTERFACE_H_


#include <map>
#include <cstring>
#include "document.h"
#include "writer.h"

using namespace std;
using namespace rapidjson;

#define FREE_IDENTITY NULL


template <class TDriver, class TPointer>
class PluginInterface{

	public:
		PluginInterface(TDriver derivedClass)
		{
			driver = derivedClass;
			currentIdentity = NULL;
			error = NULL;
		};

		~PluginInterface(){};

		bool executeFunction(Value &method, Value &params, Value &result, char** error)
		{
			this->error = error;
			funcP = funcMap[(char*)method.GetString()];
			return (driver->*funcP)(params, result);
		}


		string* getIdentity(){return currentIdentity;}


		void setIdentity(string* identity)
		{
			//parameter not null
			if(identity != FREE_IDENTITY)
			{
				currentIdentity = new string(*identity);
			}
			//parameter null -> free memory, set pointer null
			else
			{
				if(currentIdentity != NULL)
				{
					delete currentIdentity;
					currentIdentity = NULL;
				}
			}
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
		map<char*, TPointer, PluginInterface::cmp_keys> funcMap;
		TPointer funcP;
		TDriver driver;
		string* currentIdentity;
		char** error;


};



#endif /* PLUGIN_ORIGIN_INCLUDE_PLUGIN_INTERFACE_H_ */
