CC = gcc
PKG_CONFIG ?= pkg-config

rwildcard = $(foreach dir,$(wildcard $(1)/*),$(call rwildcard,$(dir),$(2))) $(filter $(subst *,%,$(2)),$(wildcard $(1)/$(2)))

SRC := $(sort $(call rwildcard,src,*.c))
OBJ := $(patsubst src/%.c,build/%.o,$(SRC))
DEP := $(OBJ:.o=.d)

BIN_DIR := bin
BUILD_DIR := build
TARGET_NAME := open-sosaria
TARGET := $(BIN_DIR)/$(TARGET_NAME)

CPPFLAGS += -Isrc
CFLAGS += -std=c99 -Wall -Wextra -Werror -MMD -MP
LDFLAGS +=
LDLIBS +=
STRIP_FLAGS := -s
GUI_LDFLAGS :=
DEBUG_CPPFLAGS := -DDEBUG

GLFW_CFLAGS := $(strip $(shell $(PKG_CONFIG) --cflags glfw3 2>/dev/null))
GLFW_LIBS := $(strip $(shell $(PKG_CONFIG) --libs glfw3 2>/dev/null))

ifeq ($(OS),Windows_NT)
  TARGET := $(TARGET).exe
  CPPFLAGS += -DCR_WINDOWS
  GUI_LDFLAGS += -mwindows

  ifneq ($(GLFW_CFLAGS)$(GLFW_LIBS),)
    CPPFLAGS += $(GLFW_CFLAGS)
    LDLIBS += $(GLFW_LIBS)
  else
    CPPFLAGS += -IC:/CLibs/include
    LDFLAGS += -LC:/CLibs/libs
    LDLIBS += -lglfw3
  endif

  LDLIBS += -lopengl32 -lgdi32 -luser32 -lkernel32
else
  UNAME_S := $(shell uname -s)

  CPPFLAGS += -DCR_UNIX

  ifneq ($(GLFW_CFLAGS)$(GLFW_LIBS),)
    CPPFLAGS += $(GLFW_CFLAGS)
    LDLIBS += $(GLFW_LIBS)
  endif

  ifeq ($(UNAME_S),Linux)
    CPPFLAGS += -D_POSIX_C_SOURCE=200809L

    ifeq ($(GLFW_CFLAGS)$(GLFW_LIBS),)
      LDLIBS += -lglfw
    endif

    LDLIBS += -lGL -lX11 -lXrandr -lXi -ldl -lm -lpthread
  endif

  ifeq ($(UNAME_S),Darwin)
    ifeq ($(GLFW_CFLAGS)$(GLFW_LIBS),)
      BREW_PREFIX := $(strip $(shell brew --prefix 2>/dev/null))
      ifneq ($(BREW_PREFIX),)
        CPPFLAGS += -I$(BREW_PREFIX)/include
        LDFLAGS += -L$(BREW_PREFIX)/lib
      endif
      LDLIBS += -lglfw
    endif

    LDLIBS += -framework Cocoa -framework IOKit -framework CoreVideo
  endif
endif

all: $(TARGET)

$(TARGET): $(OBJ)
	@mkdir -p $(dir $@)
	$(CC) $(OBJ) $(LDFLAGS) $(GUI_LDFLAGS) $(LDLIBS) $(STRIP_FLAGS) -o $@

# Third-party source: relax warning promotion for glad only.
build/dependencies/glad.o: CFLAGS := $(filter-out -Werror -pedantic-errors,$(CFLAGS)) -Wno-error -Wno-pedantic

build/%.o: src/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

debug: CPPFLAGS += $(DEBUG_CPPFLAGS)
debug: STRIP_FLAGS :=
debug: GUI_LDFLAGS :=
debug: CFLAGS += -g
debug: $(TARGET)

clean:
	rm -f $(BIN_DIR)/$(TARGET_NAME) $(BIN_DIR)/$(TARGET_NAME).exe
	rm -rf $(BUILD_DIR)

.PHONY: all debug clean

-include $(DEP)