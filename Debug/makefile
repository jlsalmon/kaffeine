################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include subdir.mk
<<<<<<< HEAD
-include src/server/subdir.mk
=======
-include server/subdir.mk
>>>>>>> 593b856f515fbe2873da79ef8c55963a59103bb2
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
endif

-include ../makefile.defs

# Add inputs and outputs from these tool invocations to the build variables 

# All Target
<<<<<<< HEAD
all: kaffeine.new

# Tool invocations
kaffeine.new: $(OBJS) $(USER_OBJS)
	@echo 'Building target: $@'
	@echo 'Invoking: GCC C Linker'
	gcc  -o"kaffeine.new" $(OBJS) $(USER_OBJS) $(LIBS)
=======
all: kaffeine

# Tool invocations
kaffeine: $(OBJS) $(USER_OBJS)
	@echo 'Building target: $@'
	@echo 'Invoking: GCC C Linker'
	gcc  -o"kaffeine" $(OBJS) $(USER_OBJS) $(LIBS)
>>>>>>> 593b856f515fbe2873da79ef8c55963a59103bb2
	@echo 'Finished building target: $@'
	@echo ' '

# Other Targets
clean:
<<<<<<< HEAD
	-$(RM) $(OBJS)$(C_DEPS)$(EXECUTABLES) kaffeine.new
=======
	-$(RM) $(OBJS)$(C_DEPS)$(EXECUTABLES) kaffeine
>>>>>>> 593b856f515fbe2873da79ef8c55963a59103bb2
	-@echo ' '

.PHONY: all clean dependents
.SECONDARY:

-include ../makefile.targets
