# Compiler and Flags
CXX = g++
CXXFLAGS = -std=c++17 -Iinclude -Wno-write-strings
LEX = flex
YACC = bison

# Folders
SRC_DIR = src
INC_DIR = include
BUILD_DIR = build

# Output Binary
TARGET = inductl

# Files
SRCS = $(SRC_DIR)/main.cpp
LEX_SRC = $(SRC_DIR)/lexer.l
YACC_SRC = $(SRC_DIR)/parser.y

# Default Rule
all: $(TARGET)

$(TARGET): $(LEX_SRC) $(YACC_SRC) $(SRCS)
	# 1. Generate Parser files from Bison
	$(YACC) -d $(YACC_SRC) -o $(SRC_DIR)/parser.cpp
	# 2. Generate Lexer files from Flex
	$(LEX) -o $(SRC_DIR)/lexer.cpp $(LEX_SRC)
	# 3. Move header to include folder for cleanliness
	mv $(SRC_DIR)/parser.hpp $(INC_DIR)/parser.hpp
	# 4. Compile everything into the final binary
	$(CXX) $(CXXFLAGS) $(SRCS) $(SRC_DIR)/lexer.cpp $(SRC_DIR)/parser.cpp -o $(TARGET)

# Clean rule
clean:
	rm -f $(TARGET) $(SRC_DIR)/lexer.cpp $(SRC_DIR)/parser.cpp $(INC_DIR)/parser.hpp