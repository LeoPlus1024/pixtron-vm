package io.github.leo1024.otrvm.conf;

import java.util.Arrays;

public enum Type {
    NULL("null", 0, 0),
    BYTE("byte", 1, 1),
    SHORT("short", 2, 2),
    INT("int", 3, 4),
    LONG("long", 4, 8),
    DOUBLE("double", 5, 8),
    BOOL("bool", 6, 1),
    HANDLE("handle", 7, 8),
    VOID("void", 8, 0),
    STRING("string", 9, 0),
    ARRAY("array", 10, 0),
    OBJECT("object", 11, 0),
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
