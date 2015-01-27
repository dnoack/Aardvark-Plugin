/*
 * test.c
 *
 *  Created on: 16.01.2015
 *      Author: dnoack
 */

#include <JsonRPC.hpp>
#include "stdio.h"



//this is how a driver method should work
/*add expect 2 integer value. The two value are transported trough json rpc with a value containing an array:
 * { "params" : [A, B]} Through previous methods we got the value with name "params".
 * Now the method have to check if there is an array AND if there are two numbers in it.
 * If yes -> good, add them and return a result
 * If no -> bad, return a error rpc msg
 */
static bool add(Value &params , Value &result)
{
	static const char* kTypeNames[] = {"Null", "False", "True", "Object", "Array", "String", "Number"};

	int numberOne = 0;
	int numberTwo = 0;
	int arraySize = 0;

	try{
		//check the presence of the array
		if(params.IsArray())
		{
			//check if there are two fields
			if(params.Size() == 2)
			{
				//check if the fields are of a numbertype
				if(params[0].GetType() == kNumberType && params[1].GetType() == kNumberType)
				{
					numberOne = params[0].GetInt();
					numberTwo = params[1].GetInt();

					//add them and fill the result value
					result.SetInt(numberOne + numberTwo);
					printf("Add: %d + %d\n", numberOne, numberTwo);
					printf("Result: %d\n", result.GetInt());


				}
				else
					throw -1; //one or both fields are no number
			}
			else
				throw -2; //size of the array is not correct
		}
		else
			throw -3; //There is no array
	}
	catch(int e)
	{
		printf(" Exception occured within Method \"add\": %d\n", e);
		return false;
	}

	//return the result as value
	return true;
}

static void localTest(Plugin::JsonRPC* server)
{
	//params ok
	char* testRequest = "{\"jsonrpc\": \"2.0\", \"params\": [5,9], \"method\": \"add\", \"id\": 1}";
	//char* instead of int
	char* testRequest2 = "{\"jsonrpc\": \"2.0\", \"params\": [\"hallo\",9, 5], \"method\": \"add\", \"id\": 1}";

	char* testRequest3 = "{\"jsonrpc\": \"2.0\", \"params\": [true,9], \"method\": \"add\", \"id\": 1}";

	char* testRequest4 = "{\"jsonrpc\": \"2.0\", \"params\": \"noArray\", \"method\": \"add\", \"id\": 1}";

	char* testResponse;

	/*server->handle(testRequest, testResponse);
	server->handle(testRequest2, testResponse);
	server->handle(testRequest3, testResponse);
	server->handle(testRequest4, testResponse);*/

}


/*
int main(void)
{
	Plugin::JsonRPC* server = new Plugin::JsonRPC();
	server->registerFunction("add", &add);
	delete(server);



	return 0;
}*/



