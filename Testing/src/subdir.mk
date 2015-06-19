################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/AardvarkCareTaker.cpp \
../src/AardvarkPlugin.cpp \
../src/RemoteAardvark.cpp 

OBJS += \
./src/AardvarkCareTaker.o \
./src/AardvarkPlugin.o \
./src/RemoteAardvark.o 

CPP_DEPS += \
./src/AardvarkCareTaker.d \
./src/AardvarkPlugin.d \
./src/RemoteAardvark.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -DTESTMODE -I"/home/dave2/git/rpcUtils/include" -I"/home/dave2/git/Aardvark-Plugin/include" -I/home/dave2/git/cpputest/include -I/home/dave2/git/rapidjson/include/rapidjson -O0 -g3 -Wall -c -fmessage-length=0 ${CXXFLAGS} -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


