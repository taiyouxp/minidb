// testing only tokenizer
#include <stdio.h>
#include "m4.h"

static void show(const char *sql)
{
    printf("\n> %s\n", sql);
    const char *p = sql;
    for (token t = next_token(&p); t.type != TK_END; t = next_token(&p))
        printf("    %-7s %s\n", tk_name(t.type), t.text);
}

int main(void)
{
    tokenizer_init();
    show("create TABLE alunos (id INT, nome CHAR(20));");
    show("INSERT INTO alunos VALUES (1, 'Taiyou')");
    show("SELECT * FROM alunos WHERE nome = 'Taiyou';");
    show("SELECT id, nome FROM alunos");
    show("INSERT INTO alunos VALUES (1, 'sem fim");
    show("SELECT # FROM alunos");
    return 0;
}