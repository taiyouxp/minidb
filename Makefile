NAME     = minidb
BIN_DIR  = bin
OBJ_DIR  = obj
SRC_DIR  = src
INC_DIR  = include

CC       = gcc
CFLAGS   = 
INCLUDES = -I$(INC_DIR)

SRC      = $(wildcard $(SRC_DIR)/*.c)
OBJ      = $(SRC:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)

all: $(BIN_DIR) $(OBJ_DIR) $(BIN_DIR)/$(NAME)

$(BIN_DIR):
	mkdir $(BIN_DIR)

$(OBJ_DIR):
	mkdir $(OBJ_DIR)

$(BIN_DIR)/$(NAME): $(OBJ)
	$(CC) $(CFLAGS) $(OBJ) -o $@

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

clean:
	rm -f $(OBJ)

fclean: clean
	rm -rf $(BIN_DIR) $(OBJ_DIR)

re: fclean all