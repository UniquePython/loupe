CC := gcc

TARGET := bin/loupe

SRC := $(wildcard src/*.c)
OBJ := $(patsubst src/%.c,build/%.o,$(SRC))
DEP := $(OBJ:.o=.d)

WARNINGS := \
	-Wall \
	-Wextra \
	-Wpedantic \
	-Wshadow \
	-Wconversion \
	-Wsign-conversion \
	-Wcast-qual \
	-Wwrite-strings \
	-Wformat=2 \
	-Wundef \
	-Wstrict-prototypes \
	-Wold-style-definition \
	-Wimplicit-fallthrough \
	-Wlogical-op \
	-Wcast-align \
	-Wvla \
	-Wnull-dereference \
	-Wdouble-promotion \
	-Wformat-overflow=2 \
	-Wformat-truncation=2 \
	-Walloc-zero \
	-Warray-bounds=2 \
	-Wstringop-overflow=4 \
	-Wstrict-overflow=5 \
	-Wswitch-enum \
	-Wpointer-arith \
	-Winit-self

CFLAGS := -std=c11 -g $(WARNINGS)
CFLAGS += -Iinclude
CFLAGS += $(shell pkg-config --cflags x11)

LDLIBS := $(shell pkg-config --libs x11)

.PHONY: all clean run rebuild

all: $(TARGET)

$(TARGET): $(OBJ)
	@mkdir -p $(@D)
	$(CC) $(OBJ) -o $@ $(LDLIBS)

build/%.o: src/%.c
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -MMD -MP -c $< -o $@

-include $(DEP)

run: $(TARGET)
	./$(TARGET)

clean:
	rm -rf build bin

rebuild: clean all
