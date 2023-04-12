BIN_NAME=cubegame

OBJ_DIR=obj

C_SOURCE_FILES=$(wildcard src/*.c)
CXX_SOURCE_FILES=$(wildcard src/*.cpp)

OBJECT_FILES=$(patsubst src/%.c, $(OBJ_DIR)/%.o, $(C_SOURCE_FILES))

INCLUDE_FILES=$(wildcard include/*.h)

INCLUDE_PATHS=-I. -Iinclude

CFLAGS=-g -std=c11 -fdiagnostics-color=always

LDFLAGS=-lGL -lglfw -lGLEW -lcglm

WARN_FLAGS=-Wall -Wextra -Wpedantic -Werror -Wconversion

.PHONY: clean

all: $(BIN_NAME)

$(OBJ_DIR)/%.o: src/%.c $(INCLUDE_FILES)
	@mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS) $(WARN_FLAGS) $(INCLUDE_PATHS) -c -o $@ $<

$(BIN_NAME): $(OBJECT_FILES) $(INCLUDE_FILES)
	$(CXX) $(CXXFLAGS) $(WARN_FLAGS) $(INCLUDE_PATHS) -o $@ $(OBJECT_FILES) $(LDFLAGS)

clean:
	rm -f ./$(BIN_NAME)
	rm -rf ./$(OBJ_DIR)
