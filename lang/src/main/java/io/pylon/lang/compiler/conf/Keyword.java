package io.pylon.lang.compiler.conf;

/**
 * Language define  keywords
 */
public enum Keyword {
    BYTE("byte"),
    SHORT("short"),
    INT("int"),
    LONG("long"),
    DOUBLE("double"),
    ;
    final String symbol;

    Keyword(final String symbol) {
        this.symbol = symbol;
    }

    public String getSymbol() {
        return symbol;
    }
}
