#include <stdio.h>
#include "m4.h"

int main(void)
{
    tokenizer_init();
    
    const char *sql1 = "CREATE TABLE users (id INT, name CHAR, age INT)";
    ast_node *ast1 = parse(sql1);
    if (ast1 && semantic_check(ast1) == 0)
        printf("OK: CREATE TABLE %s\n", ast1->table);
    ast_free(ast1);
    
    const char *sql2 = "INSERT INTO users VALUES (1, 'joao', 25)";
    ast_node *ast2 = parse(sql2);
    if (ast2 && semantic_check(ast2) == 0)
        printf("OK: INSERT INTO %s, %d values\n", ast2->table, ast2->n_values);
    ast_free(ast2);
    
    const char *sql3 = "SELECT * FROM users WHERE id = 1";
    ast_node *ast3 = parse(sql3);
    if (ast3 && semantic_check(ast3) == 0)
        printf("OK: SELECT FROM %s WHERE %s=%s\n", 
               ast3->table, ast3->where_col, ast3->where_val);
    ast_free(ast3);
    
    const char *sql4 = "SELECT id, name FROM users";
    ast_node *ast4 = parse(sql4);
    if (ast4 && semantic_check(ast4) == 0)
        printf("OK: SELECT %d cols FROM %s\n", ast4->n_cols_sel, ast4->table);
    ast_free(ast4);
    
    return 0;
}