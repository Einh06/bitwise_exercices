#define POP() (*--top)
#define PUSH(x) (*top++ = (x))
#define POPS(n) assert(top - stack >= (n))
#define PUSHES(n) assert(top + (n) <= stack + MAX_STACK) 

s64 vm_exec(const u8* code) {
    enum { MAX_STACK = 1024};
    s64 stack[MAX_STACK] = {0};
    s64* top = stack;
    for (;;) {
        u8 op = *code++;
        switch (op) {
            
            case ADD:
            {
                POPS(2);
                s64 right = POP();
                s64 left = POP();
                PUSH((right + left));
            }break;
            
            case SUB:
            {
                POPS(2);
                s64 right = POP();
                s64 left = POP();
                PUSH(right - left);
            } break;
            
            case OR:
            {
                POPS(2);
                s64 right = POP();
                s64 left = POP();
                PUSH(right | left);
            }break;
            
            case XOR:
            {
                POPS(2);
                s64 right = POP();
                s64 left = POP();
                PUSH(right ^ left);
            }break;
            
            case MUL:
            {
                POPS(2);
                s64 right = POP();
                s64 left = POP();
                PUSH(right * left);
            }break;
            
            case DIV:
            {
                POPS(2);
                s64 right = POP();
                s64 left = POP();
                PUSH(right / left);
            }break;
            
            case MOD:
            {
                POPS(2);
                s64 right = POP();
                s64 left = POP();
                PUSH(right % left);
            }break;
            
            case SHFTL:
            {
                POPS(2);
                s64 right = POP();
                s64 left = POP();
                PUSH(right << left);
            }break;
            
            case SHFTR:
            {
                POPS(2);
                s64 right = POP();
                s64 left = POP();
                PUSH(right >> left);
            }break;
            
            case AND:
            {
                POPS(2);
                s64 right = POP();
                s64 left = POP();
                PUSH(right & left);
            }break;
            
            case COMPL:
            {
                POPS(1);
                s64 val = POP();
                PUSH(~val);
            }break;
            
            case NEG:
            {
                POPS(1);
                s64 val = POP();
                PUSH(-val);
            }break;
            
            case LIT:
            {
                PUSHES(1);
                PUSH(*(s64*)code); //assumes little endian
                code += sizeof(s64);
                break;
            }
            
            case HALT: 
            {
                POPS(1);
                return POP();
            }
            default:
            {
                printf("Unrecognized code: %d\n", op);
                exit(1);
            }
        }
    }
}

void vm_test() {
    
    {
        u8 code[] = { LIT, 1, 0, 0, 0, 0, 0, 0, 0, HALT };
        s64 result = vm_exec(code);
        assert(result == 1);
    }
    
    {
        u8 code[] = {LIT, 1, 0, 0, 0, 0, 0, 0, 0, LIT, 1, 0, 0, 0, 0, 0, 0, 0, ADD, HALT};
        s64 result = vm_exec(code);
        assert(result == 2);
    }
    
    {
        u8 code[] = { 
            LIT, 0x0F, 0, 0, 0, 0, 0, 0, 0, 
            NEG, 
            LIT, 0x13, 0, 0, 0, 0, 0, 0, 0,
            ADD,
            LIT, 0x07, 0, 0 ,0, 0, 0, 0, 0,
            MUL,
            HALT };
        
        s64 result = vm_exec(code);
        assert(result == 28);
    }
}