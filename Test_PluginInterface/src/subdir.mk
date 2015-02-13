################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/Aardvark.cpp \
../src/JsonRPC.cpp \
../src/PluginAardvark.cpp \
../src/UdsServer.cpp \
../src/UdsWorker.cpp 

OBJS += \
./src/Aardvark.o \
./src/JsonRPC.o \
./src/PluginAardvark.o \
./src/UdsServer.o \
./src/UdsWorker.o 

CPP_DEPS += \
./src/Aardvark.d \
./src/JsonRPC.d \
./src/PluginAardvark.d \
./src/UdsServer.d \
./src/UdsWorker.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -I../include -I../cpputest/include/CppUTest -I../cpputest/include/CppUTestExt -I../rapidjson/include/rapidjson -O0 -g3 -Wall -c -fmessage-length=0 -Wno-write-strings -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


