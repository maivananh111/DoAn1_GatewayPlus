################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (10.3-2021.10)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../Drivers/RTOSSDK/peripherals/source/dma.cpp \
../Drivers/RTOSSDK/peripherals/source/exti.cpp \
../Drivers/RTOSSDK/peripherals/source/fmc_sdram.cpp \
../Drivers/RTOSSDK/peripherals/source/gpio.cpp \
../Drivers/RTOSSDK/peripherals/source/i2c.cpp \
../Drivers/RTOSSDK/peripherals/source/iwdg.cpp \
../Drivers/RTOSSDK/peripherals/source/rcc.cpp \
../Drivers/RTOSSDK/peripherals/source/spi.cpp \
../Drivers/RTOSSDK/peripherals/source/systick.cpp \
../Drivers/RTOSSDK/peripherals/source/tim.cpp \
../Drivers/RTOSSDK/peripherals/source/usart.cpp 

OBJS += \
./Drivers/RTOSSDK/peripherals/source/dma.o \
./Drivers/RTOSSDK/peripherals/source/exti.o \
./Drivers/RTOSSDK/peripherals/source/fmc_sdram.o \
./Drivers/RTOSSDK/peripherals/source/gpio.o \
./Drivers/RTOSSDK/peripherals/source/i2c.o \
./Drivers/RTOSSDK/peripherals/source/iwdg.o \
./Drivers/RTOSSDK/peripherals/source/rcc.o \
./Drivers/RTOSSDK/peripherals/source/spi.o \
./Drivers/RTOSSDK/peripherals/source/systick.o \
./Drivers/RTOSSDK/peripherals/source/tim.o \
./Drivers/RTOSSDK/peripherals/source/usart.o 

CPP_DEPS += \
./Drivers/RTOSSDK/peripherals/source/dma.d \
./Drivers/RTOSSDK/peripherals/source/exti.d \
./Drivers/RTOSSDK/peripherals/source/fmc_sdram.d \
./Drivers/RTOSSDK/peripherals/source/gpio.d \
./Drivers/RTOSSDK/peripherals/source/i2c.d \
./Drivers/RTOSSDK/peripherals/source/iwdg.d \
./Drivers/RTOSSDK/peripherals/source/rcc.d \
./Drivers/RTOSSDK/peripherals/source/spi.d \
./Drivers/RTOSSDK/peripherals/source/systick.d \
./Drivers/RTOSSDK/peripherals/source/tim.d \
./Drivers/RTOSSDK/peripherals/source/usart.d 


# Each subdirectory must supply rules for building sources it contributes
Drivers/RTOSSDK/peripherals/source/%.o Drivers/RTOSSDK/peripherals/source/%.su Drivers/RTOSSDK/peripherals/source/%.cyclo: ../Drivers/RTOSSDK/peripherals/source/%.cpp Drivers/RTOSSDK/peripherals/source/subdir.mk
	arm-none-eabi-g++ "$<" -mcpu=cortex-m4 -std=gnu++14 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F429xx -c -I../Core/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -I../Middlewares/Third_Party/FreeRTOS/Source/include -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2 -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F -I../Drivers/RTOSSDK/config -I../Drivers/RTOSSDK/peripherals -I../Drivers/RTOSSDK/system -I../Drivers/RTOSSDK/components -I../Drivers/RTOSSDK/startup -I../LWIP/App -I../LWIP/Target -I../Middlewares/Third_Party/LwIP/src/include -I../Middlewares/Third_Party/LwIP/system -I../Drivers/BSP/Components/lan8742 -I../Middlewares/Third_Party/LwIP/src/include/netif/ppp -I../Middlewares/Third_Party/LwIP/src/include/lwip -I../Middlewares/Third_Party/LwIP/src/include/lwip/apps -I../Middlewares/Third_Party/LwIP/src/include/lwip/priv -I../Middlewares/Third_Party/LwIP/src/include/lwip/prot -I../Middlewares/Third_Party/LwIP/src/include/netif -I../Middlewares/Third_Party/LwIP/src/include/compat/posix -I../Middlewares/Third_Party/LwIP/src/include/compat/posix/arpa -I../Middlewares/Third_Party/LwIP/src/include/compat/posix/net -I../Middlewares/Third_Party/LwIP/src/include/compat/posix/sys -I../Middlewares/Third_Party/LwIP/src/include/compat/stdc -I../Middlewares/Third_Party/LwIP/system/arch -O0 -ffunction-sections -fdata-sections -fno-exceptions -fno-rtti -fno-use-cxa-atexit -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Drivers-2f-RTOSSDK-2f-peripherals-2f-source

clean-Drivers-2f-RTOSSDK-2f-peripherals-2f-source:
	-$(RM) ./Drivers/RTOSSDK/peripherals/source/dma.cyclo ./Drivers/RTOSSDK/peripherals/source/dma.d ./Drivers/RTOSSDK/peripherals/source/dma.o ./Drivers/RTOSSDK/peripherals/source/dma.su ./Drivers/RTOSSDK/peripherals/source/exti.cyclo ./Drivers/RTOSSDK/peripherals/source/exti.d ./Drivers/RTOSSDK/peripherals/source/exti.o ./Drivers/RTOSSDK/peripherals/source/exti.su ./Drivers/RTOSSDK/peripherals/source/fmc_sdram.cyclo ./Drivers/RTOSSDK/peripherals/source/fmc_sdram.d ./Drivers/RTOSSDK/peripherals/source/fmc_sdram.o ./Drivers/RTOSSDK/peripherals/source/fmc_sdram.su ./Drivers/RTOSSDK/peripherals/source/gpio.cyclo ./Drivers/RTOSSDK/peripherals/source/gpio.d ./Drivers/RTOSSDK/peripherals/source/gpio.o ./Drivers/RTOSSDK/peripherals/source/gpio.su ./Drivers/RTOSSDK/peripherals/source/i2c.cyclo ./Drivers/RTOSSDK/peripherals/source/i2c.d ./Drivers/RTOSSDK/peripherals/source/i2c.o ./Drivers/RTOSSDK/peripherals/source/i2c.su ./Drivers/RTOSSDK/peripherals/source/iwdg.cyclo ./Drivers/RTOSSDK/peripherals/source/iwdg.d ./Drivers/RTOSSDK/peripherals/source/iwdg.o ./Drivers/RTOSSDK/peripherals/source/iwdg.su ./Drivers/RTOSSDK/peripherals/source/rcc.cyclo ./Drivers/RTOSSDK/peripherals/source/rcc.d ./Drivers/RTOSSDK/peripherals/source/rcc.o ./Drivers/RTOSSDK/peripherals/source/rcc.su ./Drivers/RTOSSDK/peripherals/source/spi.cyclo ./Drivers/RTOSSDK/peripherals/source/spi.d ./Drivers/RTOSSDK/peripherals/source/spi.o ./Drivers/RTOSSDK/peripherals/source/spi.su ./Drivers/RTOSSDK/peripherals/source/systick.cyclo ./Drivers/RTOSSDK/peripherals/source/systick.d ./Drivers/RTOSSDK/peripherals/source/systick.o ./Drivers/RTOSSDK/peripherals/source/systick.su ./Drivers/RTOSSDK/peripherals/source/tim.cyclo ./Drivers/RTOSSDK/peripherals/source/tim.d ./Drivers/RTOSSDK/peripherals/source/tim.o ./Drivers/RTOSSDK/peripherals/source/tim.su ./Drivers/RTOSSDK/peripherals/source/usart.cyclo ./Drivers/RTOSSDK/peripherals/source/usart.d ./Drivers/RTOSSDK/peripherals/source/usart.o ./Drivers/RTOSSDK/peripherals/source/usart.su

.PHONY: clean-Drivers-2f-RTOSSDK-2f-peripherals-2f-source

