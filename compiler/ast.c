void* ast_alloc(size_t size) {
    assert(size != 0);
    void* mem = malloc(size);
    memset(mem, 0, size);
    return mem;
}

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

