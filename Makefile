CXX      := g++
CXXFLAGS := -std=c++17 -Wall
INCLUDES := -I.

SRCS := \
    main.cpp \
    framework/framework.cpp \
    system/connection.cpp \
    system/eventloop.cpp

DEBUG_DIR := debug
DEBUG_TARGET := $(DEBUG_DIR)/server

OBJS := $(SRCS:.cpp=.o)

TARGET := server

# For the build release
all: $(TARGET)

$(TARGET): CXXFLAGS += -O2
$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) $(OBJS) -o $@

# For the debug
debug: CXXFLAGS += -g -O0
debug: $(DEBUG_DIR) $(DEBUG_TARGET)

$(DEBUG_DIR):
	mkdir -p $(DEBUG_DIR)

$(DEBUG_TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) $(OBJS) -o $@

# For handling the temp .o and other files
%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

clean:
	rm -fr $(OBJS) $(TARGET) $(DEBUG_DIR)/


.PHONY: all clean debug
