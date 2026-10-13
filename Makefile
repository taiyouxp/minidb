NAME     = minidb
BIN_DIR  = bin
OBJ_DIR  = obj
SRC_DIR  = src
INC_DIR  = include

CC       = gcc
CFLAGS   = -Wall -Wextra -g 
INCLUDES = -I$(INC_DIR)

SRC      = $(wildcard $(SRC_DIR)/*.c)
OBJ      = $(SRC:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)
DEP		 = $(OBJ:.o=.d)

all: $(BIN_DIR)/$(NAME)
# linking
$(BIN_DIR)/$(NAME): $(OBJ) | $(BIN_DIR)
	$(CC) $(CFLAGS) $(OBJ) -o $@
# compiling
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) $(INCLUDES) -MMD -MP -c $< -o $@

$(BIN_DIR) $(OBJ_DIR):
	mkdir -p $@

clean:
	rm -f $(OBJ) $(DEP)

fclean: clean
	rm -rf $(BIN_DIR) $(OBJ_DIR)

re: fclean
	$(MAKE) all

.PHONY: all clean fclean re

-include $(DEP)