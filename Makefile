EXEC = main
CC = clang++

SRC_DIR = src
BUILD_DIR = build
INCLUDE_DIR = include

CFLAGS = -Werror -Wall -std=c++20 -g -I$(INCLUDE_DIR) `pkg-config --cflags libavcodec libavutil`
CFLAGS += `pkg-config --cflags sdl2`
LIBS = `pkg-config --libs libavcodec libavutil libavformat libswscale`
LIBS += -lSDL2

SRCS = $(shell find $(SRC_DIR) -name '*.cpp')
OBJS = $(patsubst $(SRC_DIR)/%.cpp, $(BUILD_DIR)/%.o, $(SRCS))

$(EXEC): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp
	$(CC) $(CFLAGS) -c $^ -o $@

clean:
	rm $(BUILD_DIR)/*.o $(EXEC)
