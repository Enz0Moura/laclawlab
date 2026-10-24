CC = cc
AR = ar
BUILD ?= build
CPPFLAGS += -Iinclude -Isrc
CFLAGS ?= -O2 -g
CFLAGS += -std=c11 -Wall -Wextra -Wpedantic -MMD -MP
LDLIBS += -lglut -lGL -lm

CG_SOURCES := $(wildcard src/cg/*.c)
CG_OBJECTS := $(patsubst %.c,$(BUILD)/%.o,$(CG_SOURCES))
APP_SOURCES := src/app/config.c src/app/machine.c src/app/theme.c src/app/ui.c src/app/main.c
APP_OBJECTS := $(patsubst %.c,$(BUILD)/%.o,$(APP_SOURCES))
DEMO_OBJECT := $(BUILD)/examples/library_demo.o

.PHONY: all lib demo run example clean
all: $(BUILD)/prize_machine $(BUILD)/library_demo
lib: $(BUILD)/libcg.a
demo: $(BUILD)/library_demo
run: $(BUILD)/prize_machine
	./$(BUILD)/prize_machine
example: $(BUILD)/library_demo
	./$(BUILD)/library_demo

$(BUILD)/libcg.a: $(CG_OBJECTS)
	$(AR) rcs $@ $^
$(BUILD)/prize_machine: $(APP_OBJECTS) $(BUILD)/libcg.a
	$(CC) $(LDFLAGS) -o $@ $^ $(LDLIBS)
$(BUILD)/library_demo: $(DEMO_OBJECT) $(BUILD)/libcg.a
	$(CC) $(LDFLAGS) -o $@ $^ $(LDLIBS)
$(BUILD)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@
clean:
	rm -rf build

-include $(CG_OBJECTS:.o=.d) $(APP_OBJECTS:.o=.d) $(DEMO_OBJECT:.o=.d)
