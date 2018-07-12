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
void run_tests(void){
     buff_test();
     lex_test();
     str_intern_test();
}

int main(int argc, char **argv){
     run_tests();
     return 0;
}
