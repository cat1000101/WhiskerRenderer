CC = gcc
CFLAGS = -Wall -Werror

SRCS = src/main.c
OBJS = $(SRCS:.c=.o)

TARGET = WhiskerRenderer

$(TARGET): $(OBJS)
	$(CC) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)

run: $(TARGET)
	./$(TARGET) Dosis-VariableFont_wght.ttf

.PHONY: clean run