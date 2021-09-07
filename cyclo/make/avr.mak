# Change the compiler
CC  := avr-gcc
CXX := avr-g++
SIZE := avr-size

# Remove all logs
CPPFLAGS += -DFORCE_NODEBUG

CFLAGS += -funsigned-char -funsigned-bitfields -ffunction-sections -fdata-sections -fshort-enums -mmcu=atxmega128a4u
CFLAGS += -O$(if $(DEBUG),g,s)
CXXFLAGS += -fno-threadsafe-statics

LDFLAGS         += -Wl,-Map="$(BIN).map" -Wl,--start-group -Wl,-lm  -Wl,--end-group -Wl,--gc-sections -mmcu=atxmega128a4u -Wl,--demangle -Wl,-flto

OBJS = $(foreach file, $(SRCS.common) $(SRCS.avr), $(BUILD_DIR)/$(basename $(file)).o)

define DIAG
$(mute)$(SIZE) $@ | awk 'NR!=1 {print "Flash: [" $$1 "]" $$1 * 100 / 128 / 1024 "% - RAM: [" $$2 "+" $$3 "]" ($$2 + $$3) * 100 / 8192 "%"}'

endef
