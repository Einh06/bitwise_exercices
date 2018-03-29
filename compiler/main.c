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
#include "vm.h"

#include "buf.c"
#include "lex.c"
#include "vm.c"


#if 0
expr3 = VAL
expr2 = [-~] expr3
expr1 = expr2 ([*/%&] expr2)*
expr0 = expr1 ([+-|^] expr1)*
expr = expr0
#endif

void* ast_alloc(size_t size) {
    assert(size != 0);
    void* mem = malloc(size);
    memset(mem, 0, size);
    return mem;
}

//Changed with better handled struct
typedef enum ExprType {
    EXPRTYPE_OP,
    EXPRTYPE_VAL,
} ExprType;

typedef enum OpExprType {
    OPTYPE_BINARY,
    OPTYPE_UNARY,
} OpExprType;

typedef enum OpCode {
    OPCODE_ADD,
    OPCODE_SUB,
    OPCODE_OR,
    OPCODE_XOR,
    OPCODE_MUL,
    OPCODE_DIV,
    OPCODE_MOD,
    OPCODE_SHFTL,
    OPCODE_SHFTR,
    OPCODE_AND,
    OPCODE_COMPL,
    OPCODE_NEG,
    OPCODE_COUNT,
} OpCode;

static char *OpStrings[] = {
    [OPCODE_ADD] = "+",
    [OPCODE_SUB] = "-",
    [OPCODE_OR] = "|",
    [OPCODE_XOR] = "^",
    [OPCODE_MUL] = "*",
    [OPCODE_DIV] = "/",
    [OPCODE_MOD] = "%",
    [OPCODE_SHFTL] = "<<",
    [OPCODE_SHFTR] = ">>",
    [OPCODE_AND] = "&",
    [OPCODE_COMPL] = "~",
    [OPCODE_NEG] = "-",
};



typedef struct Expr Expr;
struct Expr {
    ExprType type;
    union {
        struct {
            OpExprType type;
            OpCode op_code;
            union {
                struct {
                    Expr* left;
                    Expr* right;
                };
                struct {
                    Expr* expr;
                };
            };
        } op;
        s64 val;
    };
};

Expr* expr_binary_op(OpCode op, Expr* left, Expr* right) {
    Expr* result = ast_alloc(sizeof(Expr));
    result->type = EXPRTYPE_OP;
    result->op.type = OPTYPE_BINARY;
    result->op.op_code = op;
    result->op.left = left;
    result->op.right = right;
    return result;
}

Expr* expr_unary_op(OpCode op, Expr* expr) {
    Expr* result = ast_alloc(sizeof(Expr));
    result->type = EXPRTYPE_OP;
    result->op.type = OPTYPE_UNARY;
    result->op.op_code = op;
    result->op.expr = expr;
    return result;
}

Expr* expr_val(s64 val) {
    Expr* result = ast_alloc(sizeof(Expr));
    result->type = EXPRTYPE_VAL;
    result->val = val;
    return result;
}

void print_expr(Expr* e) {
    switch (e->type) {
        case EXPRTYPE_VAL:
        {
            printf("%llu", e->val);
            break;
        }
        case EXPRTYPE_OP:
        {
            printf("(");
            printf("%s ", OpStrings[e->op.op_code]);
            switch (e->op.type) {
                case OPTYPE_BINARY:
                {
                    print_expr(e->op.left);
                    printf(" ");
                    print_expr(e->op.right);
                    break;
                }
                case OPTYPE_UNARY:
                {
                    print_expr(e->op.expr);
                    break;
                }
            }
            printf(")");
            break;
        }
    }
}

Expr* parse_term() {
    assert(is_token(TOKENKIND_VAL));
    Expr* e = expr_val(token.val);
    next_token();
    return e;
}

Expr* parse_unary() {
    if (is_token(TOKENKIND_MIN) || is_token(TOKENKIND_COMPL)) {
        OpCode op;
        if (match_token(TOKENKIND_MIN)) {
            op = OPCODE_NEG;
        }
        if (match_token(TOKENKIND_COMPL)) {
            op = OPCODE_COMPL;
        }
        return expr_unary_op(op, parse_term());
    }
    return parse_term();
}

Expr* parse_mul() {
    Expr* e = parse_unary();
    while (is_token_mul(token.kind)) {
        OpCode op;
        if (match_token(TOKENKIND_MUL)) {
            op = OPCODE_MUL;
        }
        if (match_token(TOKENKIND_DIV)) {
            op = OPCODE_DIV;
        }
        if (match_token(TOKENKIND_MOD)) {
            op = OPCODE_MOD;
        }
        if (match_token(TOKENKIND_SHFTL)) {
            op = OPCODE_SHFTL;
        }
        
        if (match_token(TOKENKIND_SHFTR)) {
            op = OPCODE_SHFTR;
        }
        
        if (match_token(TOKENKIND_AND)) {
            op = OPCODE_AND;
        }
        
        e = expr_binary_op(op, e, parse_unary());
    }
    return e;
}

Expr* parse_add() {
    Expr* e = parse_mul();
    while (is_token_add(token.kind)) {
        OpCode op;
        if (match_token(TOKENKIND_ADD)) {
            op = OPCODE_ADD;
        }
        if (match_token(TOKENKIND_MIN)) {
            op = OPCODE_SUB;
        }
        if (match_token(TOKENKIND_OR)) {
            op = OPCODE_OR;
        }
        if (match_token(TOKENKIND_XOR)) {
            op = OPCODE_XOR;
        }
        
        e = expr_binary_op(op, e, parse_mul());
    }
    return e;
}

Expr* parse_expr() {
    return parse_add();
}

void expr_test() {
    printf("\n");
    stream_init("28*8 +19/7- -15");
    print_expr(parse_expr());
    printf("\n");
    stream_init("1<<5>>2");
    print_expr(parse_expr());
    printf("\n");
}

s64 interpret_expr(Expr* e) {
    s64 result = 0;
    switch (e->type) {
        case EXPRTYPE_VAL:
        {
            result = e->val;
            break;
        }
        case EXPRTYPE_OP:
        {
            switch (e->op.type) {
                case OPTYPE_BINARY:
                {
                    switch (e->op.op_code) {
                        case OPCODE_ADD:
                        result = interpret_expr(e->op.left) + interpret_expr(e->op.right);
                        break;
                        
                        case OPCODE_SUB:
                        result = interpret_expr(e->op.left) - interpret_expr(e->op.right);
                        break;
                        
                        case OPCODE_OR:
                        result = interpret_expr(e->op.left) | interpret_expr(e->op.right);
                        break;
                        
                        case OPCODE_XOR:
                        result = interpret_expr(e->op.left) ^ interpret_expr(e->op.right);
                        break;
                        
                        case OPCODE_MUL:
                        result = interpret_expr(e->op.left) * interpret_expr(e->op.right);
                        break;
                        
                        case OPCODE_DIV:
                        result = interpret_expr(e->op.left) / interpret_expr(e->op.right);
                        break;
                        
                        case OPCODE_MOD:
                        result = interpret_expr(e->op.left) % interpret_expr(e->op.right);
                        break;
                        
                        case OPCODE_AND:
                        result = interpret_expr(e->op.left) & interpret_expr(e->op.right);
                        break;
                        
                        case OPCODE_SHFTL:
                        result = interpret_expr(e->op.left) << interpret_expr(e->op.right);
                        break;
                        
                        case OPCODE_SHFTR:
                        result = interpret_expr(e->op.left) >> interpret_expr(e->op.right);
                        break;
                        
                        default:
                        exit(1);
                    }
                }
                case OPTYPE_UNARY:
                {
                    if (e->op.op_code == OPCODE_NEG) {
                        result = -interpret_expr(e->op.expr);
                    } else if (e->op.op_code == OPCODE_COMPL) {
                        result = ~interpret_expr(e->op.expr);
                    }
                    break;
                }
            }
            break;
        }
    }
    return result;
}

void inter_expr_test() {
    stream_init("28*8+19/7--15");
    s64 result = interpret_expr(parse_expr());
    assert(result == 241);
    stream_init("1 << 5 >> 2");
    result = interpret_expr(parse_expr());
    assert(result == 8);
}

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
        compile_expr("28*8+19/7--15", &code);
        s64 result = vm_exec(code);
        assert(result == 241);
        
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
