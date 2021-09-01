# By default, build for the AVR target. Export sim to build a simulator
target := $(if $(SIM),linux,avr)
build_type := $(if $(DEBUG),debug,release)
mute := $(if $(VERBOSE),@set -x;,@)

include make/$(target).mak

CC  ?= gcc
CXX ?= g++

BUILD_DIR       := $(target)_$(build_type)
SRC_DIR         := src

# Flags for the compilation of C files
CFLAGS          += -ggdb3 -Wall -O$(if $(DEBUG),g,s)

# Flags for the compilation of C++ files
CXXFLAGS        += $(CFLAGS) -std=c++17 -Wno-subobject-linkage -fno-exceptions

# Flag for the linker
LDFLAGS         += -ggdb3 -O$(if $(DEBUG),g,s)

ifdef SIM
LDFLAGS         += -pthread 
endif

# Pre-processor flags
CPPFLAGS        += $(foreach p, $(INCLUDE_DIRS), -I$(SRC_DIR)/$(p))

# Dependencies creation flags
DEPFLAGS         = -MT $@ -MMD -MP -MF $(BUILD_DIR)/$*.d
POSTCOMPILE      = mv -f $(BUILD_DIR)/$*.Td $(BUILD_DIR)/$*.d && touch $@

OBJS.c           = $(foreach p, $(SRCS.c), $(BUILD_DIR)/$(p:%.c=%.o))
OBJS.cxx         = $(foreach p, $(SRCS.cxx), $(BUILD_DIR)/$(p:%.cpp=%.o))
OBJS.as          = $(foreach p, $(SRCS.as), $(BUILD_DIR)/$(p:%.s=%.o))
OBJ_FILES        = $(OBJS.c) $(OBJS.cxx) $(OBJS.as)
DEP_FILES        = $(OBJ_FILES:%.o=%.d)

COMPILE.c        = $(CC) $(DEPFLAGS) $(CFLAGS) $(CPPFLAGS) $(TARGET_ARCH) -c
COMPILE.cxx      = $(CXX) $(DEPFLAGS) $(CXXFLAGS) $(CPPFLAGS) $(TARGET_ARCH) -c

all : $(BUILD_DIR)/$(BIN)

# Create the build directory
$(BUILD_DIR): ; @-mkdir -p $@

$(BUILD_DIR)/$(BIN) : $(OBJ_FILES)
	@echo "Linking $^"
	$(mute)$(CXX) -Wl,--start-group $^ -Wl,--end-group ${LDFLAGS} -o $@

-include ${DEP_FILES}

$(BUILD_DIR)/%.o : $(SRC_DIR)/%.c $(BUILD_DIR)/%.d | $(@D)
	@echo "Compiling $<"
	$(mute)$(COMPILE.c) $< -o $@

${BUILD_DIR}/%.o : $(SRC_DIR)/%.cpp $(BUILD_DIR)/%.d | $(@D)
	@echo "Compiling C++ $<"
	$(mute)$(COMPILE.cxx) $< -o $@

$(BUILD_DIR)/%.o : $(SRC_DIR)/%.s $(BUILD_DIR)/%.d | $(@D)
	@echo "Assembling $<"
	$(mute)$(CXX) -Wa,-gdwarf2 -x assembler-with-cpp $(CPPFLAGS) -c -mmcu=atxmega128a4u -Wa,-g $< -o $@

$(DEP_FILES):
	@[ -d $(@D) ] || mkdir -p $(@D)

include $(wildcard $(DEP_FILES))

.PHONY: clean

clean:
	-rm -rf $(BUILD_DIR)
