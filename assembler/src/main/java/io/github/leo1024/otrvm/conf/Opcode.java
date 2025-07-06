package io.github.leo1024.otrvm.conf;

import io.github.leo1024.otrvm.ex.ParserException;
import io.github.leo1024.otrvm.lexer.Token;

import java.util.Arrays;

public enum Opcode {
    LI8("li8", 0),
    LI16("li16", 1),
    LI32("li32", 2),
    LI64("li64", 3),
    LF64("lf64", 4),
    LLOCAL("llocal", 5),
    SLOCAL("slocal", 6),
    ADD("add", 7),
    SUB("sub", 8),
    MUL("mul", 9),
    DIV("div", 10),
    GOTO("goto", 11),
    RET("ret", 12),
    I2L("i2l", 13),
    I2F("i2f", 14),
    L2I("l2i", 15),
    L2F("l2f", 16),
    F2I("f2i", 17),
    F2L("f2l", 18),
    CALL("call", 19),
    IFEQ("ifeq", 20),
    IFNE("ifne", 21),
    IFLT("iflt", 22),
    IFGE("ifge", 23),
    IFGT("ifgt", 24),
    IFLE("ifle", 25),
    ICMP("icmp", 26),
    LCMP("lcmp", 27),
    DCMP("dcmp", 28),
    POP("pop", 29),
    ASSERT("assert", 30),
    ISHL("ishl", 31),
    ISHR("ishr", 32),
    IUSHR("iushr", 33),
    LSHL("ishl", 34),
    LSHR("ishr", 35),
    LUSHR("lushr", 36),
    NEW_ARRAY("newarray", 37),
    GET_ARRAY("getarray", 38),
    SET_ARRAY("setarray", 39),
    IINC("iinc", 40),
    LFIELD("lfield", 41),
    SFIELD("sfield", 42),
    REFINC("refinc", 43),
    REFDEC("refdec", 44),
    SREFINC("srefinc", 45),
    SREFDEC("srefdec", 46),
    LDC("ldc", 47);

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
