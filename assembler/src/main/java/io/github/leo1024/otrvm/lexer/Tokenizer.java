package io.github.leo1024.otrvm.lexer;

import io.github.leo1024.otrvm.conf.Constants;
import io.github.leo1024.otrvm.conf.Opcode;
import io.github.leo1024.otrvm.conf.Pseudo;
import io.github.leo1024.otrvm.conf.TokenKind;
import io.github.leo1024.otrvm.ex.TokenizerException;

import java.io.InputStream;
import java.util.ArrayList;
import java.util.List;
import java.util.regex.Pattern;

public class Tokenizer {
    static final Pattern INT_PATTERN = Pattern.compile("[0-9]+");
    static final Pattern FLOAT_PATTERN = Pattern.compile("[0-9]?\\.[0-9]+");

    final CharSequence charSequence;

    public Tokenizer(InputStream inputStream) {
        this.charSequence = new CharSequence(inputStream);
    }

    public List<Token> tokenize() {
        List<Token> tokenList = new ArrayList<>();
        for (; ; ) {
            char chr = this.charSequence.read();
            if (this.charSequence.isEof()) {
                break;
            }
            if (chr == Constants.CR || chr == Constants.LF || chr == Constants.TAB || chr == Constants.SPACE) {
                continue;
            }
            int line = this.charSequence.getLine();
            int column = this.charSequence.getColumn();
            Token token = switch (chr) {
                case Constants.COMMA,
                     Constants.LEFT_PAREN,
                     Constants.DOT,
                     Constants.RIGHT_PAREN -> new Token(TokenKind.DELIMITER, chr, line, column);
                case Constants.COMMENT, Constants.DOLLAR -> new Token(TokenKind.LITERAL_PREFIX, chr, line, column);
                default -> {
                    String text = this.charSequence.readUntilDelimiter();
                    if (Character.isDigit(chr)) {
                        yield createDigitToken(line, column, text);
                    }
                    Opcode opcode = Opcode.of(text);
                    if (opcode != null) {
                        yield new Token(TokenKind.OPCODE, text, line, column);
                    }
                    Pseudo pseudo = Pseudo.of(text);
                    if (pseudo != null) {
                        yield new Token(TokenKind.PSEUDO, text, line, column);
                    }
                    yield new Token(TokenKind.TEXT, text, line, column);
                }
            };
            tokenList.add(token);
        }
        return tokenList;
    }

    private Token createDigitToken(int line, int column, String text) {
        boolean isFloat = FLOAT_PATTERN.matcher(text).matches();
        if (isFloat) {
            return new Token(TokenKind.FLOAT, text, line, column);
        }
        boolean isInt = INT_PATTERN.matcher(text).matches();
        if (isInt) {
            return new Token(TokenKind.INTEGER, text, line, column);
        }
        throw TokenizerException.create(line, column, "Invalid digit token", text);
    }
}
