package io.github.leo1024.otrvm.ex;

import io.github.leo1024.otrvm.lexer.Token;

import java.util.Optional;

public class ParserException extends RuntimeException {
    public ParserException(String message) {
        super(message);
    }

    public ParserException(Throwable cause) {
        super(cause);
    }

    public static ParserException create(int line, int column, String title, String text) {
        String message = String.format("(%d,%d): %s\n> %s", line, column, title, text);
        return new ParserException(message);
    }

    public static ParserException create(Token token, String title) {
        String position = Optional.ofNullable(token).map(Token::getPosition).orElse("?,?");
        String message = String.format("[%s] %s", position, title);
        return new ParserException(message);
    }
}
