CC = gcc
CFLAGS = -g -Wall -Werror -Iinclude -L/usr/lib64 -I/usr/include
LDFLAGS = -lraylib -lm

SRCS = $(wildcard src/*.c)
OBJS = $(SRCS:src/%.c=$(BUILD_DIR)/%.o)

BUILD_DIR = build

TARGET = $(BUILD_DIR)/WhiskerRenderer
TEST_FONT = Dosis-VariableFont_wght.ttf

$(TARGET): $(OBJS)
	$(CC) -o $@ $^ $(LDFLAGS)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(BUILD_DIR)/%.o: src/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)

run: $(TARGET)
	./$(TARGET) $(TEST_FONT)

.PHONY: clean run
