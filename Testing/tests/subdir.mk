################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../tests/TestRunner_Plugin-Server.cpp \
../tests/test_AardvarkCareTaker.cpp \
../tests/test_RemoteAardvark.cpp 

OBJS += \
./tests/TestRunner_Plugin-Server.o \
./tests/test_AardvarkCareTaker.o \
./tests/test_RemoteAardvark.o 

CPP_DEPS += \
./tests/TestRunner_Plugin-Server.d \
./tests/test_AardvarkCareTaker.d \
./tests/test_RemoteAardvark.d 


# Each subdirectory must supply rules for building sources it contributes
tests/TestRunner_Plugin-Server.o: ../tests/TestRunner_Plugin-Server.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -DTESTMODE -I"/home/dave2/git/Aardvark-Plugin/include" -I/home/dnoack/cpputest-3.6/include/CppUTest -I/home/dnoack/cpputest-3.6/include/CppUTestExt -I/home/dnoack/libs/rapidjson/include/rapidjson -O0 -g3 -Wall -c -fmessage-length=0 ${CXXFLAGS} -MMD -MP -MF"$(@:%.o=%.d)" -MT"tests/TestRunner_Plugin-Server.d" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

tests/%.o: ../tests/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -DTESTMODE -I"/home/dave2/git/rpcUtils/include" -I"/home/dave2/git/Aardvark-Plugin/include" -I/home/dave2/git/cpputest/include -I/home/dave2/git/rapidjson/include/rapidjson -O0 -g3 -Wall -c -fmessage-length=0 ${CXXFLAGS} -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


