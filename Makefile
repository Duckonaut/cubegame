BIN_NAME=cubegame

OBJ_DIR=obj

C_SOURCE_FILES=$(wildcard src/*.c)
CXX_SOURCE_FILES=$(wildcard src/*.cpp)

OBJECT_FILES=$(patsubst src/%.c, $(OBJ_DIR)/%.o, $(C_SOURCE_FILES))

INCLUDE_FILES=$(wildcard include/*.h) $(wildcard libs/stb/*.h)

# Include libs with -isystem to surpress our overzealous warnings
INCLUDE_PATHS=-I. -Iinclude -isystem libs/stb

CFLAGS=-g -std=c11 -fdiagnostics-color=always

LDFLAGS=-lGL -lglfw -lGLEW -lcglm -lm

WARN_FLAGS=-Wall -Wextra -Wpedantic -Werror -Wconversion

.PHONY: clean

all: $(BIN_NAME)

$(OBJ_DIR)/%.o: src/%.c $(INCLUDE_FILES)
	@mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS) $(WARN_FLAGS) $(INCLUDE_PATHS) -c -o $@ $<

$(BIN_NAME): $(OBJECT_FILES) $(INCLUDE_FILES)
	$(CC) $(CXXFLAGS) $(WARN_FLAGS) $(INCLUDE_PATHS) -o $@ $(OBJECT_FILES) $(LDFLAGS)

clean:
	rm -f ./$(BIN_NAME)
	rm -rf ./$(OBJ_DIR)
