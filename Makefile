CXX := g++
CFLAGS := -Wall -Werror -std=c++1z -g
LFLAGS := -lamqpcpp -lpthread -ldl -lev -ljsoncpp -lz -lcurl -lssl -lcrypto -lboost_system -lpugixml -lpcre2-8

# Source files and directories
SRC_DIR := src
TEST_DIR := test

LOG_ANALYSIS := $(SRC_DIR)/analysis
LOG_COLLECTOR := $(SRC_DIR)/log-collector
PATCH_MANAGEMENT := $(SRC_DIR)/patch
DEVICE_MONITOR := $(SRC_DIR)/monitor
ROOTCHECK := $(SRC_DIR)/rootcheck
COMMON := $(SRC_DIR)/utils


LOG_ANALYSIS_SRCS = $(wildcard $(LOG_ANALYSIS)/*.cpp)
LOG_COLLECTOR_SRCS = $(wildcard $(LOG_COLLECTOR)/*.cpp)
PATCH_MANAGEMENT_SRCS = $(wildcard $(PATCH_MANAGEMENT)/*.cpp)
DEVICE_MONITOR_SRCS = $(wildcard $(DEVICE_MONITOR)/*.cpp)
ROOTCHECK_SRCS = $(wildcard $(ROOTCHECK)/*.cpp)
COMMON_SRCS = $(wildcard $(COMMON)/*.cpp)
TEST_SRCS = $(wildcard $(TEST_DIR)/*.cpp)

LOG_ANALYSIS_OBJS = $(LOG_ANALYSIS_SRCS:.cpp=.o)
LOG_COLLECTOR_OBJS = $(LOG_COLLECTOR_SRCS:.cpp=.o)
PATCH_MANAGEMENT_OBJS = $(PATCH_MANAGEMENT_SRCS:.cpp=.o)
DEVICE_MONITOR_OBJS = $(DEVICE_MONITOR_SRCS:.cpp=.o)
ROOTCHECK_OBJS = $(ROOTCHECK_SRCS:.cpp=.o)
COMMON_OBJS = $(COMMON_SRCS:.cpp=.o)
TEST_OBJS = $(TEST_SRCS:.cpp=.o)

all: log-analysis log-collector patch-management device-monitor rootcheck

log-analysis: $(LOG_ANALYSIS_OBJS) $(COMMON_OBJS)
	$(CXX) $(CFLAGS) -o $@ $^ $(LFLAGS)

log-collector: $(LOG_COLLECTOR_OBJS) $(COMMON_OBJS)
	$(CXX) $(CFLAGS) -o $@ $^ $(LFLAGS)

patch-management: $(PATCH_MANAGEMENT_OBJS) $(COMMON_OBJS)
	$(CXX) $(CFLAGS) $^ -o $@ $(LFLAGS)

device-monitor: $(DEVICE_MONITOR_OBJS) $(COMMON_OBJS)
	$(CXX) $(CFLAGS) $^ -o $@ $(LFLAGS)

rootcheck: $(ROOTCHECK_OBJS) $(COMMON_OBJS)
	$(CXX) $(CFLAGS) $^ -o $@ $(LFLAGS)

test: test-log-analysis test-log-collector test-patch-management test-device-monitor test-rootcheck

test-log-analysis: $(LOG_ANALYSIS_OBJS) $(COMMON_OBJS) $(TEST_OBJS)
	$(CXX) $(CFLAGS) $^ -o $@ $(LFLAGS)

test-log-collector: $(LOG_COLLECTOR_OBJS) $(COMMON_OBJS) $(TEST_OBJS)
	$(CXX) $(CFLAGS) $^ -o $@ $(LFLAGS)

test-patch-management: $(PATCH_MANAGEMENT_OBJS) $(COMMON_OBJS) $(TEST_OBJS)
	$(CXX) $(CFLAGS) $^ -o $@ $(LFLAGS)

test-device-monitor: $(DEVICE_MONITOR_OBJS) $(COMMON_OBJS) $(TEST_OBJS)
	$(CXX) $(CFLAGS) $^ -o $@ $(LFLAGS)

test-rootcheck: $(ROOTCHECK_OBJS) $(COMMON_OBJS) $(TEST_OBJS)
	$(CXX) $(CFLAGS) $^ -o $@ $(LFLAGS) -lgtest -lgtest_main

%.o: %.cpp
	$(CXX) $(CFLAGS) -Isrc -c $< -o $@ $(LFLAGS) 

clean:
	rm -f $(LOG_ANALYSIS_OBJS) $(LOG_COLLECTOR_OBJS) $(PATCH_MANAGEMENT_OBJS) $(DEVICE_MONITOR_OBJS) $(ROOTCHECK_OBJS) log-analysis log-collector patch-management device-monitor rootcheck test-* *.o

.PHONY: all clean




# CXX := g++
# CFLAGS := -Wall -Werror -std=c++1z -g -Isrc
# LFLAGS := -lamqpcpp -lpthread -ldl -lev -ljsoncpp -lz -lcurl -lssl -lcrypto -lboost_system -lpugixml -lpcre2-8 -lgtest -lgtest_main

# # Directories
# SRC_DIR := src
# OBJ_DIR := obj
# BIN_DIR := bin
# TEST_DIR := test

# LOG_ANALYSIS := $(SRC_DIR)/analysis
# LOG_COLLECTOR := $(SRC_DIR)/log-collector
# PATCH_MANAGEMENT := $(SRC_DIR)/patch
# DEVICE_MONITOR := $(SRC_DIR)/monitor
# ROOTCHECK := $(SRC_DIR)/rootcheck
# COMMON := $(SRC_DIR)/utils

# # Source files
# LOG_ANALYSIS_SRCS = $(wildcard $(LOG_ANALYSIS)/*.cpp)
# LOG_COLLECTOR_SRCS = $(wildcard $(LOG_COLLECTOR)/*.cpp)
# PATCH_MANAGEMENT_SRCS = $(wildcard $(PATCH_MANAGEMENT)/*.cpp)
# DEVICE_MONITOR_SRCS = $(wildcard $(DEVICE_MONITOR)/*.cpp)
# ROOTCHECK_SRCS = $(wildcard $(ROOTCHECK)/*.cpp)
# COMMON_SRCS = $(wildcard $(COMMON)/*.cpp)
# TEST_SRCS = $(wildcard $(TEST_DIR)/*.cpp)

# # Object files
# LOG_ANALYSIS_OBJS = $(LOG_ANALYSIS_SRCS:$(LOG_ANALYSIS)/%.cpp=$(OBJ_DIR)/%.o)
# LOG_COLLECTOR_OBJS = $(LOG_COLLECTOR_SRCS:$(LOG_COLLECTOR)/%.cpp=$(OBJ_DIR)/%.o)
# PATCH_MANAGEMENT_OBJS = $(PATCH_MANAGEMENT_SRCS:$(PATCH_MANAGEMENT)/%.cpp=$(OBJ_DIR)/%.o)
# DEVICE_MONITOR_OBJS = $(DEVICE_MONITOR_SRCS:$(DEVICE_MONITOR)/%.cpp=$(OBJ_DIR)/%.o)
# ROOTCHECK_OBJS = $(ROOTCHECK_SRCS:$(ROOTCHECK)/%.cpp=$(OBJ_DIR)/%.o)
# COMMON_OBJS = $(COMMON_SRCS:$(COMMON)/%.cpp=$(OBJ_DIR)/%.o)
# TEST_OBJS = $(TEST_SRCS:$(TEST_DIR)/%.cpp=$(OBJ_DIR)/%.o)

# # Main Targets
# all: directories log-analysis log-collector patch-management device-monitor rootcheck test

# # Create necessary directories
# directories:
# 	mkdir -p $(OBJ_DIR)
# 	mkdir -p $(BIN_DIR)

# # Binary targets
# log-analysis: $(LOG_ANALYSIS_OBJS) $(COMMON_OBJS)
# 	$(CXX) $(CFLAGS) -o $(BIN_DIR)/$@ $^ $(LFLAGS)

# log-collector: $(LOG_COLLECTOR_OBJS) $(COMMON_OBJS)
# 	$(CXX) $(CFLAGS) -o $(BIN_DIR)/$@ $^ $(LFLAGS)

# patch-management: $(PATCH_MANAGEMENT_OBJS) $(COMMON_OBJS)
# 	$(CXX) $(CFLAGS) -o $(BIN_DIR)/$@ $^ $(LFLAGS)

# device-monitor: $(DEVICE_MONITOR_OBJS) $(COMMON_OBJS)
# 	$(CXX) $(CFLAGS) -o $(BIN_DIR)/$@ $^ $(LFLAGS)

# rootcheck: $(ROOTCHECK_OBJS) $(COMMON_OBJS)
# 	$(CXX) $(CFLAGS) -o $(BIN_DIR)/$@ $^ $(LFLAGS)

# test: test-log-analysis test-log-collector test-patch-management test-device-monitor test-rootcheck

# test-log-analysis: $(LOG_ANALYSIS_OBJS) $(COMMON_OBJS) $(TEST_OBJS)
# 	$(CXX) $(CFLAGS) -o $(BIN_DIR)/$@ $^ $(LFLAGS)

# test-log-collector: $(LOG_COLLECTOR_OBJS) $(COMMON_OBJS) $(TEST_OBJS)
# 	$(CXX) $(CFLAGS) -o $(BIN_DIR)/$@ $^ $(LFLAGS)

# test-patch-management: $(PATCH_MANAGEMENT_OBJS) $(COMMON_OBJS) $(TEST_OBJS)
# 	$(CXX) $(CFLAGS) -o $(BIN_DIR)/$@ $^ $(LFLAGS)

# test-device-monitor: $(DEVICE_MONITOR_OBJS) $(COMMON_OBJS) $(TEST_OBJS)
# 	$(CXX) $(CFLAGS) -o $(BIN_DIR)/$@ $^ $(LFLAGS)

# test-rootcheck: $(ROOTCHECK_OBJS) $(COMMON_OBJS) $(TEST_OBJS)
# 	$(CXX) $(CFLAGS) -o $(BIN_DIR)/$@ $^ $(LFLAGS)

# # Object file rules
# $(OBJ_DIR)/%.o: $(LOG_ANALYSIS)/%.cpp
# 	$(CXX) $(CFLAGS) -c -o $@ $<

# $(OBJ_DIR)/%.o: $(LOG_COLLECTOR)/%.cpp
# 	$(CXX) $(CFLAGS) -c -o $@ $<

# $(OBJ_DIR)/%.o: $(PATCH_MANAGEMENT)/%.cpp
# 	$(CXX) $(CFLAGS) -c -o $@ $<

# $(OBJ_DIR)/%.o: $(DEVICE_MONITOR)/%.cpp
# 	$(CXX) $(CFLAGS) -c -o $@ $<

# $(OBJ_DIR)/%.o: $(ROOTCHECK)/%.cpp
# 	$(CXX) $(CFLAGS) -c -o $@ $<

# $(OBJ_DIR)/%.o: $(COMMON)/%.cpp
# 	$(CXX) $(CFLAGS) -c -o $@ $<

# $(OBJ_DIR)/%.o: $(TEST_DIR)/%.cpp
# 	$(CXX) $(CFLAGS) -c -o $@ $<

# # Clean
# clean:
# 	rm -rf $(OBJ_DIR) $(BIN_DIR)




