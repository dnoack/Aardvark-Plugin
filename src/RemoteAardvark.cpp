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
#include <RemoteAardvark.hpp>
#include <Plugin_Error.h>



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

        version = reinterpret_cast<u32(*)(void)>((void *)dlsym(handle, "c_version"));
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

static int (*c_aa_find_devices_ext) (int, u16 *, int, u32 *) = 0;
int aa_find_devices_ext (
    int  num_devices,
    u16* devices,
    int  num_ids,
    u32* unique_ids
)
{
    if (c_aa_find_devices_ext == 0) {
        int res = 0;
        if (!(c_aa_find_devices_ext = reinterpret_cast<int(*)(int,u16*,int,u32*)>(_loadFunction("c_aa_find_devices_ext", &res))))
            return res;
    }
    return c_aa_find_devices_ext(num_devices, devices, num_ids, unique_ids);
}








static Aardvark (*c_aa_open) (int) = 0;
Aardvark aa_open (
    int port_number
)
{
    if (c_aa_open == 0) {
        int res = 0;
        if (!(c_aa_open = reinterpret_cast<Aardvark(*)(int)>(_loadFunction("c_aa_open", &res))))
            return res;
    }
    return c_aa_open(port_number);
}

//remote function
bool RemoteAardvark::aa_open(rapidjson::Value &params , rapidjson::Value &result)
{
	int port_number = 0;
	int fail = 0;
	Value member;

		if(findParamsMember(params, "port"))
		{
			if(params["port"].IsInt())
			{
				port_number = params["port"].GetInt();
				if (!(c_aa_open = reinterpret_cast<Aardvark(*)(int)>(_loadFunction("c_aa_open", &fail))))
				{
					throw PluginError("Could not find symbol in shared library.",  __FILE__, __LINE__);
				}
				else
				{
					result.SetInt(c_aa_open(port_number));
				}
			}
			else throw PluginError("Member \"port\" has to be an integer type.",  __FILE__, __LINE__);
		}
	return true;
}




static int (*c_aa_close) (Aardvark) = 0;
int aa_close (
    Aardvark aardvark
)
{
    if (c_aa_close == 0) {
        int res = 0;
        if (!(c_aa_close = reinterpret_cast<int(*)(Aardvark)>(_loadFunction("c_aa_close", &res))))
            return res;
    }
    return c_aa_close(aardvark);
}


bool RemoteAardvark::aa_close(rapidjson::Value &params , rapidjson::Value &result)
{
	Aardvark aardvark;
	int res = 0;

	if(findParamsMember(params, "handle"))
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


static u32 (*c_aa_unique_id) (Aardvark) = 0;
u32 aa_unique_id (
    Aardvark aardvark
)
{
    if (c_aa_unique_id == 0) {
        int res = 0;
        if (!(c_aa_unique_id = reinterpret_cast<u32(*)(Aardvark)>(_loadFunction("c_aa_unique_id", &res))))
            return res;
    }
    return c_aa_unique_id(aardvark);
}


bool RemoteAardvark::aa_unique_id(rapidjson::Value &params , rapidjson::Value &result)
{
	Aardvark aardvark;
	u32 id;
	int res = 0;

	if(findParamsMember(params, "handle"))
	{
		if(params["handle"].IsInt())
		{
			aardvark = params["handle"].GetInt();
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
		else
			throw PluginError("Member \"handle\" has to be an integer type.",  __FILE__, __LINE__);
	}
	return true;
}


static int (*c_aa_target_power) (Aardvark, u08) = 0;
int aa_target_power (
    Aardvark aardvark,
    u08      power_mask
)
{
    if (c_aa_target_power == 0) {
        int res = 0;
        if (!(c_aa_target_power = reinterpret_cast<int(*)(Aardvark, u08)>(_loadFunction("c_aa_target_power", &res))))
            return res;
    }
    return c_aa_target_power(aardvark, power_mask);
}


bool RemoteAardvark::aa_target_power(rapidjson::Value &params , rapidjson::Value &result)
{
	Aardvark aardvark;
	u08 power_mask;
	int res = 0;


	if(findParamsMember(params, "handle") && findParamsMember(params, "powerMask"))
	{
		if(params["handle"].IsInt() )
		{
			if(params["powerMask"].IsUint())
			{
				aardvark = params["handle"].GetInt();
				power_mask = params["powerMask"].GetInt();

				if (!(c_aa_target_power = reinterpret_cast<int(*)(Aardvark, u08)>(_loadFunction("c_aa_target_power", &res))))
				{
					throw PluginError("Could not find symbol in shared library.",  __FILE__, __LINE__);
				}
				else
				{
					c_aa_target_power(aardvark, power_mask);
					result.SetUint(5);
				}
			}
			else
				throw PluginError("Member \"powerMask\" has to be an unsigned integer.",  __FILE__, __LINE__);
		}
		else
			throw PluginError("Member \"handle\" has to be an integer.",  __FILE__, __LINE__);
	}
	return true;
}

