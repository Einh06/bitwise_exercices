//TODO(Florian): Handle error properly when EOF is there
//TODO(Florian): Handle eating whitespace
//TODO(Florian): refactor TokenKind to use directly char for OP, as shown by Per Vognsen for bitwise
//TODO(Florian): 

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
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
    
    TOKENKIND_MIN,
    TOKENKIND_COMPL,
    
    TOKENKIND_MUL,
    TOKENKIND_DIV,
    TOKENKIND_MOD,
    TOKENKIND_SHFTL,
    TOKENKIND_SHFTR,
    TOKENKIND_AND,
    
    TOKENKIND_ADD,
    TOKENKIND_OR,
    TOKENKIND_XOR,
    
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

#if 0
expr3 = VAL
expr2 = [-~] expr3
expr1 = expr2 ([*/%&] expr2)*
expr0 = expr1 ([+-|^] expr1)*
expr = expr0
#endif

// TODO(Florian): Implement grouping with '(' and ')'
s64 inter_parse_expr3() {
    if (!is_token(TOKENKIND_VAL)) {
        perror("Not expect tokenkind in depth3");
    }
    
    s64 val = token.val;
    next_token();
    return val;
}

s64 inter_parse_expr2() {
    if (match_token(TOKENKIND_MIN)) {
        return -inter_parse_expr3();
    }
    if (match_token(TOKENKIND_COMPL)) {
        return ~(inter_parse_expr3());
    }
    return inter_parse_expr3();
}

s64 inter_parse_expr1() {
    s64 val = inter_parse_expr2();
    while (is_token(TOKENKIND_MUL) || is_token(TOKENKIND_DIV) || is_token(TOKENKIND_MOD) || is_token(TOKENKIND_AND) || is_token(TOKENKIND_SHFTL) || is_token(TOKENKIND_SHFTR)) {
        
        if (match_token(TOKENKIND_MUL)){
            val *= inter_parse_expr2();
        }
        
        if(match_token(TOKENKIND_DIV)) {
            val /= inter_parse_expr2();
        }
        
        if(match_token(TOKENKIND_MOD)) {
            val %= inter_parse_expr2();
        }
        
        if(match_token(TOKENKIND_AND)) {
            val &= inter_parse_expr2();
        }
        
        if(match_token(TOKENKIND_SHFTL)) {
            val <<= inter_parse_expr2();
        }
        
        if(match_token(TOKENKIND_SHFTR)) {
            val >>= inter_parse_expr2();
        }
    }
    return val;
}

s64 inter_parse_expr0() {
    s64 val = inter_parse_expr1();
    while (is_token(TOKENKIND_ADD) || is_token(TOKENKIND_MIN) || is_token(TOKENKIND_OR) || is_token(TOKENKIND_XOR)) {
        
        if(match_token(TOKENKIND_ADD)) {
            val += inter_parse_expr1();
        }
        
        if(match_token(TOKENKIND_MIN)) {
            val -= inter_parse_expr1();
        }
        
        if(match_token(TOKENKIND_OR)) {
            val |= inter_parse_expr1();
        }
        
        if(match_token(TOKENKIND_XOR)) {
            val ^= inter_parse_expr1();
        }
    }
    return val;
}

s64 inter_parse_expr() {
    return inter_parse_expr0();
}

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

void inter_test() {
    stream_init("12");
    assert(inter_parse_expr() == 12);
    stream_init("12+13");
    assert(inter_parse_expr() == 25);
    stream_init("2*3+4");
    assert(inter_parse_expr() == 10);
    stream_init("2*2-4*4/8");
    assert(inter_parse_expr() == 2);
}

typedef enum NodeType {
    NODETYPE_OP,
    NODETYPE_VAL,
} NodeType;

typedef struct Ast_Node {
    NodeType type;
    union {
        s64 val;
        char op;
    };
    struct Ast_Node *left;
    struct Ast_Node *right;
}Ast_Node;

typedef struct Ast {
    Ast_Node root;
} Ast;

Ast_Node* make_op(char op) {
    Ast_Node* result = malloc(sizeof(Ast_Node));
    result->type = NODETYPE_OP;
    result->op = op;
    result->left = result->right = NULL;
    return result;
}

Ast_Node* make_val(s64 val) {
    Ast_Node* result = malloc(sizeof(Ast_Node));
    result->type = NODETYPE_VAL;
    result->val = val;
    result->left = result->right = NULL;
    return result;
}

Ast_Node* parse_expr3() {
    assert(is_token(TOKENKIND_VAL));
    Ast_Node* result = make_val(token.val);
    next_token();
    return result;
}

Ast_Node* parse_expr2() {
    Ast_Node* op_node = NULL;
    if (match_token(TOKENKIND_MIN)) {
        op_node = make_op('-');
    }
    if (match_token(TOKENKIND_COMPL)) {
        op_node = make_op('~');
    }
    Ast_Node* n = parse_expr3();
    if (op_node != NULL) {
        op_node->right = n;
        return op_node;
    }
    return n;
}

Ast_Node* parse_expr1() {
    Ast_Node* n = parse_expr2();
    Ast_Node* result = n;
    Ast_Node* left = n;
    
    while (is_token(TOKENKIND_MUL) || is_token(TOKENKIND_DIV) || is_token(TOKENKIND_MOD) || 
           is_token(TOKENKIND_AND) || is_token(TOKENKIND_SHFTL) || is_token(TOKENKIND_SHFTR)) {
        
        char op;
        
        if(match_token(TOKENKIND_MUL)) {
            op = '*';
        }
        
        if(match_token(TOKENKIND_DIV)) {
            op = '/';
        }
        
        if(match_token(TOKENKIND_MOD)) {
            op = '%';
        }
        
        if(match_token(TOKENKIND_AND)) {
            op = '&';
        }
        
        if(match_token(TOKENKIND_SHFTL)) {
            //TODO(Florian): Handle string instead of char
            op = '<';
        }
        
        if(match_token(TOKENKIND_SHFTR)) {
            //TODO(Florian): Handle string instead of char
            op = '>';
        }
        
        result = make_op(op);
        result->left = left;
        result->right = parse_expr2();
        left = result;
    }
    return result;
}

Ast_Node* parse_expr0() {
    Ast_Node* n = parse_expr1();
    Ast_Node* result = n;
    Ast_Node* left = n;
    while (is_token(TOKENKIND_ADD) || is_token(TOKENKIND_MIN) || is_token(TOKENKIND_OR) || is_token(TOKENKIND_XOR)) {
        
        char op;
        
        if(match_token(TOKENKIND_ADD)) {
            op = '+';
            
        }
        
        if(match_token(TOKENKIND_MIN)) {
            op = '-';
        }
        
        if(match_token(TOKENKIND_OR)) {
            op = '|';
        }
        
        if(match_token(TOKENKIND_XOR)) {
            op = '^';
        }
        
        result = make_op(op);
        result->left = left;
        result->right = parse_expr1();
        left = result;
    }
    return result;
}

Ast_Node* parse_expr() {
    return parse_expr0();
}

void print_Ast(Ast_Node *n) {
    switch (n->type) {
        case NODETYPE_OP: {
            printf("(%c ", n->op);
            if (n->left) { print_Ast(n->left); }
            if (n->right) { print_Ast(n->right); }
            printf(")");
        } break;
        case NODETYPE_VAL: {
            printf("%lld ", n->val);
        }break;
    }
}

void parse_test() {
    printf("\n");
    stream_init("28*8+19/7-15");
    Ast_Node* r = parse_expr();
    print_Ast(r);
    printf("\n");
    stream_init("1<<5>>2");
    r = parse_expr();
    print_Ast(r);
}

void do_test() {
    buf_test();
    lex_test();
    inter_test();
    parse_test();
}

int main(int argc, char** argv) {
    do_test();
    return 0;
}
