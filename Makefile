CC=gcc
FLAGS=-O3

SERVER_LIB_DIR=server-lib
SRC_DIR=implementation
OBJ_DIR=obj
TARGET_DIR=target

SERVER_LIB_HEADER_FILES = $(wildcard $(SERVER_LIB_DIR)/*.h)
SERVER_LIB_SOURCE_FILES := $(wildcard $(SERVER_LIB_DIR)/*.c)
SERVER_LIB_OBJ := $(patsubst $(SERVER_LIB_DIR)/%.c, $(OBJ_DIR)/%-server-lib.o, $(SERVER_LIB_SOURCE_FILES))
SERVER_FILES := $(wildcard $(SRC_DIR)/*.c)

.PHONY: clean


implementation: $(SERVER_LIB_OBJ) $(SERVER_FILES) 	
	$(CC) $^ -o $(TARGET_DIR)/$@ $(FLAGS)


$(OBJ_DIR)/%-server-lib.o: $(SERVER_LIB_DIR)/%.c $(SERVER_LIB_HEADER_FILES)
	$(CC) -c -o $@ $< $(FLAGS)


clean:
	rm obj/* -f
	rm target/* -f
