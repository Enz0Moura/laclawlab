CC = cc
AR = ar
BUILD ?= build
CPPFLAGS += -Iinclude -Isrc
CFLAGS ?= -O2 -g
CFLAGS += -std=c11 -Wall -Wextra -Wpedantic -MMD -MP
LDLIBS += -lglut -lGL -lm

CG_SOURCES := $(wildcard src/cg/*.c)
CG_OBJECTS := $(patsubst %.c,$(BUILD)/%.o,$(CG_SOURCES))
DEMO_OBJECT := $(BUILD)/examples/library_demo.o

.PHONY: all lib demo example clean
all: $(BUILD)/library_demo
lib: $(BUILD)/libcg.a
demo: $(BUILD)/library_demo
example: $(BUILD)/library_demo
	./$(BUILD)/library_demo

$(BUILD)/libcg.a: $(CG_OBJECTS)
	$(AR) rcs $@ $^
$(BUILD)/library_demo: $(DEMO_OBJECT) $(BUILD)/libcg.a
	$(CC) $(LDFLAGS) -o $@ $^ $(LDLIBS)
$(BUILD)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@
clean:
	rm -rf build

-include $(CG_OBJECTS:.o=.d) $(DEMO_OBJECT:.o=.d)
