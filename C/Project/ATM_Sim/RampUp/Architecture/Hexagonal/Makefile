CC = gcc
CFLAGS = -Wall -Wextra -std=c11

# Include paths
INCLUDES = \
    -Ibusiness_logic \
    -Ioutbound_adapter \
    -Ioutbound_port

# Source directories
SRC_DIR = .
BL_DIR = business_logic
ADAPTER_DIR = outbound_adapter

# Source files
SRCS = \
    $(SRC_DIR)/main.c \
    $(BL_DIR)/atm.c \
    $(ADAPTER_DIR)/*.c

OBJS = $(SRCS:.c=.o)

TARGET = atm_app

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)

.PHONY: all clean
