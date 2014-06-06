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
all: Algoritmos

dependents:
	-cd /home/utnso/workspace/utilidades/Debug && $(MAKE) all

# Tool invocations
Algoritmos: $(OBJS) $(USER_OBJS) /home/utnso/workspace/utilidades/Debug/libutilidades.so
	@echo 'Building target: $@'
	@echo 'Invoking: GCC C Linker'
	gcc -L"/home/utnso/workspace/utilidades/Debug" -o "Algoritmos" $(OBJS) $(USER_OBJS) $(LIBS)
	@echo 'Finished building target: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) $(OBJS)$(C_DEPS)$(EXECUTABLES) Algoritmos
	-@echo ' '

.PHONY: all clean dependents
.SECONDARY:
/home/utnso/workspace/utilidades/Debug/libutilidades.so:

-include ../makefile.targets
