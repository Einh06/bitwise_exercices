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

#include "buf.c"
#include "lex.c"
#include "ast.c"
#include "parse.c"
#include "interp.c"
#include "vm.c"

static VmOpCode OpCodeToVM[] = {
    [OPCODE_ADD] = ADD,
    [OPCODE_SUB] = SUB,
    [OPCODE_OR] = OR,
    [OPCODE_XOR] = XOR,
    [OPCODE_MUL] = MUL,
    [OPCODE_DIV] = DIV,
    [OPCODE_MOD] = MOD,
    [OPCODE_SHFTL] = SHFTL,
    [OPCODE_SHFTR] = SHFTR,
    [OPCODE_AND] = AND,
    [OPCODE_COMPL] = COMPL,
    [OPCODE_NEG] = NEG,
};

void generate_bytecode0(Expr* e, u8** code) {
    switch (e->type) {
        case EXPRTYPE_VAL:
        {
            buf_push(*code, LIT);
            u64 mask = 0xFF;
            for ( int i = 0; i < sizeof(e->val); i++) {
                buf_push(*code, e->val & mask);
                mask <<= 8;
            }
            
            break;
        }
        case EXPRTYPE_OP:
        {
            switch (e->op.type) {
                case OPTYPE_BINARY:
                {
                    generate_bytecode0(e->op.right, code);
                    generate_bytecode0(e->op.left, code);
                    buf_push(*code, OpCodeToVM[e->op.op_code]);
                    break;
                }
                case OPTYPE_UNARY:
                {
                    generate_bytecode0(e->op.expr, code);
                    buf_push(*code, (OpCodeToVM[e->op.op_code]));
                    break;
                }
            }
            break;
        }
    }
}

void generate_bytecode(Expr* e, u8** code) {
    generate_bytecode0(e, code);
    buf_push(*code, HALT);
}

void bytecode_generation_test() {
#define STREAM_EQUAL(b, r) do { for (int i = 0; i < sizeof((r)); i++) { assert( (b)[i] == (r)[i] ); } }while(0)
    
    Expr* e;
    u8* code = NULL;
    {
        e = expr_val(1);
        u8 expected_result[] = { LIT, 1, 0, 0, 0, 0, 0, 0, 0, HALT };
        generate_bytecode(e, &code);
        
        assert(buf_len(code) == sizeof(expected_result)); 
        STREAM_EQUAL(code, expected_result);
        
        buf_free(code);
    }
    
    {
        e = expr_binary_op(OPCODE_ADD, expr_val(1), expr_val(1));
        u8 expected_result[] = {LIT, 1, 0, 0, 0, 0, 0, 0, 0, LIT, 1, 0, 0, 0, 0, 0, 0, 0, ADD, HALT};
        generate_bytecode(e, &code);
        
        assert(buf_len(code) == sizeof(expected_result));
        STREAM_EQUAL(code, expected_result);
        buf_free(code);
    }
    
    {
        e = expr_unary_op(OPCODE_NEG, expr_val(1));
        u8 expected_result[] = { LIT, 1, 0, 0, 0, 0, 0, 0, 0, NEG, HALT };
        generate_bytecode(e, &code);
        
        assert(buf_len(code) == sizeof(expected_result));
        STREAM_EQUAL(code, expected_result);
        buf_free(code);
    }
    
    {
        e = expr_binary_op(OPCODE_MUL,expr_val(7),  expr_binary_op(OPCODE_ADD, expr_val(19), expr_unary_op(OPCODE_NEG, expr_val(15))));
        u8 expected_result[] = { 
            LIT, 0x0F, 0, 0, 0, 0, 0, 0, 0, 
            NEG, 
            LIT, 0x13, 0, 0, 0, 0, 0, 0, 0,
            ADD,
            LIT, 0x07, 0, 0 ,0, 0, 0, 0, 0,
            MUL,
            HALT };
        generate_bytecode(e, &code);
        
        assert(buf_len(code) == sizeof(expected_result));
        STREAM_EQUAL(code, expected_result);
        buf_free(code);
    }
    
#undef STREAM_EQUAL
}

void compile_expr(char *expr, u8** code) {
    stream_init(expr);
    Expr* e = parse_expr();
    generate_bytecode(e, code);
}


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
