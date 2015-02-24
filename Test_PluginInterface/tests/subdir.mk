################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../tests/TestRunner_Plugin-Server.cpp \
../tests/test_JsonRPC.cpp \
../tests/test_PluginAardvark.cpp \
../tests/test_PluginInterface.cpp \
../tests/test_RemoteAardvark.cpp \
../tests/test_UdsWorker.cpp 

OBJS += \
./tests/TestRunner_Plugin-Server.o \
./tests/test_JsonRPC.o \
./tests/test_PluginAardvark.o \
./tests/test_PluginInterface.o \
./tests/test_RemoteAardvark.o \
./tests/test_UdsWorker.o 

CPP_DEPS += \
./tests/TestRunner_Plugin-Server.d \
./tests/test_JsonRPC.d \
./tests/test_PluginAardvark.d \
./tests/test_PluginInterface.d \
./tests/test_RemoteAardvark.d \
./tests/test_UdsWorker.d 


# Each subdirectory must supply rules for building sources it contributes
tests/%.o: ../tests/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -DTESTMODE -O0 -g3 -Wall -c -fmessage-length=0 -Wno-write-strings -I../include -I../cpputest/include/CppUTest -I../cpputest/include/CppUTestExt -I../include/rapidjson -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


