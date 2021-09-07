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
ASF_DIR               := ASF
CXX_DIR               := c++

# -D for all
CPPFLAGS = \
   -DBOARD=USER_BOARD \
   -DCPP_FREERTOS_NO_EXCEPTIONS \
   -DGFX_MONO_UG_2832HSWEG04=1 \

# -I throughout (C and C++)
INCLUDE_DIRS = \
   . \
   include config \
   ${KERNEL_DIR}/include \
   ${LOGGER_DIR}/include \
   ${BOOST_DIR} \
   ${FX_DIR}/include \
   ${RTOS_DIR}/include \

ifdef SIM
INCLUDE_DIRS += \
  $(KERNEL_DIR)/portable/ThirdParty/GCC/Posix \
  $(KERNEL_DIR)/portable/ThirdParty/GCC/Posix/utils \
  ${SIMULATION_DIR}/include \
  $(ASF_DIR)/common/services/gfx_mono \

else
INCLUDE_DIRS += \
   ${KERNEL_DIR}/portable/ThirdParty/GCC/ATxmega \
   $(ASF_DIR)/common/boards/user_board \
   $(ASF_DIR)/common/boards \
   $(ASF_DIR)/xmega/utils/preprocessor \
   $(ASF_DIR)/xmega/utils \
   $(ASF_DIR)/common/utils \
   $(ASF_DIR)/xmega/drivers/cpu \
   $(ASF_DIR)/xmega/drivers/nvm \
   $(ASF_DIR)/xmega/drivers/pmic  \
   $(ASF_DIR)/xmega/drivers/sleep \
   $(ASF_DIR)/xmega/drivers/tc \
   $(ASF_DIR)/common/services/clock \
   $(ASF_DIR)/common/services/sleepmgr \
   $(ASF_DIR)/common/services/ioport \
   $(ASF_DIR)/common/services/twi \
   $(ASF_DIR)/xmega/drivers/twi \
   $(ASF_DIR)/common/components/display/ssd1306 \
   $(ASF_DIR)/common/services/gfx_mono \
   $(ASF_DIR)/xmega/drivers/spi \
   $(ASF_DIR)/common/services/delay \
   $(ASF_DIR)/common/services/spi/xmega_spi \
   $(ASF_DIR)/common/services/spi \
   $(ASF_DIR)/xmega/drivers/crc \
   $(ASF_DIR)/xmega/drivers/wdt \
   $(ASF_DIR)/common/services/cpu \
   $(ASF_DIR)/common/services/usb \
   $(ASF_DIR)/common/services/usb/class/cdc \
   $(ASF_DIR)/common/services/usb/class/cdc/device \
   $(ASF_DIR)/common/services/usb/udc \
   $(ASF_DIR)/common/utils/stdio/stdio_usb \
   $(ASF_DIR)/thirdparty/wireless/addons/sio2host/usb \
   $(ASF_DIR)/xmega/drivers/usb \
   $(CXX_DIR)/include \

endif

# Command source files (AVR and simulation)
SRCS.common := \
   ${RTOS_DIR}/src/rtos_hooks.c \
   $(KERNEL_DIR)/list.c \
   $(KERNEL_DIR)/queue.c \
   $(KERNEL_DIR)/stream_buffer.c \
   $(KERNEL_DIR)/tasks.c \
   $(KERNEL_DIR)/timers.c \
   ${RTOS_DIR}/src/rtos.cpp \
   ${FX_DIR}/src/fx.cpp \
   console.cpp \
   contact.cpp \
   keypad_tasklet.cpp \
   main.cpp \
   nonc_tasklet.cpp \
   parser.cpp \
   program_manager.cpp \
   sequencer_worker.cpp \
   ui_model.cpp \
   ui_view.cpp \
   ui_worker.cpp

SRCS.sim += \
   ${KERNEL_DIR}/portable/ThirdParty/GCC/Posix/utils/wait_for_event.c \
   ${KERNEL_DIR}/portable/ThirdParty/GCC/Posix/port.c \
   ${LOGGER_DIR}/src/logger_common.c \
   ${LOGGER_DIR}/src/logger_cxx.cpp \
   ${LOGGER_DIR}/src/logger_init_unix.cpp \
   ${LOGGER_DIR}/src/logger_os_linux.cpp \
   ${LOGGER_DIR}/src/logger_trace_stack_linux.cpp \
   ${SIMULATION_DIR}/src/sim.cpp \
   ${SIMULATION_DIR}/src/nvm.cpp \
   ${SIMULATION_DIR}/src/ssd1306.c \
   ${SIMULATION_DIR}/src/logger.c \
   $(ASF_DIR)/common/services/gfx_mono/gfx_mono_framebuffer.c  \
   $(ASF_DIR)/common/services/gfx_mono/gfx_mono_generic.c \
   $(ASF_DIR)/common/services/gfx_mono/gfx_mono_text.c  \
   $(ASF_DIR)/common/services/gfx_mono/gfx_mono_ug_2832hsweg04.c  \
   $(ASF_DIR)/common/services/gfx_mono/sysfont.c \

# ASF sources
SRCS.avr += \
   ${KERNEL_DIR}/portable/ThirdParty/GCC/ATxmega/port.c \
   $(ASF_DIR)/common/services/usb/class/cdc/device/udi_cdc.c  \
   $(ASF_DIR)/common/services/usb/class/cdc/device/udi_cdc_desc.c \
   $(ASF_DIR)/common/services/usb/udc/udc.c \
   $(ASF_DIR)/xmega/drivers/crc/crc.c  \
   $(ASF_DIR)/xmega/drivers/usb/usb_device.c  \
   $(ASF_DIR)/xmega/drivers/wdt/wdt.c \
   $(ASF_DIR)/common/components/display/ssd1306/ssd1306.c \
   $(ASF_DIR)/common/services/gfx_mono/gfx_mono_framebuffer.c  \
   $(ASF_DIR)/common/services/gfx_mono/gfx_mono_generic.c \
   $(ASF_DIR)/common/services/gfx_mono/gfx_mono_text.c  \
   $(ASF_DIR)/common/services/gfx_mono/gfx_mono_ug_2832hsweg04.c  \
   $(ASF_DIR)/common/services/gfx_mono/sysfont.c \
   $(ASF_DIR)/xmega/drivers/spi/spi.c \
   $(ASF_DIR)/xmega/drivers/nvm/nvm.c \
   $(ASF_DIR)/xmega/drivers/twi/twim.c  \
   $(ASF_DIR)/common/services/ioport/xmega/ioport_compat.c  \
   $(ASF_DIR)/common/services/sleepmgr/xmega/sleepmgr.c \
   $(ASF_DIR)/common/services/clock/xmega/sysclk.c \
   $(ASF_DIR)/xmega/drivers/tc/tc.c \
   $(ASF_DIR)/common/boards/user_board/init.c \
   $(ASF_DIR)/xmega/drivers/nvm/nvm_asm.s  \
   $(ASF_DIR)/xmega/drivers/cpu/ccp.s \
   trace.c \
   keypad.cpp \
   c++/src/new.cpp \

# Inlude the actual build rules
include make/rules.mak
