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
        
        case '(':
        {
            token.kind = TOKENKIND_OBRACKET;
            break;
        }
        
        case ')':
        {
            token.kind = TOKENKIND_CBRACKET;
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

void stream_init(char* expr) {
    stream = expr;
    next_token();
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
        fatal("Unexpected token found\n");
        return false;
    }
}

bool is_token_add(TokenKind kind) {
    return kind >= TOKENKIND_FIRST_ADD && kind <= TOKENKIND_LAST_ADD;
}

bool is_token_mul(TokenKind kind) {
    return kind >= TOKENKIND_FIRST_MUL && kind <= TOKENKIND_LAST_MUL;
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
    char *expr = "(12+54)*1234-2";
    stream_init(expr);
    assert(token.kind == TOKENKIND_OBRACKET);
    next_token();
    assert(token.kind == TOKENKIND_VAL);
    assert(token.val == 12);
    next_token();
    assert(token.kind == TOKENKIND_ADD);
    next_token();
    assert(token.kind == TOKENKIND_VAL);
    assert(token.val == 54);
    next_token();
    assert(token.kind == TOKENKIND_CBRACKET);
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


