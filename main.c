#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>
#include <ctype.h>
#include <string.h>
#include <inttypes.h>
#include <math.h>
#include <assert.h>
#include <stdbool.h>
#include <stdarg.h>
#include <limits.h>

#include "common.c"
#include "lex.c"
#include "ast.c"
#include "parse.c"


void run_tests(void){
     buff_test();
     str_intern_test();
     lex_test();
     ast_test();
     parse_test();
       arena_test();
}

int main(int argc, char **argv){
     run_tests();
     return 0;
}
