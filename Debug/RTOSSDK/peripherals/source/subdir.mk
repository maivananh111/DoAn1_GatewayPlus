################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (10.3-2021.10)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../RTOSSDK/peripherals/source/adc.cpp \
../RTOSSDK/peripherals/source/dma.cpp \
../RTOSSDK/peripherals/source/exti.cpp \
../RTOSSDK/peripherals/source/fmc_sdram.cpp \
../RTOSSDK/peripherals/source/gpio.cpp \
../RTOSSDK/peripherals/source/i2c.cpp \
../RTOSSDK/peripherals/source/iwdg.cpp \
../RTOSSDK/peripherals/source/rcc.cpp \
../RTOSSDK/peripherals/source/rng.cpp \
../RTOSSDK/peripherals/source/spi.cpp \
../RTOSSDK/peripherals/source/systick.cpp \
../RTOSSDK/peripherals/source/tim.cpp \
../RTOSSDK/peripherals/source/usart.cpp 

OBJS += \
./RTOSSDK/peripherals/source/adc.o \
./RTOSSDK/peripherals/source/dma.o \
./RTOSSDK/peripherals/source/exti.o \
./RTOSSDK/peripherals/source/fmc_sdram.o \
./RTOSSDK/peripherals/source/gpio.o \
./RTOSSDK/peripherals/source/i2c.o \
./RTOSSDK/peripherals/source/iwdg.o \
./RTOSSDK/peripherals/source/rcc.o \
./RTOSSDK/peripherals/source/rng.o \
./RTOSSDK/peripherals/source/spi.o \
./RTOSSDK/peripherals/source/systick.o \
./RTOSSDK/peripherals/source/tim.o \
./RTOSSDK/peripherals/source/usart.o 

CPP_DEPS += \
./RTOSSDK/peripherals/source/adc.d \
./RTOSSDK/peripherals/source/dma.d \
./RTOSSDK/peripherals/source/exti.d \
./RTOSSDK/peripherals/source/fmc_sdram.d \
./RTOSSDK/peripherals/source/gpio.d \
./RTOSSDK/peripherals/source/i2c.d \
./RTOSSDK/peripherals/source/iwdg.d \
./RTOSSDK/peripherals/source/rcc.d \
./RTOSSDK/peripherals/source/rng.d \
./RTOSSDK/peripherals/source/spi.d \
./RTOSSDK/peripherals/source/systick.d \
./RTOSSDK/peripherals/source/tim.d \
./RTOSSDK/peripherals/source/usart.d 


# Each subdirectory must supply rules for building sources it contributes
RTOSSDK/peripherals/source/%.o RTOSSDK/peripherals/source/%.su RTOSSDK/peripherals/source/%.cyclo: ../RTOSSDK/peripherals/source/%.cpp RTOSSDK/peripherals/source/subdir.mk
	arm-none-eabi-g++ "$<" -mcpu=cortex-m4 -std=gnu++14 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F429xx -DSTM32F4_Device -D_GNU_SOURCE -DSTM32F4 -c -I../Core/Inc -I../Middlewares/Third_Party/mbedTLS/include/psa -I../RTOSSDK/config -I../RTOSSDK/peripherals -I../RTOSSDK/system -I../RTOSSDK/components -I../RTOSSDK/network -I../RTOSSDK/network/mbedtls/include -I../RTOSSDK/network/mbedtls/configs -I../RTOSSDK/protocols -I../RTOSSDK/startup -I../LWIP/App -I../LWIP/Target -I../FATFS/Target -I../FATFS/App -I../Middlewares/Third_Party/LwIP/src/include -I../Middlewares/Third_Party/LwIP/system -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Middlewares/Third_Party/FreeRTOS/Source/include -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2 -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F -I../Middlewares/Third_Party/FatFs/src -I../Drivers/BSP/Components/lan8742 -I../Middlewares/Third_Party/LwIP/src/include/netif/ppp -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Middlewares/Third_Party/LwIP/src/include/lwip -I../Middlewares/Third_Party/LwIP/src/include/lwip/apps -I../Middlewares/Third_Party/LwIP/src/include/lwip/priv -I../Middlewares/Third_Party/LwIP/src/include/lwip/prot -I../Middlewares/Third_Party/LwIP/src/include/netif -I../Middlewares/Third_Party/LwIP/src/include/compat/posix -I../Middlewares/Third_Party/LwIP/src/include/compat/posix/arpa -I../Middlewares/Third_Party/LwIP/src/include/compat/posix/net -I../Middlewares/Third_Party/LwIP/src/include/compat/posix/sys -I../Middlewares/Third_Party/LwIP/src/include/compat/stdc -I../Middlewares/Third_Party/LwIP/system/arch -I../Drivers/CMSIS/Include -I../Middlewares/Third_Party/mbedTLS/library -O0 -ffunction-sections -fdata-sections -fno-exceptions -fno-rtti -fno-use-cxa-atexit -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-RTOSSDK-2f-peripherals-2f-source

clean-RTOSSDK-2f-peripherals-2f-source:
	-$(RM) ./RTOSSDK/peripherals/source/adc.cyclo ./RTOSSDK/peripherals/source/adc.d ./RTOSSDK/peripherals/source/adc.o ./RTOSSDK/peripherals/source/adc.su ./RTOSSDK/peripherals/source/dma.cyclo ./RTOSSDK/peripherals/source/dma.d ./RTOSSDK/peripherals/source/dma.o ./RTOSSDK/peripherals/source/dma.su ./RTOSSDK/peripherals/source/exti.cyclo ./RTOSSDK/peripherals/source/exti.d ./RTOSSDK/peripherals/source/exti.o ./RTOSSDK/peripherals/source/exti.su ./RTOSSDK/peripherals/source/fmc_sdram.cyclo ./RTOSSDK/peripherals/source/fmc_sdram.d ./RTOSSDK/peripherals/source/fmc_sdram.o ./RTOSSDK/peripherals/source/fmc_sdram.su ./RTOSSDK/peripherals/source/gpio.cyclo ./RTOSSDK/peripherals/source/gpio.d ./RTOSSDK/peripherals/source/gpio.o ./RTOSSDK/peripherals/source/gpio.su ./RTOSSDK/peripherals/source/i2c.cyclo ./RTOSSDK/peripherals/source/i2c.d ./RTOSSDK/peripherals/source/i2c.o ./RTOSSDK/peripherals/source/i2c.su ./RTOSSDK/peripherals/source/iwdg.cyclo ./RTOSSDK/peripherals/source/iwdg.d ./RTOSSDK/peripherals/source/iwdg.o ./RTOSSDK/peripherals/source/iwdg.su ./RTOSSDK/peripherals/source/rcc.cyclo ./RTOSSDK/peripherals/source/rcc.d ./RTOSSDK/peripherals/source/rcc.o ./RTOSSDK/peripherals/source/rcc.su ./RTOSSDK/peripherals/source/rng.cyclo ./RTOSSDK/peripherals/source/rng.d ./RTOSSDK/peripherals/source/rng.o ./RTOSSDK/peripherals/source/rng.su ./RTOSSDK/peripherals/source/spi.cyclo ./RTOSSDK/peripherals/source/spi.d ./RTOSSDK/peripherals/source/spi.o ./RTOSSDK/peripherals/source/spi.su ./RTOSSDK/peripherals/source/systick.cyclo ./RTOSSDK/peripherals/source/systick.d ./RTOSSDK/peripherals/source/systick.o ./RTOSSDK/peripherals/source/systick.su ./RTOSSDK/peripherals/source/tim.cyclo ./RTOSSDK/peripherals/source/tim.d ./RTOSSDK/peripherals/source/tim.o ./RTOSSDK/peripherals/source/tim.su ./RTOSSDK/peripherals/source/usart.cyclo ./RTOSSDK/peripherals/source/usart.d ./RTOSSDK/peripherals/source/usart.o ./RTOSSDK/peripherals/source/usart.su

.PHONY: clean-RTOSSDK-2f-peripherals-2f-source

