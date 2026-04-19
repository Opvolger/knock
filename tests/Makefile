CC      = gcc
CFLAGS  = -g -Wall -pedantic -I..
SRCS    = list_test.c ../src/list.c
TARGET  = list_test

.PHONY: all check memcheck clean

all: $(TARGET)

$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) -o $@ $^

check: $(TARGET)
	./$(TARGET)

memcheck: $(TARGET)
	valgrind --leak-check=full --error-exitcode=1 ./$(TARGET)

clean:
	rm -f $(TARGET)
