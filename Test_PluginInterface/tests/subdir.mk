################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../tests/TestRunner_Plugin-Server.cpp \
../tests/test_DriverInterface.cpp \
../tests/test_JsonRPC.cpp \
../tests/test_RemoteAardvark.cpp \
../tests/test_UdsComWorker.cpp 

OBJS += \
./tests/TestRunner_Plugin-Server.o \
./tests/test_DriverInterface.o \
./tests/test_JsonRPC.o \
./tests/test_RemoteAardvark.o \
./tests/test_UdsComWorker.o 

CPP_DEPS += \
./tests/TestRunner_Plugin-Server.d \
./tests/test_DriverInterface.d \
./tests/test_JsonRPC.d \
./tests/test_RemoteAardvark.d \
./tests/test_UdsComWorker.d 


# Each subdirectory must supply rules for building sources it contributes
tests/TestRunner_Plugin-Server.o: ../tests/TestRunner_Plugin-Server.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -DTESTMODE -I"/home/Dave/git/Aardvark-Plugin/include" -I/home/dnoack/cpputest-3.6/include/CppUTest -I/home/dnoack/cpputest-3.6/include/CppUTestExt -I/home/dnoack/libs/rapidjson/include/rapidjson -O0 -g3 -Wall -c -fmessage-length=0 ${CXXFLAGS} -MMD -MP -MF"$(@:%.o=%.d)" -MT"tests/TestRunner_Plugin-Server.d" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

tests/%.o: ../tests/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -DTESTMODE -I"/home/Dave/git/Aardvark-Plugin/include" -I"/home/Dave/git/RSD-and-Plugin-lib/include" -I/home/dnoack/libs/rapidjson/include/rapidjson -I/home/dnoack/cpputest-3.6/include/CppUTest -I/home/dnoack/cpputest-3.6/include/CppUTestExt -O0 -g3 -Wall -c -fmessage-length=0 ${CXXFLAGS} -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


