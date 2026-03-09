CC = gcc
CFLAGS = -std=c99

files := $(wildcard src/*.c) $(wildcard src/**/*.c) $(wildcard src/**/**/*.c)
outputDir := bin/
objectDir := build/
LDFlags := 
OSBuildFlags := -s
OSDebugFlags := -DDEBUG

ifeq ($(OS), Windows_NT)
  LDFlags += -IC:/CLibs/include -I$(CURDIR)/src -LC:/CLibs/libs -lglfw3 -lopengl32 -lgdi32 -luser32 -lkernel32 -Wall -Wextra -Werror -DCR_WINDOWS
  OSBuildFlags += -mwindows
else
  UNAME_S := $(shell uname -s)

  ifeq ($(UNAME_S), Linux)
    LDFlags += -I/usr/local/include -I$(CURDIR)/src -L/usr/local/lib -lglfw -lGL -lX11 -lXrandr -lXi -ldl -lm -lpthread -DCR_UNIX -D_POSIX_C_SOURCE=200809L -pedantic-errors -Wall -Wextra -Werror
  endif

  ifeq ($(UNAME_S), Darwin)
    LDFlags += -I/usr/local/include -I$(CURDIR)/src -L/usr/local/lib -lglfw -DCR_UNIX
  endif
endif

objects := $(patsubst src/%.c, $(objectDir)%.o, $(files))

open-sosaria: $(objects) makeOutputDir
	$(CC) $(CFLAGS) $(objects) $(LDFlags) -o $(outputDir)open-sosaria $(OSBuildFlags)

# Special rule: compile glad.c without -Werror and -pedantic-errors
$(objectDir)dependencies/glad.o: src/dependencies/glad.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(filter-out -Werror -pedantic-errors, $(LDFlags)) -Wno-error -Wno-pedantic -c $< -o $@

$(objectDir)%.o: src/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(LDFlags) -c $< -o $@

debug: CFLAGS += -g $(OSDebugFlags)
debug: OSBuildFlags :=
debug: open-sosaria

makeOutputDir:
	mkdir -p $(outputDir)

clean:
	rm -f $(outputDir)open-sosaria 
	rm -rf $(objectDir)