CC = gcc
CFLAGS = -Wall -Wextra -std=c11

# Include paths
INCLUDES = \
    -Ibusiness_logic \
    -Ioutbound_adapter \
    -Iinbound_adapter \
    -Ioutbound_port \

# Directories
SRC_DIR = .
BL_DIR = business_logic
OUTBOUND_ADAPTER_DIR = outbound_adapter
INBOUND_ADAPTER_DIR = inbound_adapter
BUILD_DIR = build

# Collect all .c files
SRCS := $(wildcard $(SRC_DIR)/*.c) \
        $(wildcard $(BL_DIR)/*.c) \
        $(wildcard $(OUTBOUND_ADAPTER_DIR)/*.c) \
        $(wildcard $(INBOUND_ADAPTER_DIR)/*.c)

# Convert source paths → build/*.o paths
OBJS := $(SRCS:%=$(BUILD_DIR)/%.o)

TARGET = $(BUILD_DIR)/atm_app

# Build rule
build: $(BUILD_DIR) $(TARGET)

# Create build directory if missing
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# Link final binary
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ $^

# Compile .c → build/*.o
$(BUILD_DIR)/%.c.o: %.c
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

# Run the program with parameters
run: build
	./$(TARGET) $(RUN_ARGS)

clean:
	rm -rf $(BUILD_DIR)

.PHONY: build clean run
