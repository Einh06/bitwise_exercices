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
