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

