/*
 * Plugin_Error.h
 *
 *  Created on: 28.01.2015
 *      Author: dnoack
 */

#ifndef INCLUDE_PLUGIN_ERROR_H_
#define INCLUDE_PLUGIN_ERROR_H_

#include <stdexcept>
#include <string>
#include <sstream>

using namespace std;

class PluginError : public runtime_error{


	public:
		PluginError(const string &msg, char* file, int line) : runtime_error(msg)
		{
			this->msg = &msg;
			this->file = file;
			this->line = line;
			msgOut = new string();
		}


		~PluginError() throw()
		{
			delete(msgOut);
		}


		string* get() const throw()
		{
			ostringstream oStream;
			oStream << "An error was thrown in file: " << file << " at line: " << line << " ### " << *msg ;
			*msgOut = oStream.str();
			return msgOut;
		}


	private:
		const string* msg;
		char* file;
		int line;
		string* msgOut;



};

//string PluginError::msgOut;

#define throw_PluginError(msg) throw PluginError(msg, __FILE__, __LINE__);



#endif /* INCLUDE_PLUGIN_ERROR_H_ */
