#include <ctype.h>
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
    { // symbols: one char each 
        t.type = TK_SYM;
        t.text[0] = *p++;
        t.text[1] = '\0';
    }
    else if (isalnum((unsigned char)*p) || *p == '_') 
    { // word, name or number 
        const char *start = p;
        int n = 0, only_digits = 1;
        while (isalnum((unsigned char)*p) || *p == '_') {
            if (!isdigit((unsigned char)*p)) only_digits = 0;
            p++;
            n++;
        }
        if (n >= TK_MAX) 
        { // too long: error (never cut silently)
            t.type = TK_ERROR;
            strcpy(t.text, "token too long");
        } else {
            for (int i = 0; i < n; i++) // upper-case copy, used for the lookup 
                t.text[i] = (char)toupper((unsigned char)start[i]);
            t.text[n] = '\0';

            if (only_digits)
                t.type = TK_NUM;
            else if (lookup(t.text))
                t.type = TK_WORD;
            else {
                t.type = TK_VAR;
                memcpy(t.text, start, (size_t)n); // names keep their case
            }
        }
    }
    else if (*p == '\'')
    { // text 
        const char *start = ++p; // skip the opening quote
        while (*p && *p != '\'')
            p++;
        int n = (int)(p - start);

        if (*p != '\'') 
        { // reached the end with no closing quote 
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
            memcpy(t.text, start, (size_t)n); // the quotes are not stored
            t.text[n] = '\0';
            t.type = TK_STR;
            p++; // skip the closing quote 
        }
    }
    else 
    { // anything else 
        t.type = TK_ERROR;
        t.text[0] = *p++;
        t.text[1] = '\0';
    }

    *src = p;
    return t;
}