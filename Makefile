CC=clang++
CFLAGS=-Wall -Wextra -std=c++11 -pedantic
LDFLAGS=-lopencv_core -lopencv_imgproc -lopencv_features2d -lopencv_highgui

BIN=exe
SRC=main.cc \

ARGS=

BIN_FOLDER=bin
SRC_FOLDER=src
OBJ_FOLDER=obj

INCS=-I$(SRC_FOLDER)

BIN_=$(BIN_FOLDER)/$(BIN)
OBJS=$(foreach obj, $(SRC:.cc=.o), $(OBJ_FOLDER)/$(obj))

# Colors
DEF=\\033[0m
RED=\\033[31m
GRE=\\033[32m
BLU=\\033[34m
MAG=\\033[35m

.PHONY: all
all: $(BIN_)

.PHONY: tidy
tidy: $(BIN_) softclean

.PHONY: run
run: $(BIN_)
	@echo -e "$(BLU)[ EXE ] $(DEF) $<"
	@$(EXEC) ./$(BIN_) $(ARGS)

.PHONY: leak
leak: CFLAGS += -g -ggdb3
leak: EXEC += valgrind --leak-check=full
leak: clean run

$(BIN_): $(OBJS)
	@mkdir -p $(BIN_FOLDER)
	@$(CC) $(LDFLAGS) $(OBJS) -o $(BIN_)
	@echo -e "$(MAG)[ LD  ] $(DEF) $(BIN_)"

$(OBJ_FOLDER)/%.o: $(SRC_FOLDER)/%.cc
	@mkdir -p `dirname $@`
	@$(CC) $(CFLAGS) $(INCS) -c $< -o $@
	@echo -e "$(GRE)[ CXX ] $(DEF) $@"

.PHONY: clean
clean: softclean
	@rm -rf $(BIN_FOLDER)
	@echo -e "$(RED)[ RM  ] $(DEF) $(BIN_FOLDER)"

.PHONY: softclean
softclean:
	@rm -rf $(OBJ_FOLDER)
	@echo -e "$(RED)[ RM  ] $(DEF) $(OBJ_FOLDER)"
