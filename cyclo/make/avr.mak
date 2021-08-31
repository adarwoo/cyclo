# Change the compiler
CC  := /opt/avr-gcc-11-20210501-x64-linux/bin/avr-gcc
CXX := /opt/avr-gcc-11-20210501-x64-linux/bin/avr-g++

CFLAGS += -funsigned-char -funsigned-bitfields -ffunction-sections -fdata-sections -fshort-enums -g2 -Wall -mmcu=atxmega128a4u
CXXFLAGS += -fno-threadsafe-statics

LDFLAGS         += -Wl,-Map="$(BIN).map" -Wl,--start-group -Wl,-lm  -Wl,--end-group -Wl,--gc-sections -mmcu=atxmega128a4u -Wl,--demangle -Wl,-flto 

# atmel\XMEGAA_DFP\1.2.141\include