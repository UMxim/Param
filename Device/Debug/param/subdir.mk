################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (11.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../param/param.c \
../param/param_hal_stm32L011D4P6.c 

OBJS += \
./param/param.o \
./param/param_hal_stm32L011D4P6.o 

C_DEPS += \
./param/param.d \
./param/param_hal_stm32L011D4P6.d 


# Each subdirectory must supply rules for building sources it contributes
param/%.o param/%.su param/%.cyclo: ../param/%.c param/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m0plus -std=gnu11 -g3 -DDEBUG -DUSE_FULL_LL_DRIVER -DHSE_VALUE=8000000 -DHSE_STARTUP_TIMEOUT=100 -DLSE_STARTUP_TIMEOUT=5000 -DLSE_VALUE=32768 -DMSI_VALUE=2097000 -DHSI_VALUE=16000000 -DLSI_VALUE=37000 -DVDD_VALUE=3300 -DPREFETCH_ENABLE=0 -DINSTRUCTION_CACHE_ENABLE=1 -DDATA_CACHE_ENABLE=1 -DSTM32L011xx -c -I../Core/Inc -I../Drivers/STM32L0xx_HAL_Driver/Inc -I../Drivers/CMSIS/Device/ST/STM32L0xx/Include -I../Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"

clean: clean-param

clean-param:
	-$(RM) ./param/param.cyclo ./param/param.d ./param/param.o ./param/param.su ./param/param_hal_stm32L011D4P6.cyclo ./param/param_hal_stm32L011D4P6.d ./param/param_hal_stm32L011D4P6.o ./param/param_hal_stm32L011D4P6.su

.PHONY: clean-param

