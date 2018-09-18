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
#include "print.c"
#include "ast.c"
#include "parse.c"
#include "resolve.c"

void run_tests(void){
     buff_test();
     str_intern_test();
     lex_test();
     arena_test();
     ast_test();
     parse_test();
     type_test(); 
     resolve_test();
//     order_test();
     flush_print_buff(stdout);
}

int main(int argc, char **argv){
     run_tests();
     return 0;
}
