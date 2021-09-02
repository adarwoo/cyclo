tc_prefix:=

CPPFLAGS = -D_POSIX

ifdef DEBUG
  CFLAGS := \
    -fsanitize=address \
    -fsanitize=alignment \
    -fno-omit-frame-pointer \
    -static-libstdc++ \
    -static-libasan

  LDFLAGS += -lrt -fsanitize=address -fsanitize=alignment
endif

