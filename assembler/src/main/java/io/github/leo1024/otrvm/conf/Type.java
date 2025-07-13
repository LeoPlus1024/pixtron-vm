package io.github.leo1024.otrvm.conf;

import java.util.Arrays;

public enum Type {
    BYTE("byte", 0, 1),
    SHORT("short", 1, 2),
    INT("int", 2, 4),
    LONG("long", 3, 8),
    DOUBLE("double", 4, 8),
    BOOL("bool", 5, 1),
    HANDLE("handle", 6, 8),
    VOID("void", 7, 0),
    STRING("string", 8, 0),
    ARRAY("array", 9, 0),
    OBJECT("object", 10, 0),
    ;

    private final String symbol;
    private final int id;
    private final int length;

    Type(String symbol, int code, int length) {
        this.length = length;
        this.id = code;
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
        return this != HANDLE && this != VOID;
    }

    public int getId() {
        return id;
    }

    public int getLength() {
        return length;
    }
}
