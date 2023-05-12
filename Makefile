EXEC = main
CC = g++

SRC_DIR = src
BUILD_DIR = build
INCLUDE_DIR = include

LDFLAGS = -lSDL2 -lm
CFLAGS = -Werror -Wall -std=c++2a -g -I$(INCLUDE_DIR)

SRCS = $(shell find $(SRC_DIR) -name '*.cpp')
OBJS = $(patsubst $(SRC_DIR)/%.cpp, $(BUILD_DIR)/%.o, $(SRCS))

$(EXEC): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp
	$(CC) $(CFLAGS) -c $^ -o $@

clean:
	rm $(BUILD_DIR)/*.o $(EXEC)
