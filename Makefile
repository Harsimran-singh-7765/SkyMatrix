# Makefile for Hierarchical Satellite Image Analytics Engine
# Design and Analysis of Algorithms Project
# 
# Compiler: g++ with C++17 standard
# Optimization: -O2 for performance, -g for debugging

# Compiler and flags
CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -Wpedantic -O2
DEBUGFLAGS = -std=c++17 -Wall -Wextra -Wpedantic -g -O0 -DDEBUG

# Directories
SRC_DIR = src
INC_DIR = include
BUILD_DIR = build

# Target executable
TARGET = satellite_analytics
DEBUG_TARGET = satellite_analytics_debug

# Source files
SOURCES = $(wildcard $(SRC_DIR)/*.cpp)

# Object files
OBJECTS = $(patsubst $(SRC_DIR)/%.cpp,$(BUILD_DIR)/%.o,$(SOURCES))
DEBUG_OBJECTS = $(patsubst $(SRC_DIR)/%.cpp,$(BUILD_DIR)/debug_%.o,$(SOURCES))

# Header dependencies
HEADERS = $(wildcard $(INC_DIR)/*.h)

# Default target
all: $(TARGET)

# Debug build
debug: $(DEBUG_TARGET)

# Create build directory
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# Compile source files to object files
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp $(HEADERS) | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -I$(INC_DIR) -c $< -o $@

# Debug object files
$(BUILD_DIR)/debug_%.o: $(SRC_DIR)/%.cpp $(HEADERS) | $(BUILD_DIR)
	$(CXX) $(DEBUGFLAGS) -I$(INC_DIR) -c $< -o $@

# Link object files to create executable
$(TARGET): $(OBJECTS)
	$(CXX) $(CXXFLAGS) $^ -o $@
	@echo "Build complete: $(TARGET)"

# Debug executable
$(DEBUG_TARGET): $(DEBUG_OBJECTS)
	$(CXX) $(DEBUGFLAGS) $^ -o $@
	@echo "Debug build complete: $(DEBUG_TARGET)"

# Run the program
run: $(TARGET)
	./$(TARGET)

# Run with custom parameters
run-small: $(TARGET)
	./$(TARGET) --size 256 --topk 5

run-large: $(TARGET)
	./$(TARGET) --size 1024 --anomalies 15 --topk 20

run-quiet: $(TARGET)
	./$(TARGET) --no-visual --quiet

# Clean build artifacts
clean:
	rm -rf $(BUILD_DIR) $(TARGET) $(DEBUG_TARGET) *.pgm

# Clean everything including output files
distclean: clean
	rm -f output_*.pgm

# Show help
help:
	@echo "Hierarchical Satellite Image Analytics Engine"
	@echo ""
	@echo "Available targets:"
	@echo "  make          - Build optimized executable"
	@echo "  make debug    - Build debug executable"
	@echo "  make run      - Build and run with default settings"
	@echo "  make run-small - Run with smaller image (256x256)"
	@echo "  make run-large - Run with larger image (1024x1024)"
	@echo "  make run-quiet - Run without visualization"
	@echo "  make clean    - Remove build artifacts"
	@echo "  make help     - Show this help message"
	@echo ""
	@echo "Runtime options:"
	@echo "  ./$(TARGET) --size N        Image size NxN"
	@echo "  ./$(TARGET) --anomalies N   Number of anomalies"
	@echo "  ./$(TARGET) --topk N        Top-K parameter"
	@echo "  ./$(TARGET) --threshold T   Anomaly threshold"
	@echo "  ./$(TARGET) --help          Show all options"

# Phony targets
.PHONY: all debug run run-small run-large run-quiet clean distclean help
