# Project Data
EXEC_NAME = Main
SRC_DIR = src
BUILD_DIR = build
INCLUDES_DIR = $(SRC_DIR)/includes

# Flags, compiler stuff
# Necessary project flags
CFLAGS = -MMD -MP -I$(INCLUDES_DIR)
CFLAGS += -std=c++20

# Windows no cmd line
CFLAGS += -mwindows

# Extra warnings
CFLAGS += -Wall -Wextra -pedantic
# CFLAGS += -Werror

LIBS =

# Obtain all source, object, and dependency files
SRC = $(wildcard $(SRC_DIR)/*.cpp)
OBJS = $(SRC:$(SRC_DIR)/%.cpp=$(BUILD_DIR)/%.o)
DEPS = $(SRC:$(SRC_DIR)/%.cpp=$(BUILD_DIR)/%.d)

# Link all object files into one executable
$(EXEC_NAME): $(OBJS)
	g++ -o $@ $(OBJS) $(CFLAGS) $(LIBS)

# Line by line compile all srcs into objs
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(BUILD_DIR)
	g++ -c $< -o $@ $(CFLAGS) $(LIBS)

# Include dependency files generated from -MMD -MP flags
-include $(DEPS)

clean:
	rm -rf $(BUILD_DIR) $(EXEC_NAME)
	@mkdir $(BUILD_DIR)
