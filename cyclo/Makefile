# Name of the binary to produce
BIN                   := cyclo

# Paths, local to src
FREERTOS_DIR          := FreeRTOS
LOGGER_DIR            := logger
BOOST_DIR             := boost
KERNEL_DIR            := ${FREERTOS_DIR}/Source
RTOS_DIR              := rtos++
SIMULATION_DIR        := simulation
FX_DIR                := fx

# -I throughout (C and C++)
INCLUDE_DIRS = \
  . include config \
  ${KERNEL_DIR}/include \
  ${LOGGER_DIR}/include \
  ${BOOST_DIR} \
  ${FX_DIR}/include \
  ${RTOS_DIR}/include \

ifdef SIM
INCLUDE_DIRS += \
  ${KERNEL_DIR}/portable/ThirdParty/GCC/Posix \
  ${KERNEL_DIR}/portable/ThirdParty/GCC/Posix/utils \
  ${SIMULATION_DIR}/include \

endif

# C source files
SRCS.c := \
   ${RTOS_DIR}/src/rtos_hooks.c \
   $(wildcard ${FREERTOS_DIR}/Source/*.c) \
   ${LOGGER_DIR}/src/logger_common.c

ifdef SIM
SRCS.c += \
   ${KERNEL_DIR}/portable/ThirdParty/GCC/Posix/utils/wait_for_event.c \
   ${KERNEL_DIR}/portable/ThirdParty/GCC/Posix/port.c \

endif

# C++ source files
SRCS.cxx := \
  ${LOGGER_DIR}/src/logger_cxx.cpp \
  ${LOGGER_DIR}/src/logger_init_unix.cpp \
  ${LOGGER_DIR}/src/logger_os_linux.cpp \
  ${LOGGER_DIR}/src/logger_trace_stack_linux.cpp \
  ${RTOS_DIR}/src/rtos.cpp \
  ${FX_DIR}/src/fx.cpp \
  keypad_tasklet.cpp \
  nonc_tasklet.cpp \
  program_manager.cpp \
  sequencer_worker.cpp \
  contact.cpp ui_view.cpp \
  ui_worker.cpp \
  ui_model.cpp \
  main.cpp

ifdef SIM
SRCS.cxx += \
  ${SIMULATION_DIR}/src/sim.cpp \

endif

include make/rules.mak
