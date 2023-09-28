CXX = g++
CFLAGS = -Wall -Werror -std=c++1z -g
INCLUDES = -Iinclude
LFLAGS = -lamqpcpp -lpthread -ldl -lev -ljsoncpp -lz -lcurl -lssl -lcrypto -lboost_system  -lgtest -lgtest_main -lpugixml -lpcre2-8

# Source files and directories
SRC_DIR = src
SRCS = $(wildcard $(SRC_DIR)/*.cpp)

# Test source files and directories
TEST_DIR = test
TEST_SRCS = $(wildcard $(TEST_DIR)/*.cpp)

# Object files and directories
OBJ_DIR = obj
OBJS = $(SRCS:$(SRC_DIR)/%.cpp=$(OBJ_DIR)/%.o)
TEST_OBJS = $(TEST_SRCS:$(TEST_DIR)/%.cpp=$(OBJ_DIR)/%.o)

# Executable file and directories
BIN_DIR = bin
TARGET = $(BIN_DIR)/agent
TEST_TARGET = $(BIN_DIR)/test

# Build rules
all: $(TARGET)

$(TARGET): $(OBJS)
	@mkdir -p $(BIN_DIR)
	$(CXX) $(CFLAGS) $(INCLUDES) $^ -o $@ $(LFLAGS)

test: $(TEST_TARGET)

$(TEST_TARGET): $(TEST_OBJS) $(OBJS)
	@mkdir -p $(BIN_DIR)
	$(CXX) $(CFLAGS) $(INCLUDES) $^ -o $@ $(LFLAGS) 

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CFLAGS) $(INCLUDES) -c $< -o $@ $(LFLAGS) 

$(OBJ_DIR)/%.o: $(TEST_DIR)/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CFLAGS) $(INCLUDES) -c $< -o $@ $(LFLAGS)

clean:
	rm -rf $(OBJ_DIR) $(BIN_DIR)

deploy: stop_service copy_binary start_service

stop_service:
	@echo "Stopping agent service..."
	@sudo systemctl stop agent.service

copy_binary:
	@echo "Copying binary to root directory"
	@sudo cp $(TARGET) /etc/scl/bin/

start_service:
	@echo "Starting agent service..."
	@sudo systemctl start agent.service