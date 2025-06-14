package io.github.leo1024.otrvm.conf;

import java.util.Arrays;

public enum Type {
    BYTE("byte"),
    SHORT("short"),
    INT("int"),
    LONG("short"),
    FLOAT("float");

    private final String symbol;

    Type(String symbol) {
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
}
