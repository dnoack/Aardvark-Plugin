/*
 * JsonRPC.h
 *
 *  Created on: 16.01.2015
 *      Author: dnoack
 */

#ifndef SRC_JSONRPC_H_
#define SRC_JSONRPC_H_

#define JSON_PROTOCOL_VERSION "2.0"

#include <Aardvark.hpp>
#include <map>
#include <vector>


//rapdijson includes
#include "document.h"
#include "writer.h"


using namespace std;
using namespace rapidjson;



namespace Plugin{


class JsonRPC {

	public:

		JsonRPC()
		{
			this->currentValue = NULL;
			this->result = NULL;


			jsonWriter = new Writer<StringBuffer>(sBuffer);
			responseDOM = new Document();
			errorDOM = new Document();

			generateResponseDOM(*responseDOM);
			generateErrorDOM(*errorDOM);
			detectDevices();
		};



		~JsonRPC()
		{
			delete &sBuffer;
			delete jsonWriter;
			delete responseDOM;
			delete errorDOM;
			deviceList.clear();
		};


		//receive json-rpc msg, check if it is a request, process
		char* handle(string* request, string* identity);

		bool checkJsonRpcFormat(Document* dom);

		bool checkJsonRpcVersion(Document* dom);


	private:

		//compare functor handels char* compare for the map
		struct cmp_keys
		{
			bool operator()(char const* input, char const* key)
			{
				return strcmp(input, key) < 0;
			}
		};

		StringBuffer sBuffer;
		Writer<StringBuffer>* jsonWriter;
		vector<RemoteAardvark*> deviceList;

		//represents the current jsonrpc msg as dom (document object model)
		Document* requestDOM;
		Document* responseDOM;
		Document* errorDOM;

		//Value from dom which is currently examined
		Value currentValue;

		//result from the processed function
		Value result;
		char* responseMsg;
		char* error;


		//lookup for function
		char* process(Value &method, Value &params, Value &id, string* identity);

		char* response(Value &id);

		char* responseError(Value &id, int code, char* msg);

		void generateResponseDOM(Document &dom);

		void generateErrorDOM(Document &dom);

		void detectDevices();

};

}; //namespace Plugin


#endif /* SRC_JSONRPC_H_ */
