CC ?= gcc
CFLAGS := -Wall -g

BIN_PATH := bin
OBJ_PATH := obj
SRC_PATH := src

TARGET_NAME := koish
TARGET := $(BIN_PATH)/$(TARGET_NAME)

SRC := $(foreach DIR, $(SRC_PATH), $(wildcard $(DIR)/*.c))
OBJ := $(addprefix $(OBJ_PATH)/,$(notdir $(SRC:.c=.o)))

default: makedir all

$(OBJ_PATH)/%.o: $(SRC_PATH)/%.c*
	@$(CC) -c $(CFLAGS) -o $@ $<

$(TARGET): $(OBJ)
	@$(CC) -o $@ $(OBJ) $(CFLAGS)

.PHONY: makedir
makedir:
	@mkdir -p $(BIN_PATH) $(OBJ_PATH)

.PHONY: all
all: $(TARGET)

.PHONY: clean
clean:
	@rm -rf $(BIN_PATH) $(OBJ_PATH)
