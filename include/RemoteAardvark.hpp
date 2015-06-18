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


#ifndef __aardvark_h__
#define __aardvark_h__

#include <RPCInterface.hpp>
#include <stdio.h>


#include "JsonRPC.hpp"


using namespace rapidjson;

#ifdef __cplusplus
extern "C" {
#endif


/*=========================================================================
| TYPEDEFS
 ========================================================================*/
#ifndef TOTALPHASE_DATA_TYPES
#define TOTALPHASE_DATA_TYPES

#ifndef _MSC_VER
/* C99-compliant compilers (GCC) */
#include <stdint.h>
typedef uint8_t   u08;
typedef uint16_t  u16;
typedef uint32_t  u32;
typedef uint64_t  u64;
typedef int8_t    s08;
typedef int16_t   s16;
typedef int32_t   s32;
typedef int64_t   s64;

#else
/* Microsoft compilers (Visual C++) */
typedef unsigned __int8   u08;
typedef unsigned __int16  u16;
typedef unsigned __int32  u32;
typedef unsigned __int64  u64;
typedef signed   __int8   s08;
typedef signed   __int16  s16;
typedef signed   __int32  s32;
typedef signed   __int64  s64;

#endif /* __MSC_VER */

#endif /* TOTALPHASE_DATA_TYPES */


/*=========================================================================
| DEBUG
 ========================================================================*/
/* Set the following macro to '1' for debugging */
#define AA_DEBUG 0


/*=========================================================================
| VERSION
 ========================================================================*/
#define AA_HEADER_VERSION  0x050a   /* v5.10 */


/*=========================================================================
| STATUS CODES
 ========================================================================*/
/*
 * All API functions return an integer which is the result of the
 * transaction, or a status code if negative.  The status codes are
 * defined as follows:
 */
enum AardvarkStatus {
    /* General codes (0 to -99) */
    AA_OK                        =    0,
    AA_UNABLE_TO_LOAD_LIBRARY    =   -1,
    AA_UNABLE_TO_LOAD_DRIVER     =   -2,
    AA_UNABLE_TO_LOAD_FUNCTION   =   -3,
    AA_INCOMPATIBLE_LIBRARY      =   -4,
    AA_INCOMPATIBLE_DEVICE       =   -5,
    AA_COMMUNICATION_ERROR       =   -6,
    AA_UNABLE_TO_OPEN            =   -7,
    AA_UNABLE_TO_CLOSE           =   -8,
    AA_INVALID_HANDLE            =   -9,
    AA_CONFIG_ERROR              =  -10,

    /* I2C codes (-100 to -199) */
    AA_I2C_NOT_AVAILABLE         = -100,
    AA_I2C_NOT_ENABLED           = -101,
    AA_I2C_READ_ERROR            = -102,
    AA_I2C_WRITE_ERROR           = -103,
    AA_I2C_SLAVE_BAD_CONFIG      = -104,
    AA_I2C_SLAVE_READ_ERROR      = -105,
    AA_I2C_SLAVE_TIMEOUT         = -106,
    AA_I2C_DROPPED_EXCESS_BYTES  = -107,
    AA_I2C_BUS_ALREADY_FREE      = -108,

    /* SPI codes (-200 to -299) */
    AA_SPI_NOT_AVAILABLE         = -200,
    AA_SPI_NOT_ENABLED           = -201,
    AA_SPI_WRITE_ERROR           = -202,
    AA_SPI_SLAVE_READ_ERROR      = -203,
    AA_SPI_SLAVE_TIMEOUT         = -204,
    AA_SPI_DROPPED_EXCESS_BYTES  = -205,

    /* GPIO codes (-400 to -499) */
    AA_GPIO_NOT_AVAILABLE        = -400,

    /* I2C bus monitor codes (-500 to -599) */
    AA_I2C_MONITOR_NOT_AVAILABLE = -500,
    AA_I2C_MONITOR_NOT_ENABLED   = -501
};
#ifndef __cplusplus
typedef enum AardvarkStatus AardvarkStatus;
#endif


/*=========================================================================
| GENERAL TYPE DEFINITIONS
 ========================================================================*/
/* Aardvark handle type definition */
typedef int Aardvark;

/*
 * Deprecated type definitions.
 *
 * These are only for use with legacy code and
 * should not be used for new development.
 */
typedef u08 aa_u08;

typedef u16 aa_u16;

typedef u32 aa_u32;

typedef s08 aa_s08;

typedef s16 aa_s16;

typedef s32 aa_s32;

/*
 * Aardvark version matrix.
 *
 * This matrix describes the various version dependencies
 * of Aardvark components.  It can be used to determine
 * which component caused an incompatibility error.
 *
 * All version numbers are of the format:
 *   (major << 8) | minor
 *
 * ex. v1.20 would be encoded as:  0x0114
 */
struct AardvarkVersion {
    /* Software, firmware, and hardware versions. */
    u16 software;
    u16 firmware;
    u16 hardware;

    /* Firmware requires that software must be >= this version. */
    u16 sw_req_by_fw;

    /* Software requires that firmware must be >= this version. */
    u16 fw_req_by_sw;

    /* Software requires that the API interface must be >= this version. */
    u16 api_req_by_sw;
};
#ifndef __cplusplus
typedef struct AardvarkVersion AardvarkVersion;
#endif


/*=========================================================================
| GENERAL API
 ========================================================================*/
/*
 * Get a list of ports to which Aardvark devices are attached.
 *
 * nelem   = maximum number of elements to return
 * devices = array into which the port numbers are returned
 *
 * Each element of the array is written with the port number.
 * Devices that are in-use are ORed with AA_PORT_NOT_FREE (0x8000).
 *
 * ex.  devices are attached to ports 0, 1, 2
 *      ports 0 and 2 are available, and port 1 is in-use.
 *      array => 0x0000, 0x8001, 0x0002
 *
 * If the array is NULL, it is not filled with any values.
 * If there are more devices than the array size, only the
 * first nmemb port numbers will be written into the array.
 *
 * Returns the number of devices found, regardless of the
 * array size.
 */
#define AA_PORT_NOT_FREE 0x8000




/*
 * Open the Aardvark port, returning extended information
 * in the supplied structure.  Behavior is otherwise identical
 * to aa_open() above.  If 0 is passed as the pointer to the
 * structure, this function is exactly equivalent to aa_open().
 *
 * The structure is zeroed before the open is attempted.
 * It is filled with whatever information is available.
 *
 * For example, if the firmware version is not filled, then
 * the device could not be queried for its version number.
 *
 * This function is recommended for use in complex applications
 * where extended information is required.  For more simple
 * applications, the use of aa_open() is recommended.
 */
struct AardvarkExt {
    /* Version matrix */
    AardvarkVersion   version;

    /* Features of this device. */
    int               features;
};
#ifndef __cplusplus
typedef struct AardvarkExt AardvarkExt;
#endif


/*
 * Return the device features as a bit-mask of values, or
 * an error code if the handle is not valid.
 */
#define AA_FEATURE_SPI 0x00000001
#define AA_FEATURE_I2C 0x00000002
#define AA_FEATURE_GPIO 0x00000008
#define AA_FEATURE_I2C_MONITOR 0x00000010


/*
 * Enable logging to a file.  The handle must be standard file
 * descriptor.  In C, a file descriptor can be obtained by using
 * the ANSI C function "open" or by using the function "fileno"
 * on a FILE* stream.  A FILE* stream can be obtained using "fopen"
 * or can correspond to the common "stdout" or "stderr" --
 * available when including stdlib.h
 */
#define AA_LOG_STDOUT 1
#define AA_LOG_STDERR 2


/*
 * Configure the device by enabling/disabling I2C, SPI, and
 * GPIO functions.
 */
enum AardvarkConfig {
    AA_CONFIG_GPIO_ONLY = 0x00,
    AA_CONFIG_SPI_GPIO  = 0x01,
    AA_CONFIG_GPIO_I2C  = 0x02,
    AA_CONFIG_SPI_I2C   = 0x03,
    AA_CONFIG_QUERY     = 0x80
};
#ifndef __cplusplus
typedef enum AardvarkConfig AardvarkConfig;
#endif

#define AA_CONFIG_SPI_MASK 0x00000001
#define AA_CONFIG_I2C_MASK 0x00000002



/*
 * Configure the target power pins.
 * This is only supported on hardware versions >= 2.00
 */
#define AA_TARGET_POWER_NONE 0x00
#define AA_TARGET_POWER_BOTH 0x03
#define AA_TARGET_POWER_QUERY 0x80



/*=========================================================================
| ASYNC MESSAGE POLLING
 ========================================================================*/
/*
 * Polling function to check if there are any asynchronous
 * messages pending for processing. The function takes a timeout
 * value in units of milliseconds.  If the timeout is < 0, the
 * function will block until data is received.  If the timeout is 0,
 * the function will perform a non-blocking check.
 */
#define AA_ASYNC_NO_DATA 0x00000000
#define AA_ASYNC_I2C_READ 0x00000001
#define AA_ASYNC_I2C_WRITE 0x00000002
#define AA_ASYNC_SPI 0x00000004
#define AA_ASYNC_I2C_MONITOR 0x00000008

/*=========================================================================
| I2C API
 ========================================================================*/
enum AardvarkI2cFlags {
    AA_I2C_NO_FLAGS          = 0x00,
    AA_I2C_10_BIT_ADDR       = 0x01,
    AA_I2C_COMBINED_FMT      = 0x02,
    AA_I2C_NO_STOP           = 0x04,
    AA_I2C_SIZED_READ        = 0x10,
    AA_I2C_SIZED_READ_EXTRA1 = 0x20
};
#ifndef __cplusplus
typedef enum AardvarkI2cFlags AardvarkI2cFlags;
#endif


enum AardvarkI2cStatus {
    AA_I2C_STATUS_OK            = 0,
    AA_I2C_STATUS_BUS_ERROR     = 1,
    AA_I2C_STATUS_SLA_ACK       = 2,
    AA_I2C_STATUS_SLA_NACK      = 3,
    AA_I2C_STATUS_DATA_NACK     = 4,
    AA_I2C_STATUS_ARB_LOST      = 5,
    AA_I2C_STATUS_BUS_LOCKED    = 6,
    AA_I2C_STATUS_LAST_DATA_ACK = 7
};
#ifndef __cplusplus
typedef enum AardvarkI2cStatus AardvarkI2cStatus;
#endif


/* Read the data collected by the bus monitor */
#define AA_I2C_MONITOR_DATA 0x00ff
#define AA_I2C_MONITOR_NACK 0x0100
#define AA_I2C_MONITOR_CMD_START 0xff00
#define AA_I2C_MONITOR_CMD_STOP 0xff01


/*
 * Configure the I2C pullup resistors.
 * This is only supported on hardware versions >= 2.00
 */
#define AA_I2C_PULLUP_NONE 0x00
#define AA_I2C_PULLUP_BOTH 0x03
#define AA_I2C_PULLUP_QUERY 0x80



/*=========================================================================
| SPI API
 ========================================================================*/
/*
 * These configuration parameters specify how to clock the
 * bits that are sent and received on the Aardvark SPI
 * interface.
 *
 *   The polarity option specifies which transition
 *   constitutes the leading edge and which transition is the
 *   falling edge.  For example, AA_SPI_POL_RISING_FALLING
 *   would configure the SPI to idle the SCK clock line low.
 *   The clock would then transition low-to-high on the
 *   leading edge and high-to-low on the trailing edge.
 *
 *   The phase option determines whether to sample or setup on
 *   the leading edge.  For example, AA_SPI_PHASE_SAMPLE_SETUP
 *   would configure the SPI to sample on the leading edge and
 *   setup on the trailing edge.
 *
 *   The bitorder option is used to indicate whether LSB or
 *   MSB is shifted first.
 *
 * See the diagrams in the Aardvark datasheet for
 * more details.
 */
enum AardvarkSpiPolarity {
    AA_SPI_POL_RISING_FALLING = 0,
    AA_SPI_POL_FALLING_RISING = 1
};
#ifndef __cplusplus
typedef enum AardvarkSpiPolarity AardvarkSpiPolarity;
#endif


enum AardvarkSpiPhase {
    AA_SPI_PHASE_SAMPLE_SETUP = 0,
    AA_SPI_PHASE_SETUP_SAMPLE = 1
};
#ifndef __cplusplus
typedef enum AardvarkSpiPhase AardvarkSpiPhase;
#endif


enum AardvarkSpiBitorder {
    AA_SPI_BITORDER_MSB = 0,
    AA_SPI_BITORDER_LSB = 1
};
#ifndef __cplusplus
typedef enum AardvarkSpiBitorder AardvarkSpiBitorder;
#endif



/*
 * Change the output polarity on the SS line.
 *
 * Note: When configured as an SPI slave, the Aardvark will
 * always be setup with SS as active low.  Hence this function
 * only affects the SPI master functions on the Aardvark.
 */
enum AardvarkSpiSSPolarity {
    AA_SPI_SS_ACTIVE_LOW  = 0,
    AA_SPI_SS_ACTIVE_HIGH = 1
};
#ifndef __cplusplus
typedef enum AardvarkSpiSSPolarity AardvarkSpiSSPolarity;
#endif





/*=========================================================================
| GPIO API
 ========================================================================*/
/*
 * The following enumerated type maps the named lines on the
 * Aardvark I2C/SPI line to bit positions in the GPIO API.
 * All GPIO API functions will index these lines through an
 * 8-bit masked value.  Thus, each bit position in the mask
 * can be referred back its corresponding line through the
 * enumerated type.
 */
enum AardvarkGpioBits {
    AA_GPIO_SCL  = 0x01,
    AA_GPIO_SDA  = 0x02,
    AA_GPIO_MISO = 0x04,
    AA_GPIO_SCK  = 0x08,
    AA_GPIO_MOSI = 0x10,
    AA_GPIO_SS   = 0x20
};
#ifndef __cplusplus
typedef enum AardvarkGpioBits AardvarkGpioBits;
#endif

/*
 * Configure the GPIO, specifying the direction of each bit.
 *
 * A call to this function will not change the value of the pullup
 * mask in the Aardvark.  This is illustrated by the following
 * example:
 *   (1) Direction mask is first set to 0x00
 *   (2) Pullup is set to 0x01
 *   (3) Direction mask is set to 0x01
 *   (4) Direction mask is later set back to 0x00.
 *
 * The pullup will be active after (4).
 *
 * On Aardvark power-up, the default value of the direction
 * mask is 0x00.
 */
#define AA_GPIO_DIR_INPUT 0
#define AA_GPIO_DIR_OUTPUT 1
int aa_gpio_direction (
    Aardvark aardvark,
    u08      direction_mask
);


/*
 * Enable an internal pullup on any of the GPIO input lines.
 *
 * Note: If a line is configured as an output, the pullup bit
 * for that line will be ignored, though that pullup bit will
 * be cached in case the line is later configured as an input.
 *
 * By default the pullup mask is 0x00.
 */
#define AA_GPIO_PULLUP_OFF 0
#define AA_GPIO_PULLUP_ON 1
int aa_gpio_pullup (
    Aardvark aardvark,
    u08      pullup_mask
);



//predfine symbol RemoteAardvark for our typedef
class RemoteAardvark;

//define our type for function pointer to members of RemoteAardvark
typedef bool (RemoteAardvark::*afptr)(Value&, Value&);

#define NUMBER_OF_FUNCTIONS 23

struct _function
{
	const char* _name;
	afptr _funcPtr;
	int paramCount;
	_param* paramArray;
};

//Definition of all known parameters
static _param _num_devices = {"num_devices", kNumberType};
static _param _port = {"port", kNumberType };
static _param _aardvark = {"Aardvark", kNumberType};
static _param _status = {"status", kNumberType};
static _param _powerMask = {"powerMask", kNumberType};
static _param _slave_addr = {"slave_addr", kNumberType};
static _param _flags = {"AardvarkI2cFlags", kNumberType};
static _param _num_bytes = {"num_bytes", kNumberType};
static _param _data_out = {"data_out", kArrayType};
static _param _config = {"AardvarkConfig", kNumberType};
static _param _bitrate = {"bitrate", kNumberType};
static _param _pullup_mask = {"pullup_mask", kNumberType};
static _param _maxTxBytes = {"maxTxBytes", kNumberType};
static _param _maxRxBytes = {"maxRxBytes", kNumberType};
static _param _timeout = {"timeout", kNumberType};
static _param _polarity = {"polarity", kNumberType};
static _param _phase = {"phase", kNumberType};
static _param _bitorder = {"bitorder", kNumberType};



//Define which function uses which parameter
static _param aa_find_devices_params[1] = {_num_devices};
static _param aa_open_params[1] = {_port};
static _param aa_port_params[1] = {_aardvark};
static _param aa_status_string_params[1] = {_status};
static _param aa_target_power_params[2] = {_aardvark, _powerMask};
static _param aa_i2c_write_params[4] = {_aardvark,_slave_addr, _flags, _data_out};
static _param aa_i2c_read_params[4] = {_aardvark, _slave_addr, _flags, _num_bytes};
static _param aa_configure_params[2] = {_aardvark, _config};
static _param aa_i2c_bitrate_params[2] = {_aardvark, _bitrate};
static _param aa_i2c_pullup_params[2] = {_aardvark, _pullup_mask};
static _param aa_i2c_slave_enable_params[4] = {_aardvark, _slave_addr, _maxTxBytes, _maxRxBytes};
static _param aa_i2c_slave_read_params[2] = {_aardvark, _num_bytes};
static _param aa_async_poll_params[2] = {_aardvark, _timeout};
static _param aa_spi_bitrate_params[2] = {_aardvark, _bitrate};
static _param aa_spi_configure_params[4] = {_aardvark, _polarity, _phase, _bitorder};
static _param aa_spi_write_params[3] = {_aardvark, _data_out, _num_bytes};
static _param aa_spi_master_ss_polarity_params[2] = {_aardvark, _polarity};


//Connect functionname and _param struct
static _function _aa_find_devices = {"Aardvark.aa_find_devices", NULL, 1, aa_find_devices_params};
static _function _aa_find_devices_ext = {"Aardvark.aa_find_devices_ext", NULL, 1, aa_find_devices_params};
static _function _aa_open = {"Aardvark.aa_open", NULL, 1, aa_open_params};
static _function _aa_open_ext = {"Aardvark.aa_open_ext", NULL, 1 , aa_open_params};
static _function _aa_close = {"Aardvark.aa_close", NULL, 1, aa_port_params};
static _function _aa_port = {"Aardvark.aa_port", NULL, 1 , aa_port_params};
static _function _aa_features = {"Aardvark.aa_features", NULL, 1, aa_port_params};
static _function _aa_unique_id = {"Aardvark.aa_unique_id", NULL, 1, aa_port_params};
static _function _aa_status_string = {"Aardvark.aa_status_string", NULL, 1, aa_status_string_params};
static _function _aa_target_power = {"Aardvark.aa_target_power", NULL, 2, aa_target_power_params};
static _function _aa_version = {"Aardvark.aa_version", NULL, 1, aa_port_params};
static _function _aa_i2c_write = {"Aardvark.aa_i2c_write", NULL, 4, aa_i2c_write_params};
static _function _aa_i2c_read = {"Aardvark.aa_i2c_read", NULL, 4, aa_i2c_read_params};
static _function _aa_configure = {"Aardvark.aa_configure", NULL, 2, aa_configure_params};
static _function _aa_i2c_bitrate = {"Aardvark.aa_i2c_bitrate", NULL, 2, aa_i2c_bitrate_params};
static _function _aa_i2c_pullup = {"Aardvark.aa_i2c_pullup", NULL, 2, aa_i2c_pullup_params};
static _function _aa_i2c_slave_enable = {"Aardvark.aa_i2c_slave_enable", NULL, 4, aa_i2c_slave_enable_params};
static _function _aa_i2c_slave_read = {"Aardvark.aa_i2c_slave_read", NULL, 2, aa_i2c_slave_read_params};
static _function _aa_async_poll = {"Aardvark.aa_async_poll", NULL, 2, aa_async_poll_params};
static _function _aa_spi_bitrate = {"Aardvark.aa_spi_bitrate", NULL, 2, aa_spi_bitrate_params};
static _function _aa_spi_configure = {"Aardvark.aa_spi_configure", NULL, 4, aa_spi_configure_params};
static _function _aa_spi_write = {"Aardvark.aa_spi_write", NULL, 3, aa_spi_write_params};
static _function _aa_spi_master_ss_polarity = {"Aardvark.aa_spi_master_ss_polarity", NULL, 2, aa_spi_master_ss_polarity_params};



class RemoteAardvark : public RPCInterface<RemoteAardvark*, afptr>{

	public:
		RemoteAardvark(int port) : RPCInterface<RemoteAardvark*, afptr>(this)
		{
			this->port = port;
			uniqueId = 0;
			handle = 0;
			contextNumber = 0;
			json = new JsonRPC();

			//get relativ address of function
			_aa_find_devices._funcPtr = &RemoteAardvark::aa_find_devices;
			//save the relativ address to the map with the corresponding key for rpc
			funcMap.insert(pair<const char*, afptr>(_aa_find_devices._name, _aa_find_devices._funcPtr));

			_aa_find_devices_ext._funcPtr = &RemoteAardvark::aa_find_devices_ext;
			funcMap.insert(pair<const char*, afptr>(_aa_find_devices_ext._name, _aa_find_devices_ext._funcPtr));

			_aa_status_string._funcPtr = &RemoteAardvark::aa_status_string;
			funcMap.insert(pair<const char*, afptr>(_aa_status_string._name, _aa_status_string._funcPtr));

			if(port > -1)
			{
				_aa_open._funcPtr = &RemoteAardvark::aa_open;
				funcMap.insert(pair<const char*, afptr>(_aa_open._name, _aa_open._funcPtr));

				_aa_open_ext._funcPtr = &RemoteAardvark::aa_open_ext;
				funcMap.insert(pair<const char*, afptr>(_aa_open_ext._name, _aa_open_ext._funcPtr));

				_aa_close._funcPtr = &RemoteAardvark::aa_close;
				funcMap.insert(pair<const char* , afptr>(_aa_close._name, _aa_close._funcPtr));

				_aa_port._funcPtr = &RemoteAardvark::aa_port;
				funcMap.insert(pair<const char* , afptr>(_aa_port._name, _aa_port._funcPtr));

				_aa_features._funcPtr = &RemoteAardvark::aa_features;
				funcMap.insert(pair<const char* , afptr>(_aa_features._name, _aa_features._funcPtr));

				_aa_unique_id._funcPtr = &RemoteAardvark::aa_unique_id;
				funcMap.insert(pair<const char* , afptr>(_aa_unique_id._name, _aa_unique_id._funcPtr));

				_aa_version._funcPtr = &RemoteAardvark::aa_version;
				funcMap.insert(pair<const char* , afptr>(_aa_version._name, _aa_version._funcPtr));

				_aa_target_power._funcPtr = &RemoteAardvark::aa_target_power;
				funcMap.insert(pair<const char*, afptr>(_aa_target_power._name, _aa_target_power._funcPtr));

				_aa_i2c_write._funcPtr = &RemoteAardvark::aa_i2c_write;
				funcMap.insert(pair<const char*, afptr>(_aa_i2c_write._name , _aa_i2c_write._funcPtr ));

				_aa_i2c_read._funcPtr = &RemoteAardvark::aa_i2c_read;
				funcMap.insert(pair<const char*, afptr>(_aa_i2c_read._name, _aa_i2c_read._funcPtr));

				_aa_configure._funcPtr = &RemoteAardvark::aa_configure;
				funcMap.insert(pair<const char*, afptr>(_aa_configure._name, _aa_configure._funcPtr));

				_aa_i2c_bitrate._funcPtr = &RemoteAardvark::aa_i2c_bitrate;
				funcMap.insert(pair<const char*, afptr>(_aa_i2c_bitrate._name, _aa_i2c_bitrate._funcPtr));

				_aa_i2c_pullup._funcPtr = &RemoteAardvark::aa_i2c_pullup;
				funcMap.insert(pair<const char*, afptr>(_aa_i2c_pullup._name, _aa_i2c_pullup._funcPtr));

				_aa_i2c_slave_enable._funcPtr = &RemoteAardvark::aa_i2c_slave_enable;
				funcMap.insert(pair<const char*, afptr>(_aa_i2c_slave_enable._name, _aa_i2c_slave_enable._funcPtr));

				_aa_i2c_slave_read._funcPtr = &RemoteAardvark::aa_i2c_slave_read;
				funcMap.insert(pair<const char*, afptr>(_aa_i2c_slave_read._name, _aa_i2c_slave_read._funcPtr));

				_aa_async_poll._funcPtr = &RemoteAardvark::aa_async_poll;
				funcMap.insert(pair<const char*, afptr>(_aa_async_poll._name, _aa_async_poll._funcPtr));

				_aa_spi_bitrate._funcPtr = &RemoteAardvark::aa_spi_bitrate;
				funcMap.insert(pair<const char*, afptr>(_aa_spi_bitrate._name, _aa_spi_bitrate._funcPtr));

				_aa_spi_configure._funcPtr = &RemoteAardvark::aa_spi_configure;
				funcMap.insert(pair<const char*, afptr>(_aa_spi_configure._name, _aa_spi_configure._funcPtr));

				_aa_spi_write._funcPtr = &RemoteAardvark::aa_spi_write;
				funcMap.insert(pair<const char*, afptr>(_aa_spi_write._name, _aa_spi_write._funcPtr));

				_aa_spi_master_ss_polarity._funcPtr = &RemoteAardvark::aa_spi_master_ss_polarity;
				funcMap.insert(pair<const char*, afptr>(_aa_spi_master_ss_polarity._name, _aa_spi_master_ss_polarity._funcPtr));
			}

		};



		~RemoteAardvark()
		{
			delete json;
		};


		//impelemt the driver functions !
		bool aa_find_devices(Value &params, Value &result);
		bool aa_find_devices_ext(Value &params, Value &result);
		bool aa_open(Value &params, Value &result);
		bool aa_open_ext(Value &params, Value &result);
		bool aa_close(Value &params, Value &result);
		bool aa_port(Value &params, Value &result);
		bool aa_features(Value &params, Value &result);
		bool aa_unique_id(Value &params, Value &result);
		bool aa_status_string(Value &params, Value &result);
		bool aa_version(Value &params, Value &result);
		bool aa_target_power(Value &params , Value &result);
		bool aa_i2c_write(Value &params, Value &result);
		bool aa_i2c_read(Value &params, Value &result);
		bool aa_configure(Value &params, Value &result);
		bool aa_i2c_bitrate(Value &params, Value &result);
		bool aa_i2c_pullup(Value &params, Value &result);
		bool aa_i2c_slave_enable(Value &params, Value &result);
		bool aa_i2c_slave_read(Value &params, Value &result);
		bool aa_async_poll(Value &params, Value &result);
		bool aa_spi_bitrate(Value &params, Value &result);
		bool aa_spi_configure(Value &params, Value &result);
		bool aa_spi_write(Value &params, Value &result);
		bool aa_spi_master_ss_polarity(Value &params, Value &result);

		void close();
		int getPort(){return this->port;}
		int getHandle(){return this->handle;}
		int getContextNumber(){return this->contextNumber;}
		void setContextNumber(int contextNumber){this->contextNumber = contextNumber;}


	private:

		Document dom;
		JsonRPC* json;
		int port;
		unsigned int uniqueId;
		int handle;
		int contextNumber;

		void closeLibrary();
};



#ifdef __cplusplus
}
#endif

#endif  /* __aardvark_h__ */
