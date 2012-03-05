################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/server/kaffeine.c \
../src/server/vcp.c 

OBJS += \
./src/server/kaffeine.o \
./src/server/vcp.o 

C_DEPS += \
./src/server/kaffeine.d \
./src/server/vcp.d 


# Each subdirectory must supply rules for building sources it contributes
src/server/%.o: ../src/server/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -std=c99 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


