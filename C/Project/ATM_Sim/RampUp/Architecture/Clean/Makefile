CC = gcc
CFLAGS = -Wall -Wextra -I./adapter -I./domain -I./infra -I./usecase

SRC = $(wildcard *.c) \
      $(wildcard adapter/*.c) \
      $(wildcard domain/*.c) \
      $(wildcard infra/*.c) \
      $(wildcard usecase/*.c)

OBJ = $(SRC:.c=.o)

TARGET = app

build: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^

run: build
	./$(TARGET)

clean:
	rm -f $(OBJ) $(TARGET)
