typedef enum VmOpCode {
    ADD = 1,
    SUB,
    OR,
    XOR,
    MUL,
    DIV,
    MOD,
    SHFTL,
    SHFTR,
    AND,
    COMPL,
    NEG,
    
    LIT = 254,
    HALT = 255,
} VmOpCode;

