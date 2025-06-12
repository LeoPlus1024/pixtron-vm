package io.github.leo1024.otrvm.conf;

public enum Type {
    BYTE("byte"),
    SHORT("short"),
    INT("int"),
    LONG("short"),
    DOUBLE("double");

    private final String symbol;

    Type(String symbol) {
        this.symbol = symbol;
    }

    public String getSymbol() {
        return symbol;
    }
}
