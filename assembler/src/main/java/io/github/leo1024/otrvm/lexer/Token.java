package io.github.leo1024.otrvm.lexer;

import io.github.leo1024.otrvm.conf.TokenKind;

public class Token {
    private final TokenKind kind;
    private final String value;
    private final int line;
    private final int column;

    public Token(TokenKind kind, String value, int line, int column) {
        this.line = line;
        this.kind = kind;
        this.value = value;
        this.column = column;
    }

    public Token(TokenKind kind, char c, int line, int column) {
        this(kind, String.valueOf(c), line, column);
    }

    @Override
    public String toString() {
        return "[%d,%d]'%s'".formatted(line, column, value);
    }

    public TokenKind getKind() {
        return kind;
    }

    public String getValue() {
        return value;
    }

    public int getLine() {
        return line;
    }

    public int getColumn() {
        return column;
    }
}
