CC                    := gcc
CXX                   := g++
BIN                   := cyclo

BUILD_DIR             := ../build
FREERTOS_DIR          := FreeRTOS
LOGGER_DIR            := logger
BOOST_DIR             := boost
KERNEL_DIR            := ${FREERTOS_DIR}/Source
RTOS_DIR              := rtos++
SIMULATION_DIR        := simulation
FX_DIR                := fx

INCLUDE_DIRS          := -I.
INCLUDE_DIRS          += -Iinclude
INCLUDE_DIRS          += -Iconfig
INCLUDE_DIRS          += -I${KERNEL_DIR}/include
INCLUDE_DIRS          += -I${KERNEL_DIR}/portable/ThirdParty/GCC/Posix
INCLUDE_DIRS          += -I${KERNEL_DIR}/portable/ThirdParty/GCC/Posix/utils
INCLUDE_DIRS          += -I${LOGGER_DIR}/include
INCLUDE_DIRS          += -I${BOOST_DIR}
INCLUDE_DIRS          += -I${FX_DIR}/include
INCLUDE_DIRS          += -I${RTOS_DIR}/include
INCLUDE_DIRS          += -I${SIMULATION_DIR}/include

# FreeRTOS
SOURCE_FILES          += $(wildcard ${FREERTOS_DIR}/Source/*.c)

# posix port
SOURCE_FILES          += ${KERNEL_DIR}/portable/ThirdParty/GCC/Posix/utils/wait_for_event.c
SOURCE_FILES          += ${KERNEL_DIR}/portable/ThirdParty/GCC/Posix/port.c

# Logger
SOURCE_FILES          += ${LOGGER_DIR}/src/logger_common.c
CXX_SRCS              := ${LOGGER_DIR}/src/logger_cxx.cpp \
                         ${LOGGER_DIR}/src/logger_init_unix.cpp \
                         ${LOGGER_DIR}/src/logger_os_linux.cpp \
                         ${LOGGER_DIR}/src/logger_trace_stack_linux.cpp

# RTOS
SOURCE_FILES          += ${RTOS_DIR}/src/rtos_hooks.c
CXX_SRCS              += ${RTOS_DIR}/src/rtos.cpp

# Fx
CXX_SRCS              += ${FX_DIR}/src/fx.cpp

# Simulation
CXX_SRCS              += ${SIMULATION_DIR}/src/sim.cpp

# Main
CXX_SRCS              += keypad_tasklet.cpp \
                         nonc_tasklet.cpp \
                         program_manager.cpp \
                         sequencer_worker.cpp \
                         contact.cpp ui_view.cpp \
                         ui_worker.cpp \
						 ui_model.cpp \
                         main.cpp

CFLAGS                :=    -ggdb3 -Wall
LDFLAGS               :=    -ggdb3 -pthread
CPPFLAGS              :=    $(INCLUDE_DIRS) -DBUILD_DIR=\"$(BUILD_DIR_ABS)\"
CPPFLAGS              +=    -D_WINDOWS_ -D_POSIX
CXXFLAGS              :=    -std=c++17 -Wno-subobject-linkage -fno-exceptions $(CFLAGS)

CFLAGS              +=   -O0
LDFLAGS             +=   -O0

ifdef SANITIZE_ADDRESS
  CFLAGS              +=   -fsanitize=address -fsanitize=alignment
  LDFLAGS             +=   -fsanitize=address -fsanitize=alignment
endif

ifdef SANITIZE_LEAK
  LDFLAGS             +=   -fsanitize=leak
endif

OBJ_FILES = $(SOURCE_FILES:%.c=$(BUILD_DIR)/%.o)

OBJ_FILES += $(CXX_SRCS:%.cpp=$(BUILD_DIR)/%.o)

DEP_FILE = $(OBJ_FILES:%.o=%.d)

${BIN} : $(BUILD_DIR)/$(BIN)

${BUILD_DIR}/${BIN} : ${OBJ_FILES}
	@-mkdir -p ${@D}
	@echo "Linking $^"
	@$(CXX) -Wl,--start-group $^ -Wl,--end-group ${LDFLAGS} -o $@

-include ${DEP_FILE}

${BUILD_DIR}/%.o : %.c Makefile
	@-mkdir -p $(@D)
	@echo "Compiling $<"
	@$(CC) $(CPPFLAGS) $(CFLAGS) -MMD -c $< -o $@

${BUILD_DIR}/%.o : %.cpp Makefile
	@-mkdir -p $(@D)
	@echo "Compiling $<"
	@$(CXX) $(CPPFLAGS) $(CXXFLAGS) -MMD -c $< -o $@

.PHONY: clean

clean:
	-rm -rf $(BUILD_DIR)
