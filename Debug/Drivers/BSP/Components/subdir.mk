################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (10.3-2021.10)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
/home/anh/STM32Cube/Repository/STM32Cube_FW_F4_V1.27.1/Drivers/BSP/Components/lan8742/lan8742.c 

C_DEPS += \
./Drivers/BSP/Components/lan8742.d 

OBJS += \
./Drivers/BSP/Components/lan8742.o 


# Each subdirectory must supply rules for building sources it contributes
Drivers/BSP/Components/lan8742.o: /home/anh/STM32Cube/Repository/STM32Cube_FW_F4_V1.27.1/Drivers/BSP/Components/lan8742/lan8742.c Drivers/BSP/Components/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F429xx -D_GNU_SOURCE -DSTM32F4_Device '-DMBEDTLS_CONFIG_FILE="mbedtls_config.h"' -c -I../Core/Inc -I../RTOSSDK/config -I../RTOSSDK/peripherals -I../RTOSSDK/system -I../RTOSSDK/components -I../RTOSSDK/network -I../RTOSSDK/protocols -I../RTOSSDK/startup -I../LWIP/App -I../LWIP/Target -I../FATFS/Target -I../FATFS/App -I../MBEDTLS/App -I/home/anh/STM32Cube/Repository/STM32Cube_FW_F4_V1.27.1/Middlewares/Third_Party/LwIP/src/include -I/home/anh/STM32Cube/Repository/STM32Cube_FW_F4_V1.27.1/Middlewares/Third_Party/LwIP/system -I/home/anh/STM32Cube/Repository/STM32Cube_FW_F4_V1.27.1/Drivers/STM32F4xx_HAL_Driver/Inc -I/home/anh/STM32Cube/Repository/STM32Cube_FW_F4_V1.27.1/Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I/home/anh/STM32Cube/Repository/STM32Cube_FW_F4_V1.27.1/Middlewares/Third_Party/FreeRTOS/Source/include -I/home/anh/STM32Cube/Repository/STM32Cube_FW_F4_V1.27.1/Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2 -I/home/anh/STM32Cube/Repository/STM32Cube_FW_F4_V1.27.1/Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F -I/home/anh/STM32Cube/Repository/STM32Cube_FW_F4_V1.27.1/Middlewares/Third_Party/FatFs/src -I/home/anh/STM32Cube/Repository/STM32Cube_FW_F4_V1.27.1/Drivers/BSP/Components/lan8742 -I/home/anh/STM32Cube/Repository/STM32Cube_FW_F4_V1.27.1/Middlewares/Third_Party/LwIP/src/include/netif/ppp -I/home/anh/STM32Cube/Repository/STM32Cube_FW_F4_V1.27.1/Middlewares/Third_Party/mbedTLS/include/mbedtls -I/home/anh/STM32Cube/Repository/STM32Cube_FW_F4_V1.27.1/Drivers/CMSIS/Device/ST/STM32F4xx/Include -I/home/anh/STM32Cube/Repository/STM32Cube_FW_F4_V1.27.1/Middlewares/Third_Party/LwIP/src/include/lwip -I/home/anh/STM32Cube/Repository/STM32Cube_FW_F4_V1.27.1/Middlewares/Third_Party/LwIP/src/include/lwip/apps -I/home/anh/STM32Cube/Repository/STM32Cube_FW_F4_V1.27.1/Middlewares/Third_Party/LwIP/src/include/lwip/priv -I/home/anh/STM32Cube/Repository/STM32Cube_FW_F4_V1.27.1/Middlewares/Third_Party/LwIP/src/include/lwip/prot -I/home/anh/STM32Cube/Repository/STM32Cube_FW_F4_V1.27.1/Middlewares/Third_Party/LwIP/src/include/netif -I/home/anh/STM32Cube/Repository/STM32Cube_FW_F4_V1.27.1/Middlewares/Third_Party/LwIP/src/include/compat/posix -I/home/anh/STM32Cube/Repository/STM32Cube_FW_F4_V1.27.1/Middlewares/Third_Party/LwIP/src/include/compat/posix/arpa -I/home/anh/STM32Cube/Repository/STM32Cube_FW_F4_V1.27.1/Middlewares/Third_Party/LwIP/src/include/compat/posix/net -I/home/anh/STM32Cube/Repository/STM32Cube_FW_F4_V1.27.1/Middlewares/Third_Party/LwIP/src/include/compat/posix/sys -I/home/anh/STM32Cube/Repository/STM32Cube_FW_F4_V1.27.1/Middlewares/Third_Party/LwIP/src/include/compat/stdc -I/home/anh/STM32Cube/Repository/STM32Cube_FW_F4_V1.27.1/Middlewares/Third_Party/LwIP/system/arch -I/home/anh/STM32Cube/Repository/STM32Cube_FW_F4_V1.27.1/Middlewares/Third_Party/mbedTLS/include -I/home/anh/STM32Cube/Repository/STM32Cube_FW_F4_V1.27.1/Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Drivers-2f-BSP-2f-Components

clean-Drivers-2f-BSP-2f-Components:
	-$(RM) ./Drivers/BSP/Components/lan8742.cyclo ./Drivers/BSP/Components/lan8742.d ./Drivers/BSP/Components/lan8742.o ./Drivers/BSP/Components/lan8742.su

.PHONY: clean-Drivers-2f-BSP-2f-Components

