/*=========================================================================
| Aardvark Interface Library
|--------------------------------------------------------------------------
| Copyright (c) 2002-2008 Total Phase, Inc.
| All rights reserved.
| www.totalphase.com
|
| Redistribution and use in source and binary forms, with or without
| modification, are permitted provided that the following conditions
| are met:
|
| - Redistributions of source code must retain the above copyright
|   notice, this list of conditions and the following disclaimer.
|
| - Redistributions in binary form must reproduce the above copyright
|   notice, this list of conditions and the following disclaimer in the
|   documentation and/or other materials provided with the distribution.
|
| - Neither the name of Total Phase, Inc. nor the names of its
|   contributors may be used to endorse or promote products derived from
|   this software without specific prior written permission.
|
| THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
| "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
| LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
| FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE
| COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
| INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
| BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
| LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
| CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
| LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
| ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
| POSSIBILITY OF SUCH DAMAGE.
|--------------------------------------------------------------------------
| To access Aardvark devices through the API:
|
| 1) Use one of the following shared objects:
|      aardvark.so      --  Linux shared object
|      aardvark.dll     --  Windows dynamic link library
|
| 2) Along with one of the following language modules:
|      aardvark.c/h     --  C/C++ API header file and interface module
|      aardvark_py.py   --  Python API
|      aardvark.bas     --  Visual Basic 6 API
|      aardvark.cs      --  C# .NET source
|      aardvark_net.dll --  Compiled .NET binding
 ========================================================================*/


/*=========================================================================
| INCLUDES
 ========================================================================*/
/* This #include can be customized to conform to the user's build paths. */
#include "RemoteAardvark.hpp"
#include "document.h"


/*=========================================================================
| VERSION CHECK
 ========================================================================*/
#define AA_CFILE_VERSION   0x050a   /* v5.10 */
#define AA_REQ_SW_VERSION  0x050a   /* v5.10 */

/*
 * Make sure that the header file was included and that
 * the version numbers match.
 */
#ifndef AA_HEADER_VERSION
#  error Unable to include header file. Please check include path.

#elif AA_HEADER_VERSION != AA_CFILE_VERSION
#  error Version mismatch between source and header files.

#endif


/*=========================================================================
| DEFINES
 ========================================================================*/
#define API_NAME                     "libaardvark"
#define API_DEBUG                    AA_DEBUG
#define API_OK                       AA_OK
#define API_UNABLE_TO_LOAD_LIBRARY   AA_UNABLE_TO_LOAD_LIBRARY
#define API_INCOMPATIBLE_LIBRARY     AA_INCOMPATIBLE_LIBRARY
#define API_UNABLE_TO_LOAD_FUNCTION  AA_UNABLE_TO_LOAD_FUNCTION
#define API_HEADER_VERSION           AA_HEADER_VERSION
#define API_REQ_SW_VERSION           AA_REQ_SW_VERSION


/*=========================================================================
| LINUX AND DARWIN SUPPORT
 ========================================================================*/
#if defined(__APPLE_CC__) && !defined(DARWIN)
#define DARWIN
#endif

#if defined(linux) || defined(DARWIN)

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>

#ifdef DARWIN
#define DLOPEN_NO_WARN
extern int _NSGetExecutablePath (char *buf, unsigned long *bufsize);
#endif

#include <dlfcn.h>

#define DLL_HANDLE  void *
#define MAX_SO_PATH 256

static char SO_NAME[MAX_SO_PATH+1] = API_NAME ".so";

/*
 * These functions allow the Linux behavior to emulate
 * the Windows behavior as specified below in the Windows
 * support section.
 *
 * First, search for the shared object in the application
 * binary path, then in the current working directory.
 *
 * Searching the application binary path requires /proc
 * filesystem support, which is standard in 2.4.x kernels.
 *
 * If the /proc filesystem is not present, the shared object
 * will not be loaded from the execution path unless that path
 * is either the current working directory or explicitly
 * specified in LD_LIBRARY_PATH.
 */
static int _checkPath (const char *path) {
    char *filename = (char *)malloc(strlen(path) +1 + strlen(SO_NAME) +1);
    int   fd;

    // Check if the file is readable
    sprintf(filename, "%s/%s", path, SO_NAME);
    fd = open(filename, O_RDONLY);
    if (fd >= 0) {
        strncpy(SO_NAME, filename, MAX_SO_PATH);
        close(fd);
    }

    // Clean up and exit
    free(filename);
    return (fd >= 0);
}

static int _getExecPath (char *path, unsigned long maxlen) {
#ifdef linux
    return readlink("/proc/self/exe", path, maxlen);
#endif

#ifdef DARWIN
    _NSGetExecutablePath(path, &maxlen);
    return maxlen;
#endif
}

static void _setSearchPath () {
    char  path[MAX_SO_PATH+1];
    int   count;
    char *p;

    /* Make sure that SO_NAME is not an absolute path. */
    if (SO_NAME[0] == '/')  return;

    /* Check the execution directory name. */
    memset(path, 0, sizeof(path));
    count = _getExecPath(path, MAX_SO_PATH);

    if (count > 0) {
        char *p = strrchr(path, '/');
        if (p == path)  ++p;
        if (p != 0)     *p = '\0';

        /* If there is a match, return immediately. */
        if (_checkPath(path))  return;
    }

    /* Check the current working directory. */
    p = getcwd(path, MAX_SO_PATH);
    if (p != 0)  _checkPath(path);
}

#endif


/*=========================================================================
| WINDOWS SUPPORT
 ========================================================================*/
#if defined(WIN32) || defined(_WIN32)

#include <stdio.h>
#include <windows.h>

#define DLL_HANDLE           HINSTANCE
#define dlopen(name, flags)  LoadLibraryA(name)
#define dlsym(handle, name)  GetProcAddress(handle, name)
#define dlerror()            "Exiting program"
#define SO_NAME              API_NAME ".dll"

/*
 * Use the default Windows DLL loading rules:
 *   1.  The directory from which the application binary was loaded.
 *   2.  The application's current directory.
 *   3a. [Windows NT/2000/XP only] 32-bit system directory
 *       (default: c:\winnt\System32)
 *   3b. 16-bit system directory
 *       (default: c:\winnt\System or c:\windows\system)
 *   4.  The windows directory
 *       (default: c:\winnt or c:\windows)
 *   5.  The directories listed in the PATH environment variable
 */
static void _setSearchPath () {
    /* Do nothing */
}

#endif


/*=========================================================================
| SHARED LIBRARY LOADER
 ========================================================================*/
/* The error conditions can be customized depending on the application. */
static void *_loadFunction (const char *name, int *result) {
    static DLL_HANDLE handle = 0;
    void * function = 0;

    /* Load the shared library if necessary */
    if (handle == 0) {
        u32 (*version) (void);
        u16 sw_version;
        u16 api_version_req;

        _setSearchPath();
        handle = dlopen(SO_NAME, RTLD_LAZY);
        if (handle == 0) {
#if API_DEBUG
            fprintf(stderr, "Unable to load %s\n", SO_NAME);
            fprintf(stderr, "%s\n", dlerror());
#endif
            *result = API_UNABLE_TO_LOAD_LIBRARY;
            return 0;
        }

        version = reinterpret_cast<u32(*)(void)>((void*)dlsym(handle, "aa_c_version"));
        if (version == 0) {
#if API_DEBUG
            fprintf(stderr, "Unable to bind c_version() in %s\n",
                    SO_NAME);
            fprintf(stderr, "%s\n", dlerror());
#endif
            handle  = 0;
            *result = API_INCOMPATIBLE_LIBRARY;
            return 0;
        }

        sw_version      = (u16)((version() >>  0) & 0xffff);
        api_version_req = (u16)((version() >> 16) & 0xffff);
        if (sw_version  < API_REQ_SW_VERSION ||
            API_HEADER_VERSION < api_version_req)
        {
#if API_DEBUG
            fprintf(stderr, "\nIncompatible versions:\n");

            fprintf(stderr, "  Header version  = v%d.%02d  ",
                    (API_HEADER_VERSION >> 8) & 0xff, API_HEADER_VERSION & 0xff);

            if (sw_version < API_REQ_SW_VERSION)
                fprintf(stderr, "(requires library >= %d.%02d)\n",
                        (API_REQ_SW_VERSION >> 8) & 0xff,
                        API_REQ_SW_VERSION & 0xff);
            else
                fprintf(stderr, "(library version OK)\n");


            fprintf(stderr, "  Library version = v%d.%02d  ",
                    (sw_version >> 8) & 0xff,
                    (sw_version >> 0) & 0xff);

            if (API_HEADER_VERSION < api_version_req)
                fprintf(stderr, "(requires header >= %d.%02d)\n",
                        (api_version_req >> 8) & 0xff,
                        (api_version_req >> 0) & 0xff);
            else
                fprintf(stderr, "(header version OK)\n");
#endif
            handle  = 0;
            *result = API_INCOMPATIBLE_LIBRARY;
            return 0;
        }
    }

    /* Bind the requested function in the shared library */
    function = (void *)dlsym(handle, name);
    *result  = function ? API_OK : API_UNABLE_TO_LOAD_FUNCTION;
    return function;
}


/*=========================================================================
| FUNCTIONS
 ========================================================================*/




static int (*c_aa_find_devices) (int, u16*) = 0;

bool RemoteAardvark::aa_find_devices(Value &params, Value &result)
{
	int num_devices = 0;
	u16* devices = NULL;
	Value devicesArray;
	int res = 0;

	const char* paramsName = _aa_find_devices.paramArray[0]._name;
	Type paramType = _aa_find_devices.paramArray[0]._type;

	if(findObjectMember(params, paramsName, paramType))
	{
		num_devices = params[paramsName].GetInt();


		if (!(c_aa_find_devices = reinterpret_cast<int(*)(int,u16*)>(_loadFunction("c_aa_find_devices", &res))))
		{
			throw PluginError("Could not find symbol in shared library.",  __FILE__, __LINE__);
		}
		else
		{
			devices = new u16[num_devices];
			num_devices = c_aa_find_devices(num_devices, devices);


			result.SetObject();
			result.AddMember("num_devices", num_devices, dom.GetAllocator());
			devicesArray.SetArray();

			for(int i = 0 ; i < num_devices ; i++)
			{
				devicesArray.PushBack(devices[i], dom.GetAllocator());
			}
			delete[] devices;
			result.AddMember("devices", devicesArray, dom.GetAllocator());
		}
	}

	return true;
}

static int (*c_aa_find_devices_ext) (int, u16*, int, u32*) = 0;

bool RemoteAardvark::aa_find_devices_ext(Value &params, Value &result)
{
	int num_devices = 0;
	int num_ids = 0;
	u16* devices = NULL;
	u32* uniqueIds = NULL;

	Value devicesArray;
	Value uniqueIdsArray;

	int res = 0;

	const char* paramsName = _aa_find_devices_ext.paramArray[0]._name;
	Type paramType = _aa_find_devices_ext.paramArray[0]._type;

	if(findObjectMember(params, paramsName, paramType))
	{
		num_devices = params[paramsName].GetInt();
		num_ids = num_devices;


		if (!(c_aa_find_devices_ext = reinterpret_cast<int(*)(int,u16*, int, u32*)>(_loadFunction("c_aa_find_devices_ext", &res))))
		{
			throw PluginError("Could not find symbol in shared library.",  __FILE__, __LINE__);
		}
		else
		{
			devices = new u16[num_devices];
			uniqueIds = new u32[num_devices];
			for(int i = 0; i < num_devices; i++)
			{
				devices[i] = 0;
				uniqueIds[i] = 0;
			}
			num_devices = c_aa_find_devices_ext(num_devices, devices, num_ids, uniqueIds);
			num_ids = num_devices;

			result.SetObject();
			result.AddMember("num_devices", num_devices, dom.GetAllocator());
			devicesArray.SetArray();
			uniqueIdsArray.SetArray();

			for(int i = 0 ; i < num_devices ; i++)
			{
				devicesArray.PushBack(devices[i], dom.GetAllocator());
				uniqueIdsArray.PushBack((unsigned int)uniqueIds[i], dom.GetAllocator());
			}

			result.AddMember("devices", devicesArray, dom.GetAllocator());
			result.AddMember("num_ids", num_ids, dom.GetAllocator());
			result.AddMember("unique_ids", uniqueIdsArray, dom.GetAllocator());
			delete[] devices;
			delete [] uniqueIds;
		}
	}

	return true;
}


static Aardvark (*c_aa_open) (int) = 0;

bool RemoteAardvark::aa_open(rapidjson::Value &params , rapidjson::Value &result)
{
	int port_number = 0;
	int newHandle = 0;
	int res = 0;

	if(findObjectMember(params, "port"))
	{
		if(params["port"].IsInt())
		{
			port_number = params["port"].GetInt();
			if (!(c_aa_open = reinterpret_cast<Aardvark(*)(int)>(_loadFunction("c_aa_open", &res))))
			{
				throw PluginError("Could not find symbol in shared library.",  __FILE__, __LINE__);
			}
			else
			{
				newHandle = c_aa_open(port_number);
				result.SetObject();
				result.AddMember("Aardvark",newHandle, dom.GetAllocator());
				handle = newHandle;
			}
		}
		else throw PluginError("Member \"port\" has to be an integer type.",  __FILE__, __LINE__);
	}
	return true;
}


static Aardvark (*c_aa_open_ext) (int, AardvarkExt*) = 0;

bool RemoteAardvark::aa_open_ext(rapidjson::Value &params , rapidjson::Value &result)
{
	AardvarkExt aardvarkExt;
	Value aardvarkVersionValue;
	Value aardvarkExtValue;

	int port_number = 0;
	int newHandle = 0;
	int res = 0;


		if(findObjectMember(params, "port"))
		{
			if(params["port"].IsInt())
			{
				port_number = params["port"].GetInt();
				if (!(c_aa_open_ext = reinterpret_cast<Aardvark(*)(int, AardvarkExt*)>(_loadFunction("c_aa_open_ext", &res))))
				{
					throw PluginError("Could not find symbol in shared library.",  __FILE__, __LINE__);
				}
				else
				{
					newHandle = c_aa_open_ext(port_number, &aardvarkExt);

					result.SetObject();
					result.AddMember("Aardvark", newHandle, dom.GetAllocator());
					handle = newHandle;

					//create Aardvarkversion object
					aardvarkVersionValue.SetObject();
					aardvarkVersionValue.AddMember("software", aardvarkExt.version.software, dom.GetAllocator());
					aardvarkVersionValue.AddMember("firmware", aardvarkExt.version.firmware, dom.GetAllocator());
					aardvarkVersionValue.AddMember("hardware", aardvarkExt.version.hardware, dom.GetAllocator());
					aardvarkVersionValue.AddMember("sw_req_by_fw", aardvarkExt.version.sw_req_by_fw, dom.GetAllocator());
					aardvarkVersionValue.AddMember("fw_req_by_sw", aardvarkExt.version.fw_req_by_sw, dom.GetAllocator());
					aardvarkVersionValue.AddMember("api_req_by_sw", aardvarkExt.version.api_req_by_sw, dom.GetAllocator());

					//create AardvarkExt object
					aardvarkExtValue.SetObject();
					//add Aardvarkversion-object
					aardvarkExtValue.AddMember("AardvarkVersionValue", aardvarkVersionValue, dom.GetAllocator());
					//add features
					aardvarkExtValue.AddMember("features", aardvarkExt.features, dom.GetAllocator());

					//add AarvarkExt to result object
					result.AddMember("AardvarkExt", aardvarkExtValue, dom.GetAllocator());
				}
			}
			else throw PluginError("Member \"port\" has to be an integer type.",  __FILE__, __LINE__);
		}
	return true;
}




static int (*c_aa_close) (Aardvark) = 0;

bool RemoteAardvark::aa_close(rapidjson::Value &params , rapidjson::Value &result)
{
	Aardvark aardvark;
	int res = 0;

	if(findObjectMember(params, "handle"))
	{
		if(params["handle"].IsInt())
		{
			aardvark = params["handle"].GetInt();
			if (!(c_aa_close = reinterpret_cast<int(*)(Aardvark)>(_loadFunction("c_aa_close", &res))))
			{
				throw PluginError("Could not find symbol in shared library.",  __FILE__, __LINE__);
			}
			else
			{
				result.SetInt(c_aa_close(aardvark));
			}
		}
		else throw PluginError("Member \"handle\" has to be an integer type.",  __FILE__, __LINE__);
	}
	return false;
}


static int (*c_aa_port) (Aardvark) = 0;

bool RemoteAardvark::aa_port(Value &params, Value &result)
{
	int res = 0;
	int tempHandle = 0;
	int tempPort = 0;

	const char* paramsName = _aa_port.paramArray[0]._name;
	Type paramType = _aa_port.paramArray[0]._type;

	if(findObjectMember(params, paramsName, paramType))
	{

		tempHandle = params[paramsName].GetInt();
		if (!(c_aa_port = reinterpret_cast<int(*)(Aardvark)>(_loadFunction("c_aa_port", &res))))
		{
			throw PluginError("Could not find symbol in shared library.",  __FILE__, __LINE__);
		}
		else
		{
			tempPort = c_aa_port(tempHandle);
			result.SetInt(tempPort);
		}

	}
	return true;
}


static int (*c_aa_features) (Aardvark) = 0;

bool RemoteAardvark::aa_features(Value &params, Value &result)
{
	int res = 0;
	int tempHandle = 0;
	int features = 0;

	const char* paramsName = _aa_features.paramArray[0]._name;
	Type paramType = _aa_features.paramArray[0]._type;

	if(findObjectMember(params, paramsName, paramType))
	{

		tempHandle = params[paramsName].GetInt();
		if (!(c_aa_features = reinterpret_cast<int(*)(Aardvark)>(_loadFunction("c_aa_features", &res))))
		{
			throw PluginError("Could not find symbol in shared library.",  __FILE__, __LINE__);
		}
		else
		{
			features = c_aa_features(tempHandle);
			result.SetInt(features);
		}

	}
	return true;
}





static u32 (*c_aa_unique_id) (Aardvark) = 0;

bool RemoteAardvark::aa_unique_id(Value &params , Value &result)
{
	Aardvark aardvark;
	u32 id;
	int res = 0;

	const char* paramsName = _aa_unique_id.paramArray[0]._name;
	Type paramType = _aa_unique_id.paramArray[0]._type;

	if(findObjectMember(params, paramsName, paramType))
	{
		aardvark = params[paramsName].GetInt();
		if (!(c_aa_unique_id = reinterpret_cast<u32(*)(Aardvark)>(_loadFunction("c_aa_unique_id", &res))))
		{
			throw PluginError("Could not find symbol in shared library.",  __FILE__, __LINE__);
		}
		else
		{
			id = c_aa_unique_id(aardvark);
			result.SetUint(id);
		}

	}
	return true;
}

static const char* (*c_aa_status_string) (Aardvark) = 0;

bool RemoteAardvark::aa_status_string(Value &params, Value &result)
{
	int res = 0;
	int status = 0;
	const char* statusString = NULL;

	const char* paramsName = _aa_status_string.paramArray[0]._name;
	Type paramType = _aa_status_string.paramArray[0]._type;

	if(findObjectMember(params, paramsName, paramType))
	{
		status = params[paramsName].GetInt();
		if (!(c_aa_status_string = reinterpret_cast<const char*(*)(Aardvark)>(_loadFunction("c_aa_status_string", &res))))
		{
			throw PluginError("Could not find symbol in shared library.",  __FILE__, __LINE__);
		}
		else
		{
			statusString = c_aa_status_string(status);
			result.SetString(statusString, dom.GetAllocator());
		}

	}
	return true;
}


static int (*c_aa_version)(Aardvark, AardvarkVersion*) = 0;

bool RemoteAardvark::aa_version(Value &params, Value &result)
{

	int res = 0;
	int tempHandle = 0;
	int tempPort = 0;
	AardvarkVersion version;
	Value aardvarkVersionValue;

	const char* paramsName = _aa_version.paramArray[0]._name;
	Type paramType = _aa_version.paramArray[0]._type;

	if(findObjectMember(params, paramsName, paramType))
	{

		tempHandle = params[paramsName].GetInt();
		if (!(c_aa_version = reinterpret_cast<int(*)(Aardvark, AardvarkVersion*)>(_loadFunction("c_aa_version", &res))))
		{
			throw PluginError("Could not find symbol in shared library.",  __FILE__, __LINE__);
		}
		else
		{
			tempPort = c_aa_version(tempHandle, &version);
			//create Aardvarkversion object
			aardvarkVersionValue.SetObject();
			aardvarkVersionValue.AddMember("software", version.software, dom.GetAllocator());
			aardvarkVersionValue.AddMember("firmware", version.firmware, dom.GetAllocator());
			aardvarkVersionValue.AddMember("hardware", version.hardware, dom.GetAllocator());
			aardvarkVersionValue.AddMember("sw_req_by_fw", version.sw_req_by_fw, dom.GetAllocator());
			aardvarkVersionValue.AddMember("fw_req_by_sw", version.fw_req_by_sw, dom.GetAllocator());
			aardvarkVersionValue.AddMember("api_req_by_sw", version.api_req_by_sw, dom.GetAllocator());

			result.SetObject();
			result.AddMember("version", aardvarkVersionValue, dom.GetAllocator());

		}

	}
	return true;


}



static int (*c_aa_target_power) (Aardvark, u08) = 0;

bool RemoteAardvark::aa_target_power(Value &params , Value &result)
{
	Aardvark aardvark;
	u08 power_mask;
	int returnValue = 0;
	int res = 0;

	const char* paramsName = _aa_target_power.paramArray[0]._name;
	Type paramType = _aa_target_power.paramArray[0]._type;

	findObjectMember(params, paramsName, paramType);

	aardvark = params[paramsName].GetInt();

	paramsName = _aa_target_power.paramArray[1]._name;
	paramType = _aa_target_power.paramArray[1]._type;
	findObjectMember(params, paramsName, paramType);
	power_mask = params[paramsName].GetInt();

	if (!(c_aa_target_power = reinterpret_cast<int(*)(Aardvark, u08)>(_loadFunction("c_aa_target_power", &res))))
	{
		throw PluginError("Could not find symbol in shared library.",  __FILE__, __LINE__);
	}
	else
	{
		returnValue = c_aa_target_power(aardvark, power_mask);
		result.SetObject();
		result.AddMember("returnCode", returnValue, dom.GetAllocator());
	}

	return true;
}



static int (*c_aa_i2c_write) (Aardvark, u16, AardvarkI2cFlags, u16, const u08 *) = 0;

bool RemoteAardvark::aa_i2c_write(Value &params, Value &result)
{
	Aardvark aardvark;
	u16 slaveAddress = 0x0000;
	AardvarkI2cFlags flags = AA_I2C_NO_FLAGS;
	u16 numberOfBytes = 0x0000;
	u08* data = NULL;

	Value array;
	Value* tempValue = NULL;
	int res = 0;
	int returnValue = 0;


	//get handle
	const char* paramName = _aa_i2c_write.paramArray[0]._name;
	Type paramType = _aa_i2c_write.paramArray[0]._type;
	tempValue = findObjectMember(params, paramName, paramType);
	aardvark = tempValue->GetInt();

	//get slave addr
	paramName = _aa_i2c_write.paramArray[1]._name;
	paramType = _aa_i2c_write.paramArray[1]._type;
	tempValue = findObjectMember(params, paramName, paramType);
	slaveAddress = tempValue->GetUint();

	//get flags
	paramName = _aa_i2c_write.paramArray[2]._name;
	paramType = _aa_i2c_write.paramArray[2]._type;
	tempValue = findObjectMember(params, paramName, paramType);
	flags = getAardvarkI2cFlag(tempValue->GetUint());

	//get num bytes
	paramName = _aa_i2c_write.paramArray[3]._name;
	paramType = _aa_i2c_write.paramArray[3]._type;
	numberOfBytes = tempValue->GetUint();
	data = new u08[numberOfBytes];


	//get data_out
	paramName = _aa_i2c_write.paramArray[4]._name;
	paramType = _aa_i2c_write.paramArray[4]._type;
	tempValue = findObjectMember(params, paramName, paramType);
	for(int i = 0; i < numberOfBytes; i++)
			data[i] = tempValue[i].GetInt();



	if (!(c_aa_i2c_write = reinterpret_cast<int(*)(Aardvark, u16, AardvarkI2cFlags, u16, const u08 *)>(_loadFunction("c_aa_i2c_write", &res))))
	{
		throw PluginError("Could not find symbol in shared library.",  __FILE__, __LINE__);
	}
	else
	{
		returnValue = c_aa_i2c_write(aardvark, slaveAddress, flags, numberOfBytes, data);
		result.SetObject();
		result.AddMember("returnCode", returnValue, dom.GetAllocator());
	}

	delete[] data;


	return true;
}

