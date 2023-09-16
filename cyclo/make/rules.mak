# By default, build for the AVR target. Export sim to build a simulator
target := $(if $(SIM),sim,avr)
build_type = $(if $(NDEBUG),release,$(if $(DEBUG),debug,release))
mute := $(if $(VERBOSE),@set -x;,@)

include make/$(target).mak

CC  ?= gcc
CXX ?= g++
RC ?= make/rc.py
SIZE ?= size

BUILD_DIR       := $(target)_$(build_type)

# Pre-processor flags
CPPFLAGS        += $(foreach p, $(INCLUDE_DIRS), -I$(p))

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

RCDEP_FILES      = $(foreach rc, $(SRCS.resources:%.json=%.rcd), $(BUILD_DIR)/$(rc))

COMPILE.c        = $(CC) $(DEPFLAGS) $(CFLAGS) $(CPPFLAGS) $(TARGET_ARCH) -c
COMPILE.cxx      = $(CXX) $(DEPFLAGS) $(CXXFLAGS) $(CPPFLAGS) $(TARGET_ARCH) -c
COMPILE.rc       = $(RC) -E

all : $(BUILD_DIR)/$(BIN)$(BIN_EXT)

sim :
	$(mute)$(MAKE) --no-print-directory $(MAKEFLAGS) SIM=1 all

-include $(DEP_FILES)
-include $(RCDEP_FILES)

# Create the build directory
$(BUILD_DIR): ; @-mkdir -p $@

$(BUILD_DIR)/$(BIN)$(BIN_EXT) : $(OBJS)
	@echo "Linking to $@"
	$(mute)$(CXX) -Wl,--start-group $^ -Wl,--end-group ${LDFLAGS} -o $@
	$(POST_LINK)
	$(DIAG)

$(BUILD_DIR)/%.o : %.c $(BUILD_DIR)/%.d | $(@D)
	@echo "Compiling $<"
	$(mute)$(COMPILE.c) $< -o $@

${BUILD_DIR}/%.o : %.cpp $(BUILD_DIR)/%.d | $(@D)
	@echo "Compiling C++ $<"
	$(mute)$(COMPILE.cxx) $< -o $@

$(BUILD_DIR)/%.o : %.s | $(@D)
	@echo "Assembling $<"
	$(mute)$(CXX) -Wa,-gdwarf2 -x assembler-with-cpp $(CPPFLAGS) -c -mmcu=atxmega128a4u -Wa,-g $< -o $@

$(BUILD_DIR)/%.rcd : %.json
	@echo "Generating the resources from $<"
	$(mute)[ -d $(@D) ] || mkdir -p $(@D)
	$(mute)$(COMPILE.rc) $@ $<

$(DEP_FILES):
	$(mute)[ -d $(@D) ] || mkdir -p $(@D)

include $(wildcard $(DEP_FILES))
include $(RCDEP_FILES)

.PHONY: clean

clean:
	$(mute)-rm -rf $(BUILD_DIR)
