package io.github.leo1024.otrvm.conf;

import java.util.Arrays;

public enum Opcode {
    PUSH("push", 0),
    POP("pop", 1),
    ADD("add", 2),
    SBC("sbc", 3),
    MUL("mul", 4),
    DIV("div", 5),
    GOTO("goto", 6);


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
}
