# By default, build for the AVR target. Export sim to build a simulator
target := $(if $(SIM),sim,avr)
build_type = $(if $(NDEBUG),release,$(if $(DEBUG),debug,release))
mute := $(if $(VERBOSE),@set -x;,@)

include make/$(target).mak

CC  ?= gcc
CXX ?= g++
SIZE ?= size

BUILD_DIR       := $(target)_$(build_type)
SRC_DIR         := src

# Pre-processor flags
CPPFLAGS        += $(foreach p, $(INCLUDE_DIRS), -I$(SRC_DIR)/$(p))

# Flags for the compilation of C files
CFLAGS          += -ggdb3 -Wall

# Flags for the compilation of C++ files
CXXFLAGS        += $(CFLAGS) -std=c++17 -Wno-subobject-linkage -fno-exceptions

# Flag for the linker
LDFLAGS         += -ggdb3

# Dependencies creation flags
DEPFLAGS         = -MT $@ -MMD -MP -MF $(BUILD_DIR)/$*.d
POSTCOMPILE      = mv -f $(BUILD_DIR)/$*.Td $(BUILD_DIR)/$*.d && touch $@

DEP_FILES        = $(OBJS:%.o=%.d)

COMPILE.c        = $(CC) $(DEPFLAGS) $(CFLAGS) $(CPPFLAGS) $(TARGET_ARCH) -c
COMPILE.cxx      = $(CXX) $(DEPFLAGS) $(CXXFLAGS) $(CPPFLAGS) $(TARGET_ARCH) -c

all : $(BUILD_DIR)/$(BIN)

# Create the build directory
$(BUILD_DIR): ; @-mkdir -p $@

$(BUILD_DIR)/$(BIN) : $(OBJS)
	@echo "Linking to $@"
	$(mute)$(CXX) -Wl,--start-group $^ -Wl,--end-group ${LDFLAGS} -o $@
	$(mute)$(SIZE) $@

-include ${DEP_FILES}

$(BUILD_DIR)/%.o : $(SRC_DIR)/%.c $(BUILD_DIR)/%.d | $(@D)
	@echo "Compiling $<"
	$(mute)$(COMPILE.c) $< -o $@

${BUILD_DIR}/%.o : $(SRC_DIR)/%.cpp $(BUILD_DIR)/%.d | $(@D)
	@echo "Compiling C++ $<"
	$(mute)$(COMPILE.cxx) $< -o $@

$(BUILD_DIR)/%.o : $(SRC_DIR)/%.s | $(@D)
	@echo "Assembling $<"
	$(mute)$(CXX) -Wa,-gdwarf2 -x assembler-with-cpp $(CPPFLAGS) -c -mmcu=atxmega128a4u -Wa,-g $< -o $@

$(DEP_FILES):
	@[ -d $(@D) ] || mkdir -p $(@D)

include $(wildcard $(DEP_FILES))

.PHONY: clean

clean:
	$(mute)-rm -rf $(BUILD_DIR)
