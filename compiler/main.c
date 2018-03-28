//TODO(Florian): Handle error properly when EOF is there
//TODO(Florian): Handle eating whitespace
//TODO(Florian): Write more text cases

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int8_t s8;
typedef int16_t s16;
typedef int32_t s32;
typedef int64_t s64;

typedef struct BufHdr {
    size_t len;
    size_t cap;
    char buf[0];
} BufHdr;

#define buf__hdr(b) ( (BufHdr *)(((char*)(b)) - offsetof(BufHdr, buf)) )
#define buf__fits(b, n) ( buf_len((b)) + (n) <= buf_cap((b)) )
#define buf__fit(b, n) ( buf__fits((b), (n)) ? 0 : ((b) = buf__grow((b), buf_len((b)) + (n), sizeof(*(b)))) )

#define buf_len(b) ( (b) ? buf__hdr((b))->len : 0 )
#define buf_cap(b) ( (b) ? buf__hdr((b))->cap : 0 )
#define buf_reserve(b, c) ( buf_cap(b) <= (c) ? ((b) = buf__reserve((b), (c), sizeof(*(b)))) : 0 )
#define buf_free(b) ( free(buf__hdr(b)), (b) = NULL )
#define buf_push(b, v) ( buf__fit((b), 1), (b)[buf__hdr(b)->len++] = (v) )

void *buf__reserve(void *buf, size_t new_cap, size_t elem_size) {
    assert(buf_cap(buf) <= new_cap);
    size_t new_size = offsetof(BufHdr, buf) + new_cap * elem_size;
    BufHdr* hdr = buf__hdr(buf);
    if (buf) {
        hdr = realloc(hdr, new_size);
    } else {
        hdr = malloc(new_size);
        hdr->len = 0;
    }
    hdr->cap = new_cap;
    return hdr->buf;
}

void *buf__grow(void *buf, size_t new_len, size_t elem_size) {
    size_t new_cap = (buf_cap(buf) * 2) + 1;
    assert(new_len <= new_cap);
    return buf__reserve(buf, new_cap, elem_size);
}

void buf_test() {
    int* int_buf = NULL;
    assert(buf_len(int_buf) == 0);
    assert(buf_cap(int_buf) == 0);
    
    enum { N = 1024 }; 
    for (size_t i = 0; i < N; i++) {
        buf_push(int_buf, i);
    }
    for (size_t i = 0; i < buf_len(int_buf); i++) {
        printf("%d\n", int_buf[i]);
    }
    buf_free(int_buf);
    assert(buf_cap(int_buf) == 0);
    float* float_buf = NULL;
    buf_reserve(float_buf, 1024);
    size_t cap = buf_cap(float_buf);
    assert(cap == 1024);
    assert(buf_len(float_buf) == 0);
}

typedef enum TokenKind {
    
    TOKENKIND_EOF,
    
    TOKENKIND_FIRST_MUL,
    TOKENKIND_MUL = TOKENKIND_FIRST_MUL,
    TOKENKIND_DIV,
    TOKENKIND_MOD,
    TOKENKIND_SHFTL,
    TOKENKIND_SHFTR,
    TOKENKIND_AND,
    TOKENKIND_LAST_MUL = TOKENKIND_AND,
    
    TOKENKIND_FIRST_ADD,
    TOKENKIND_ADD = TOKENKIND_FIRST_ADD,
    TOKENKIND_OR,
    TOKENKIND_XOR,
    TOKENKIND_MIN,
    TOKENKIND_LAST_ADD = TOKENKIND_MIN,
    
    TOKENKIND_COMPL,
    TOKENKIND_VAL,
} TokenKind;

typedef struct Token {
    TokenKind kind;
    union {
        s64 val;
        char op;
    };
} Token;

char *stream;
Token token;

bool next_token() {
    char c;
    start:
    switch ((c = *stream++)) {
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
        {
            s64 val = c - '0';
            c = *stream;
            while (c >= '0' && c <= '9') {
                stream++;
                val *= 10;
                val += c - '0';
                c = *stream;
            }
            token.kind = TOKENKIND_VAL;
            token.val = val;
            break;
        }
        
        case '+':
        {
            token.kind = TOKENKIND_ADD;
            break;
        }
        
        case '-':
        {
            token.kind = TOKENKIND_MIN;
            break;
        }
        
        case '~': 
        {	
            token.kind = TOKENKIND_COMPL;
            break;
        }
        
        case '*':
        {
            token.kind = TOKENKIND_MUL;
            break;
        }
        
        case '/':
        {
            token.kind = TOKENKIND_DIV;
            break;
        }
        
        case '<':
        {
            if (*stream++ == '<') {
                token.kind = TOKENKIND_SHFTL;
            } else {
                printf(" Invalid token\n");
            }
            break;
        }
        
        case '>':
        {
            if (*stream++ == '>') {
                token.kind = TOKENKIND_SHFTR;
            } else {
                printf(" Invalid token\n");
            }
            break;
        }
        
        case '&':
        {
            token.kind = TOKENKIND_AND;
            break;
        }
        
        case '|':
        {
            token.kind = TOKENKIND_OR;
            break;
        }
        
        case '^':
        {
            token.kind = TOKENKIND_XOR;
            break;
        }
        
        case '\0':
        {
            token.kind = TOKENKIND_EOF;
            break;
        }
        case ' ': case '\t': case '\n':
        {
            goto start;
            break;
        }
        default:
        {
            //TODO: handle error when unrecognized character
            printf("unrecognized character");
            return false;
        }
    }
    return true;
}

bool is_token(TokenKind kind) {
    return token.kind == kind;
}

bool match_token(TokenKind kind) {
    if (is_token(kind)) {
        next_token();
        return true;
    } else {
        return false;
    }
}

bool expect_token(TokenKind kind) {
    if (is_token(kind)) {
        next_token();
        return true;
    } else {
        exit(1);
        return false;
    }
}

bool is_token_add(TokenKind kind) {
    return kind >= TOKENKIND_FIRST_ADD && kind <= TOKENKIND_LAST_ADD;
}

bool is_token_mul(TokenKind kind) {
    return kind >= TOKENKIND_FIRST_MUL && kind <= TOKENKIND_LAST_MUL;
}

#if 0
expr3 = VAL
expr2 = [-~] expr3
expr1 = expr2 ([*/%&] expr2)*
expr0 = expr1 ([+-|^] expr1)*
expr = expr0
#endif

void lex_test_1() {
    char *expr = "1234<<>>+-~|&/%";
    stream = expr;
    next_token();
    assert(token.val == 1234);
    next_token();
    assert(token.kind == TOKENKIND_SHFTL);
    next_token();
    assert(token.kind == TOKENKIND_SHFTR);
    next_token();
    assert(token.kind == TOKENKIND_ADD);
    next_token();
    assert(token.kind == TOKENKIND_MIN);
}

void lex_test_2() {
    char *expr = "12+54*1234-2";
    stream = expr;
    next_token();
    assert(token.kind == TOKENKIND_VAL);
    assert(token.val == 12);
    next_token();
    assert(token.kind == TOKENKIND_ADD);
    next_token();
    assert(token.kind == TOKENKIND_VAL);
    assert(token.val == 54);
    next_token();
    assert(token.kind == TOKENKIND_MUL);
    next_token();
    assert(token.kind == TOKENKIND_VAL);
    assert(token.val == 1234);
    next_token();
    assert(token.kind == TOKENKIND_MIN);
    next_token();
    assert(token.kind == TOKENKIND_VAL);
    assert(token.val == 2);
}

void lex_test() {
    lex_test_1();
    lex_test_2();
}

void stream_init(const char* expr) {
    stream = expr;
    next_token();
}

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
    OPCODE_MIN,
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
    [OPCODE_MIN] = "-",
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
            op = OPCODE_MIN;
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
                        
                        case OPCODE_MIN:
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

void generate_bytecode(Expr* e) {
    //TODO: Generate bytecode for Per's small virtual machine
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

void inter_expr_test() {
    stream_init("28*8+19/7--15");
    s64 result = interpret_expr(parse_expr());
    assert(result == 241);
    stream_init("1 << 5 >> 2");
    result = interpret_expr(parse_expr());
    assert(result == 8);
}

void do_test() {
    //buf_test();
    lex_test();
    expr_test();
    inter_expr_test();
}

int main(int argc, char** argv) {
    do_test();
    return 0;
}
