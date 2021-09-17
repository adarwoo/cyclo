# Make rules specific to the simulator

# Forcing debug in all cases - what's the point of the simulator otherwise?
# Turn it of with NDEBUG=1
ifndef NDEBUG
DEBUG=1
endif

tc_prefix:=
CPPFLAGS += -D_POSIX
LDFLAGS += -pthread

ifdef DEBUG
  CPPFLAGS += -DDEBUG
  CFLAGS += \
    -fsanitize=address \
    -fsanitize=alignment \
    -fno-omit-frame-pointer \
	-O$(if $(DEBUG),0,3)

  LDFLAGS += -lrt -fsanitize=address -fsanitize=alignment -static-libasan -static-libstdc++ -lX11
endif

OBJS = $(foreach file, $(SRCS.common) $(SRCS.sim) $(SRCS.rc), $(BUILD_DIR)/$(basename $(file)).o)
