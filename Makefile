CXX = g++
CFLAGS = -Wall -Werror -std=c++1z -g
INCLUDES = -Iinclude -I/usr/include/jsoncpp
LFLAGS = -lpthread -ljsoncpp -lz -lcurl -lssl -lcrypto -lgtest -lgtest_main -lpugixml -lpcre2-8 -lpaho-mqttpp3 -lpaho-mqtt3a
# LFLAGS = -lamqpcpp -lpthread -ldl -lev -ljsoncpp -lz -lcurl -lssl -lcrypto -lboost_system  -lgtest -lgtest_main -lpugixml -lpcre2-8


UNIT_FILE = /etc/systemd/system/agent.service 
# Source files and directories
SRC_DIR = src
SRCS = $(wildcard $(SRC_DIR)/*.cpp)
# SRCS := $(filter-out $(SRC_DIR)/rqueue.cpp, $(wildcard $(SRC_DIR)/*.cpp))
# SRCS := $(filter-out $(addprefix $(SRC_DIR)/, rqueue.cpp watchservice.cpp connection.cpp), $(wildcard $(SRC_DIR)/*.cpp))

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

deploy: init copy_binary enable start_service 

enable:
	echo "enabling agent.service"
	@sudo systemctl enable --now agent.service

init:

	@if [ ! -e $(UNIT_FILE) ]; then \
		touch $(UNIT_FILE); \
		echo "[Unit]" > $(UNIT_FILE); \
		echo "Description=Security agent for this device." >> $(UNIT_FILE); \
		echo "After=network.target" >> $(UNIT_FILE); \
		echo "" >> $(UNIT_FILE); \
		echo "[Service]" >> $(UNIT_FILE); \
		echo "User=root" >> $(UNIT_FILE); \
		echo "Group=root" >> $(UNIT_FILE); \
		echo "WorkingDirectory=/etc/scl/" >> $(UNIT_FILE); \
		echo "ExecStart=/etc/scl/bin/agent" >> $(UNIT_FILE); \
		echo "RestartSec=5" >> $(UNIT_FILE); \
		echo "Restart=always" >> $(UNIT_FILE); \
		echo "" >> $(UNIT_FILE); \
		echo "[Install]" >> $(UNIT_FILE); \
		echo "WantedBy=multi-user.target" >> $(UNIT_FILE); \
		echo "Content written to $(UNIT_FILE)." && \
		echo "File $(UNIT_FILE) created."; \
		@sudo systemctl daemon-reload; \
	else \
		echo "Unit file $(UNIT_FILE) already exists."; \
	fi

stop_service:
	@echo "Stopping agent service..."
	@sudo systemctl stop agent.service

copy_binary:
	@echo "Copying binary to root directory"
	@if [ ! -d "/etc/scl/bin" ]; then \
		mkdir -p /etc/scl/bin; \
	fi
	@sudo cp $(TARGET) /etc/scl/bin/

start_service:
	@echo "Starting agent service..."
	@sudo systemctl start agent.service
