#if 0
expr3 = VAL
expr2 = [-~] expr3
expr1 = expr2 ([*/%&] expr2)*
expr0 = expr1 ([+-|^] expr1)*
expr = expr0
#endif

Expr* parse_expr();

Expr* parse_term() {
    Expr* e;
    if (match_token(TOKENKIND_OBRACKET)) {
        e = parse_expr();
        expect_token(TOKENKIND_CBRACKET);
    } else {
        assert(is_token(TOKENKIND_VAL));
        e = expr_val(token.val);
        next_token();
    }
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