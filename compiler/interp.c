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
                        fatal("LEX: Unrecognized OPCODE");
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
    stream_init("28*(8+19)/7--15");
    s64 result = interpret_expr(parse_expr());
    assert(result == 123);
    stream_init("1 << 5 >> 2");
    result = interpret_expr(parse_expr());
    assert(result == 8);
}
