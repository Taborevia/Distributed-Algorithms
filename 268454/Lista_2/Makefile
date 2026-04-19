CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -I$(INC_DIR)

SRC_DIR = src
INC_DIR = inc

TARGET = simulator
SRCS = $(SRC_DIR)/main.cpp $(SRC_DIR)/GraphAL.cpp

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(SRCS)
	$(CXX) $(CXXFLAGS) -o $@ $^

clean:
	rm -f $(TARGET)
