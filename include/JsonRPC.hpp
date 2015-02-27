/*
 * JsonRPC.h
 *
 *  Created on: 16.01.2015
 *      Author: dnoack
 */

#ifndef SRC_JSONRPC_H_
#define SRC_JSONRPC_H_

#define JSON_PROTOCOL_VERSION "2.0"


#include <map>
#include <vector>


//rapdijson includes
#include "document.h"
#include "writer.h"


using namespace std;
using namespace rapidjson;




class JsonRPC {

	public:

		JsonRPC()
		{
			this->currentValue = NULL;
			this->result = NULL;
			this->error = NULL;


			jsonWriter = new Writer<StringBuffer>(sBuffer);
			inputDOM = new Document();
			requestDOM = new Document();
			responseDOM = new Document();
			errorDOM = new Document();


			generateRequestDOM(*requestDOM);
			generateResponseDOM(*responseDOM);
			generateErrorDOM(*errorDOM);
		};



		~JsonRPC()
		{
			delete jsonWriter;
			delete inputDOM;
			delete requestDOM;
			delete responseDOM;
			delete errorDOM;
		};



		/**
		 * Checks if the json rpc msg has the mandatory members (jsonrpc and method).
		 * It also calls checkJsonRpcVersion.
		 */
		bool checkJsonRpc_RequestFormat();

		/**
		 * Checks if the json rpc msg member "jsonrpc" has the correct protocol version.
		 */
		bool checkJsonRpcVersion(Document &dom);


		/**
		 * Checks if there is a member named "id". If not the msg is assumed to be
		 * a notification. If "id" is existing, the msg is a request. For checking the other
		 * mandatory request fields, you need to call checkJsonRpc_RequestFormat().
		 */
		bool isRequest();


		Document* parse(string* msg);

		char* generateRequest(Value &method, Value &params, Value &id);

		char* generateResponse(Value &id, Value &response);

		Document* getRequestDOM() { return this->inputDOM;}
		Document* getResponseDOM() { return this->responseDOM;}
		Document* getErrorDOM(){ return this->errorDOM;}


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


		//represents the current jsonrpc msg as dom (document object model)
		Document* inputDOM;
		Document* requestDOM;
		Document* responseDOM;
		Document* errorDOM;

		//Value from dom which is currently examined
		Value currentValue;

		//result from the processed function
		Value result;
		char* responseMsg;
		char* error;



		char* generateResponseError(Value &id, int code, char* msg);

		void generateRequestDOM(Document &dom);

		void generateResponseDOM(Document &dom);

		void generateErrorDOM(Document &dom);


};



#endif /* SRC_JSONRPC_H_ */
