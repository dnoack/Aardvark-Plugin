################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/AardvarkCareTaker.cpp \
../src/AardvarkPlugin.cpp \
../src/JsonRPC.cpp \
../src/RemoteAardvark.cpp \
../src/UdsComWorker.cpp \
../src/UdsRegClient.cpp \
../src/UdsRegWorker.cpp \
../src/UdsServer.cpp 

OBJS += \
./src/AardvarkCareTaker.o \
./src/AardvarkPlugin.o \
./src/JsonRPC.o \
./src/RemoteAardvark.o \
./src/UdsComWorker.o \
./src/UdsRegClient.o \
./src/UdsRegWorker.o \
./src/UdsServer.o 

CPP_DEPS += \
./src/AardvarkCareTaker.d \
./src/AardvarkPlugin.d \
./src/JsonRPC.d \
./src/RemoteAardvark.d \
./src/UdsComWorker.d \
./src/UdsRegClient.d \
./src/UdsRegWorker.d \
./src/UdsServer.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -DTESTMODE -I/home/dnoack/cpputest-3.6/lib/include/CppUTest -O0 -g3 -Wall -c -fmessage-length=0 -Wno-write-strings -I../include -I/home/dnoack/cpputest-3.6/lib/include/CppUTest -I/home/dnoack/cpputest-3.6/lib/include/CppUTestExt -I../cpputest/include/CppUTestExt -I../include/rapidjson -I../include -I/home/dnoack/cpputest-3.6/lib/include/CppUTest -I/home/dnoack/cpputest-3.6/lib/include/CppUTestExt -I../cpputest/include/CppUTestExt -I../include/rapidjson -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


