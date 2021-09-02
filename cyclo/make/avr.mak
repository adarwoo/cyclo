# Change the compiler
CC  := avr-gcc
CXX := avr-g++
SIZE := avr-size

CFLAGS += -funsigned-char -funsigned-bitfields -ffunction-sections -fdata-sections -fshort-enums -mmcu=atxmega128a4u
CXXFLAGS += -fno-threadsafe-statics

LDFLAGS         += -Wl,-Map="$(BIN).map" -Wl,--start-group -Wl,-lm  -Wl,--end-group -Wl,--gc-sections -mmcu=atxmega128a4u -Wl,--demangle -Wl,-flto
