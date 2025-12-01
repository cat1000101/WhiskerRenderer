CC = gcc
CFLAGS = -Wall -Werror -Iinclude

SRCS = $(wildcard src/*.c)
OBJS = $(SRCS:src/%.c=$(BUILD_DIR)/%.o)

BUILD_DIR = build

TARGET = $(BUILD_DIR)/WhiskerRenderer

$(TARGET): $(OBJS)
	$(CC) -o $@ $^

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(BUILD_DIR)/%.o: src/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)

run: $(TARGET)
	./$(TARGET) Dosis-VariableFont_wght.ttf

.PHONY: clean run