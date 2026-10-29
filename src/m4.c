#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "hashtab.h"
#include "m4.h"

const char *tk_name(tk_type t)
{
    static const char *n[] = { 
        "WORD", "VAR", "NUMBER", 
        "STRING", "SYMBOL", "END", 
        "ERROR" };
    return n[t];
}

void tokenizer_init(void)
{
    const char *words[] = { 
        "CREATE", "TABLE", "INSERT", "INTO", "VALUES",
        "SELECT", "FROM", "WHERE", "INT", "CHAR" 
    };
    
    for (size_t i = 0; i < sizeof words / sizeof *words; i++)
        install(words[i], "WORD");
}

token next_token(const char **src)
{
    token t = { TK_END, "" };
    const char *p = *src;

    while (isspace((unsigned char)*p)) p++;

    if (*p == '\0') { *src = p; return t; }

    if (strchr(";,()*=", *p)) 
    {
        t.type = TK_SYM;
        t.text[0] = *p++;
        t.text[1] = '\0';
    }
    else if (isalnum((unsigned char)*p) || *p == '_') 
    {
        const char *start = p;
        int n = 0, only_digits = 1;
        while (isalnum((unsigned char)*p) || *p == '_') {
            if (!isdigit((unsigned char)*p)) only_digits = 0;
            p++;
            n++;
        }
        if (n >= TK_MAX) 
        {
            t.type = TK_ERROR;
            strcpy(t.text, "token too long");
        } else {
            for (int i = 0; i < n; i++)
                t.text[i] = (char)toupper((unsigned char)start[i]);
            t.text[n] = '\0';

            if (only_digits)
                t.type = TK_NUM;
            else if (lookup(t.text))
                t.type = TK_WORD;
            else {
                t.type = TK_VAR;
                memcpy(t.text, start, (size_t)n);
            }
        }
    }
    else if (*p == '\'')
    {
        const char *start = ++p;
        while (*p && *p != '\'')
            p++;
        int n = (int)(p - start);

        if (*p != '\'') 
        {
            t.type = TK_ERROR;
            strcpy(t.text, "unterminated string");
        } 
        else if (n >= TK_MAX) 
        {
            p++;
            t.type = TK_ERROR;
            strcpy(t.text, "string too long");
        } 
        else 
        {
            memcpy(t.text, start, (size_t)n);
            t.text[n] = '\0';
            t.type = TK_STR;
            p++;
        }
    }
    else 
    {
        t.type = TK_ERROR;
        t.text[0] = *p++;
        t.text[1] = '\0';
    }

    *src = p;
    return t;
}

/*
 * PARSER
 */

static token cur;
static const char *pos;

static void advance(void)
{
    cur = next_token(&pos);
}

static int expect(tk_type t)
{
    if (cur.type != t) {
        fprintf(stderr, "parse error: expected %s, got %s (%s)\n",
                tk_name(t), tk_name(cur.type), cur.text);
        return -1;
    }
    advance();
    return 0;
}

static int expect_sym(char c)
{
    if (cur.type != TK_SYM || cur.text[0] != c) {
        fprintf(stderr, "parse error: expected '%c', got %s (%s)\n",
                c, tk_name(cur.type), cur.text);
        return -1;
    }
    advance();
    return 0;
}

static int expect_word(const char *w)
{
    if (cur.type != TK_WORD || strcmp(cur.text, w) != 0) {
        fprintf(stderr, "parse error: expected %s, got %s (%s)\n",
                w, tk_name(cur.type), cur.text);
        return -1;
    }
    advance();
    return 0;
}

static ast_node *parse_create(void)
{
    ast_node *n = calloc(1, sizeof *n);
    n->type = AST_CREATE;
    
    advance();
    if (expect_word("TABLE") < 0) goto fail;
    if (cur.type != TK_VAR) goto fail;
    strcpy(n->table, cur.text);
    advance();
    if (expect_sym('(') < 0) goto fail;
    
    column_def **tail = &n->columns;
    while (cur.type == TK_VAR) {
        column_def *col = calloc(1, sizeof *col);
        strcpy(col->name, cur.text);
        advance();
        
        if (cur.type != TK_WORD || (strcmp(cur.text, "INT") && strcmp(cur.text, "CHAR"))) {
            fprintf(stderr, "parse error: type must be INT or CHAR\n");
            free(col);
            goto fail;
        }
        strcpy(col->type, cur.text);
        advance();
        
        *tail = col;
        tail = &col->next;
        
        if (cur.type == TK_SYM && cur.text[0] == ',') {
            advance();
        } else break;
    }
    
    if (expect_sym(')') < 0) goto fail;
    return n;
    
fail:
    ast_free(n);
    return NULL;
}

static ast_node *parse_insert(void)
{
    ast_node *n = calloc(1, sizeof *n);
    n->type = AST_INSERT;
    
    advance();
    if (expect_word("INTO") < 0) goto fail;
    if (cur.type != TK_VAR) goto fail;
    strcpy(n->table, cur.text);
    advance();
    if (expect_word("VALUES") < 0) goto fail;
    if (expect_sym('(') < 0) goto fail;
    
    int cap = 4;
    n->values = malloc(cap * sizeof(char *));
    while (cur.type == TK_NUM || cur.type == TK_STR) {
        if (n->n_values >= cap) {
            cap *= 2;
            n->values = realloc(n->values, cap * sizeof(char *));
        }
        n->values[n->n_values++] = strdup(cur.text);
        advance();
        if (cur.type == TK_SYM && cur.text[0] == ',') {
            advance();
        } else break;
    }
    
    if (expect_sym(')') < 0) goto fail;
    return n;
    
fail:
    ast_free(n);
    return NULL;
}

static ast_node *parse_select(void)
{
    ast_node *n = calloc(1, sizeof *n);
    n->type = AST_SELECT;
    
    advance();
    
    if (cur.type == TK_SYM && cur.text[0] == '*') {
        advance();
    } else {
        int cap = 4;
        n->columns_sel = malloc(cap * sizeof(char *));
        while (cur.type == TK_VAR) {
            if (n->n_cols_sel >= cap) {
                cap *= 2;
                n->columns_sel = realloc(n->columns_sel, cap * sizeof(char *));
            }
            n->columns_sel[n->n_cols_sel++] = strdup(cur.text);
            advance();
            if (cur.type == TK_SYM && cur.text[0] == ',') advance();
            else break;
        }
    }
    
    if (expect_word("FROM") < 0) goto fail;
    if (cur.type != TK_VAR) goto fail;
    strcpy(n->table, cur.text);
    advance();
    
    if (cur.type == TK_WORD && strcmp(cur.text, "WHERE") == 0) {
        advance();
        if (cur.type != TK_VAR) goto fail;
        strcpy(n->where_col, cur.text);
        advance();
        if (expect_sym('=') < 0) goto fail;
        if (cur.type != TK_NUM && cur.type != TK_STR) goto fail;
        strcpy(n->where_val, cur.text);
        n->has_where = 1;
        advance();
    }
    
    return n;
    
fail:
    ast_free(n);
    return NULL;
}

ast_node *parse(const char *sql)
{
    pos = sql;
    advance();
    
    if (cur.type == TK_WORD) {
        if (strcmp(cur.text, "CREATE") == 0) return parse_create();
        if (strcmp(cur.text, "INSERT") == 0) return parse_insert();
        if (strcmp(cur.text, "SELECT") == 0) return parse_select();
    }
    
    fprintf(stderr, "parse error: unknown statement\n");
    return NULL;
}

void ast_free(ast_node *n)
{
    if (!n) return;
    
    column_def *c = n->columns;
    while (c) {
        column_def *next = c->next;
        free(c);
        c = next;
    }
    
    for (int i = 0; i < n->n_values; i++)
        free(n->values[i]);
    free(n->values);
    
    for (int i = 0; i < n->n_cols_sel; i++)
        free(n->columns_sel[i]);
    free(n->columns_sel);
    
    free(n);
}

/*
 *SEMANTIC ANALYSIS (catalog)
 */

int semantic_check(ast_node *n)
{
    if (!n) return -1;
    
    if (n->type == AST_CREATE) {
        if (lookup(n->table)) {
            fprintf(stderr, "semantic error: table %s already exists\n", n->table);
            return -1;
        }
        install(n->table, "TABLE");
        return 0;
    }
    
    if (n->type == AST_INSERT || n->type == AST_SELECT) {
        if (!lookup(n->table)) {
            fprintf(stderr, "semantic error: table %s does not exist\n", n->table);
            return -1;
        }
        return 0;
    }
    
    return -1;
}