package io.github.leo1024.otrvm.ex;

public class TokenizerException extends RuntimeException {
    public TokenizerException(String message) {
        super(message);
    }

    public TokenizerException(Throwable cause) {
        super(cause);
    }

    public static TokenizerException create(int line, int column, String title, String text) {
        String message = String.format("(%d,%d): %s\n> %s", line, column, title, text);
        return new TokenizerException(message);
    }
}
