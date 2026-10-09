CC ?= cc
CPPFLAGS ?= -Isrc
CFLAGS ?= -std=c11 -O2 -Wall -Wextra -Wpedantic -Werror
LDFLAGS ?=
BUILD := build
CORE := src/event.c src/debounce.c src/replay.c src/gpio_source.c

.PHONY: all test clean
all: $(BUILD)/gpio-recorder

$(BUILD):
	mkdir -p $(BUILD)

$(BUILD)/gpio-recorder: src/main.c $(CORE) | $(BUILD)
	$(CC) $(CPPFLAGS) $(CFLAGS) $^ $(LDFLAGS) -o $@

$(BUILD)/test_event: tests/test_event.c src/event.c | $(BUILD)
	$(CC) $(CPPFLAGS) $(CFLAGS) $^ $(LDFLAGS) -o $@

$(BUILD)/test_debounce: tests/test_debounce.c src/debounce.c | $(BUILD)
	$(CC) $(CPPFLAGS) $(CFLAGS) $^ $(LDFLAGS) -o $@

$(BUILD)/test_replay: tests/test_replay.c src/replay.c src/event.c | $(BUILD)
	$(CC) $(CPPFLAGS) $(CFLAGS) $^ $(LDFLAGS) -o $@

test: $(BUILD)/test_event $(BUILD)/test_debounce $(BUILD)/test_replay $(BUILD)/gpio-recorder
	$(BUILD)/test_event
	$(BUILD)/test_debounce
	$(BUILD)/test_replay
	$(BUILD)/gpio-recorder record --source demo --duration-ms 1000 --output $(BUILD)/test_events.csv
	$(BUILD)/gpio-recorder replay --input $(BUILD)/test_events.csv --no-sleep

clean:
	rm -rf $(BUILD)
