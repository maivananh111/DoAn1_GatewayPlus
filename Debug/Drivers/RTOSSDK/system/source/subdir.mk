################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (10.3-2021.10)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../Drivers/RTOSSDK/system/source/stm_log.cpp \
../Drivers/RTOSSDK/system/source/stm_ret_err.cpp \
../Drivers/RTOSSDK/system/source/system.cpp 

OBJS += \
./Drivers/RTOSSDK/system/source/stm_log.o \
./Drivers/RTOSSDK/system/source/stm_ret_err.o \
./Drivers/RTOSSDK/system/source/system.o 

CPP_DEPS += \
./Drivers/RTOSSDK/system/source/stm_log.d \
./Drivers/RTOSSDK/system/source/stm_ret_err.d \
./Drivers/RTOSSDK/system/source/system.d 


# Each subdirectory must supply rules for building sources it contributes
Drivers/RTOSSDK/system/source/%.o Drivers/RTOSSDK/system/source/%.su Drivers/RTOSSDK/system/source/%.cyclo: ../Drivers/RTOSSDK/system/source/%.cpp Drivers/RTOSSDK/system/source/subdir.mk
	arm-none-eabi-g++ "$<" -mcpu=cortex-m4 -std=gnu++14 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F429xx -c -I../Core/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -I../Middlewares/Third_Party/FreeRTOS/Source/include -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2 -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F -I../Drivers/RTOSSDK/config -I../Drivers/RTOSSDK/peripherals -I../Drivers/RTOSSDK/system -I../Drivers/RTOSSDK/components -I../Drivers/RTOSSDK/startup -I../LWIP/App -I../LWIP/Target -I../Middlewares/Third_Party/LwIP/src/include -I../Middlewares/Third_Party/LwIP/system -I../Drivers/BSP/Components/lan8742 -I../Middlewares/Third_Party/LwIP/src/include/netif/ppp -I../Middlewares/Third_Party/LwIP/src/include/lwip -I../Middlewares/Third_Party/LwIP/src/include/lwip/apps -I../Middlewares/Third_Party/LwIP/src/include/lwip/priv -I../Middlewares/Third_Party/LwIP/src/include/lwip/prot -I../Middlewares/Third_Party/LwIP/src/include/netif -I../Middlewares/Third_Party/LwIP/src/include/compat/posix -I../Middlewares/Third_Party/LwIP/src/include/compat/posix/arpa -I../Middlewares/Third_Party/LwIP/src/include/compat/posix/net -I../Middlewares/Third_Party/LwIP/src/include/compat/posix/sys -I../Middlewares/Third_Party/LwIP/src/include/compat/stdc -I../Middlewares/Third_Party/LwIP/system/arch -O0 -ffunction-sections -fdata-sections -fno-exceptions -fno-rtti -fno-use-cxa-atexit -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Drivers-2f-RTOSSDK-2f-system-2f-source

clean-Drivers-2f-RTOSSDK-2f-system-2f-source:
	-$(RM) ./Drivers/RTOSSDK/system/source/stm_log.cyclo ./Drivers/RTOSSDK/system/source/stm_log.d ./Drivers/RTOSSDK/system/source/stm_log.o ./Drivers/RTOSSDK/system/source/stm_log.su ./Drivers/RTOSSDK/system/source/stm_ret_err.cyclo ./Drivers/RTOSSDK/system/source/stm_ret_err.d ./Drivers/RTOSSDK/system/source/stm_ret_err.o ./Drivers/RTOSSDK/system/source/stm_ret_err.su ./Drivers/RTOSSDK/system/source/system.cyclo ./Drivers/RTOSSDK/system/source/system.d ./Drivers/RTOSSDK/system/source/system.o ./Drivers/RTOSSDK/system/source/system.su

.PHONY: clean-Drivers-2f-RTOSSDK-2f-system-2f-source

