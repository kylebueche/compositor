# Project Data
EXEC_NAME = Main
SRC_DIR = src
BUILD_DIR = build
INCLUDES_DIR = $(SRC_DIR)/includes
EXTERNAL_DIR = external

# Flags, compiler stuff
# Necessary project flags
CFLAGS = -MMD -MP -I$(INCLUDES_DIR) -I$(EXTERNAL_DIR)
CFLAGS += -std=c++20 -O3 -msse -msse2 -msse3 -mssse3 -msse4.1 -msse4.2 -mavx -mavx2 -march=native

d ?= 0
ifeq ($(d), 1)
	CFLAGS += -O0 -g
endif

# Windows no cmd line
# CFLAGS += -mwindows

# Extra warnings
CFLAGS += -Wall -Wextra -pedantic
# CFLAGS += -Werror

LIBS = -lglfw3 -lgdi32 -lopengl32 -limm32

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
