# Change the compiler
CC  := avr-gcc
CXX := avr-g++
SIZE := avr-size

BIN_EXT :=.elf

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

define POST_LINK
	$(mute)avr-objcopy -O ihex -R .eeprom -R .fuse -R .lock -R .signature -R .user_signatures  "$@" "${@:.elf=.hex}"
	$(mute)avr-objcopy -j .eeprom  --set-section-flags=.eeprom=alloc,load --change-section-lma .eeprom=0  --no-change-warnings -O ihex "$@" "${@:.elf=.eep}" || exit 0
	$(mute)avr-objdump -h -S "$@" > "${@:.elf=.lss}"
	$(mute)avr-objcopy -O srec -R .eeprom -R .fuse -R .lock -R .signature -R .user_signatures "$@" "${@:.elf=.lss}"
endef
