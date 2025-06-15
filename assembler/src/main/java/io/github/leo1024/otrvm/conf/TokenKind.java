package io.github.leo1024.otrvm.conf;

public enum TokenKind {
    DELIMITER,
    IDENTIFIER,
    HEX,
    REF_VAR,
    STRING,
    PSEUDO,
    OPCODE,
    FLOAT,
    INTEGER,
    TYPE,
    VM_OPTIONS;


    public static TokenKind[] immediate() {
        return new TokenKind[]{STRING, HEX, FLOAT, INTEGER};
    }
}
