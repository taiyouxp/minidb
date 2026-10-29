// m4 aka 'parser + catalog'
// tokenizer -> parser -> semantic analysis (catalog)
#ifndef M4_H
#define M4_H

#include <stdint.h>

// 1. tokenizer section
typedef enum {
    TK_WORD,
    TK_VAR,
    TK_NUM,
    TK_STR,
    TK_SYM,
    TK_END,
    TK_ERROR 
} tk_type; 

#define TK_MAX 256

typedef struct token 
{
    tk_type type;
    char text[TK_MAX]; 
} token;

void tokenizer_init(void); 
token next_token(const char **src);
const char *tk_name(tk_type t);

// 2. AST section
typedef enum {
    AST_CREATE,
    AST_INSERT,
    AST_SELECT
} ast_type;

typedef struct column_def {
    char name[TK_MAX];
    char type[TK_MAX];
    struct column_def *next;
} column_def;

typedef struct ast_node {
    ast_type type;
    char table[TK_MAX];
    
    column_def *columns;
    
    char **values;
    int n_values;
    
    char **columns_sel;
    int n_cols_sel;
    char where_col[TK_MAX];
    char where_val[TK_MAX];
    int has_where;
} ast_node;

// 3. parser section
ast_node *parse(const char *sql);
void ast_free(ast_node *n);

// 4. semantic section
int semantic_check(ast_node *n);

#endif