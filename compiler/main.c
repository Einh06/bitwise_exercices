//TODO(Florian): Improve error handling
//TODO(Florian): Create more facilities for unhandled case 'fatal etc'
//TODO(Florian): go through exercices and 
//TODO(Florian): Write more text cases

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>

#include "common.h"
#include "lex.h"
#include "ast.h"
#include "vm.h"

#include "lex.c"
#include "ast.c"
#include "parse.c"
#include "interp.c"
#include "vm.c"
#include "gen.c"

void main_test() {
    {
        u8* code = NULL;
        compile_expr("28*(8+19)/7--15", &code);
        s64 result = vm_exec(code);
        assert(result == 123);
        
        buf_free(code);
    }
    {
        u8* code = NULL;
        compile_expr("1 << 5 >> 2", &code);
        s64 result = vm_exec(code);
        assert(result == 8);
        
        buf_free(code);
    }
}

void do_test() {
    //buf_test();
    lex_test();
    expr_test();
    inter_expr_test();
    bytecode_generation_test();
    vm_test();
    main_test();
}

int main(int argc, char** argv) {
    do_test();
    return 0;
}
