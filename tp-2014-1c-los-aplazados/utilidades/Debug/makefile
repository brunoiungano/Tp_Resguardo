################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include subdir.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
endif

-include ../makefile.defs

# Add inputs and outputs from these tool invocations to the build variables 

# All Target
all: libutilidades.so

dependents:
	-cd /home/utnso/workspace/tp-2014-1c-los-aplazados/commons/Debug && $(MAKE) all
	-cd /home/utnso/workspace/tp-2014-1c-los-aplazados/utilidades/Release && $(MAKE) all

# Tool invocations
libutilidades.so: $(OBJS) $(USER_OBJS) /home/utnso/workspace/tp-2014-1c-los-aplazados/commons/Debug/libcommons.so /home/utnso/workspace/tp-2014-1c-los-aplazados/utilidades/Release/libutilidades.so
	@echo 'Building target: $@'
	@echo 'Invoking: GCC C Linker'
	gcc -L"/home/utnso/workspace/tp-2014-1c-los-aplazados/utilidades/Release" -L"/home/utnso/workspace/tp-2014-1c-los-aplazados/commons/Debug" -shared -o "libutilidades.so" $(OBJS) $(USER_OBJS) $(LIBS)
	@echo 'Finished building target: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) $(OBJS)$(C_DEPS)$(LIBRARIES) libutilidades.so
	-@echo ' '

.PHONY: all clean dependents
.SECONDARY:
/home/utnso/workspace/tp-2014-1c-los-aplazados/commons/Debug/libcommons.so:
/home/utnso/workspace/tp-2014-1c-los-aplazados/utilidades/Release/libutilidades.so:

-include ../makefile.targets
