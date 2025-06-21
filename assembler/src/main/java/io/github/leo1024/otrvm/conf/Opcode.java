package io.github.leo1024.otrvm.conf;

import io.github.leo1024.otrvm.ex.ParserException;
import io.github.leo1024.otrvm.lexer.Token;

import java.util.Arrays;

public enum Opcode {
    LOAD("load", 0),
    GLOAD("gload", 0),
    STORE("store", 1),
    GSTORE("gstore", 1),
    ADD("add", 2),
    SUB("sub", 3),
    MUL("mul", 4),
    DIV("div", 5),
    GOTO("goto", 6),
    RET("ret", 7),
    I2L("i2l", 8),
    I2F("i2f", 9),
    L2I("l2i", 10),
    L2F("l2f", 11),
    F2I("f2i", 12),
    F2L("f2l", 13),
    CALL("call", 14),
    IFEQ("ifeq", 15),
    IFNE("ifne", 16),
    IFLT("iflt", 17),
    IFGE("ifge", 18),
    IFGT("ifgt", 19),
    IFLE("ifle", 20),
    ICMP("icmp", 21),
    LCMP("lcmp", 22),
    DCMP("dcmp", 23),
    POP("pop", 24),
    ;


    private final byte value;
    private final String mnemonic;

    Opcode(String mnemonic, int value) {
        this.mnemonic = mnemonic;
        this.value = (byte) value;
    }

    public byte getValue() {
        return value;
    }

    public String getMnemonic() {
        return mnemonic;
    }

    public static Opcode of(String value) {
        return Arrays.stream(values())
                .filter(it -> it.mnemonic.equals(value))
                .findAny()
                .orElse(null);
    }

    public static Opcode of(Token token) {
        if (token == null || !token.tokenKindIn(TokenKind.OPCODE)) {
            throw ParserException.create(token, "Invalid opcode.");
        }
        return of(token.getValue());
    }

    @Override
    public String toString() {
        return this.mnemonic;
    }
}
