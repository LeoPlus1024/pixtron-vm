package io.github.leo1024.otrvm.lexer;

import io.github.leo1024.otrvm.conf.Opcode;
import io.github.leo1024.otrvm.conf.Pseudo;
import io.github.leo1024.otrvm.conf.TokenKind;
import io.github.leo1024.otrvm.conf.Type;
import io.github.leo1024.otrvm.parser.impl.Id;

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
        return "(%s)[%d,%d]'%s'".formatted(kind, line, column, value);
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


    public Pseudo toPseudo() {
        return Pseudo.of(value);
    }

    public Opcode toOpcode() {
        return Opcode.of(value);
    }

    public boolean valEqual(char c) {
        return this.value.equals(String.valueOf(c));
    }


    public String getPosition() {
        return "line " + getLine() + ":" + getColumn();
    }

    public Id toId() {
        return new Id(value);
    }

    public Type toType() {
        return Type.of(value);
    }

    public boolean tokenKindIn(TokenKind... kinds) {
        for (TokenKind tokenKind : kinds) {
            if (tokenKind == kind) {
                return true;
            }
        }
        return false;
    }

    public boolean isImmediate() {
        for (TokenKind tokenKind : TokenKind.immediate()) {
            if (this.kind == tokenKind) {
                return true;
            }
        }
        return false;
    }
}
