// m4 aka 'parser + catalog'
// tokenizer -> parser -> semantic analysis (catalog)
#ifndef M4_H
#define M4_H
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

#define TK_MAX 256 // max quantity of tokens
typedef struct token 
{
    tk_type type;
    char text[TK_MAX]; 
} token;

void tokenizer_init(void); 
token next_token(const char **src);
const char *tk_name(tk_type t);

#endif 