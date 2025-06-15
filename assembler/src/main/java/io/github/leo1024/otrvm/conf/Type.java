package io.github.leo1024.otrvm.conf;

import java.util.Arrays;

public enum Type {
    NIL("nil", 0, 1),
    BYTE("byte", 1, 1),
    SHORT("short", 2, 2),
    INT("int", 3, 4),
    LONG("long", 4, 8),
    FLOAT("float", 5, 8),
    BOOL("bool", 6, 1),
    STRING("str", 7, 8);

    private final String symbol;
    private final byte id;
    private final int length;

    Type(String symbol, int code, int length) {
        this.length = length;
        this.id = (byte) code;
        this.symbol = symbol;
    }

    public String getSymbol() {
        return symbol;
    }

    public static Type of(String symbol) {
        return Arrays.stream(values())
                .filter(it -> it.symbol.equals(symbol))
                .findAny()
                .orElse(null);
    }

    public boolean isPrimitive() {
        return this != STRING;
    }

    public byte getId() {
        return id;
    }

    public int getLength() {
        return length;
    }
}
