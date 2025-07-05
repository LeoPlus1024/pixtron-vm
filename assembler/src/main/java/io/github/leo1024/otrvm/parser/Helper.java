package io.github.leo1024.otrvm.parser;

import io.github.leo1024.otrvm.conf.Pseudo;
import io.github.leo1024.otrvm.conf.TokenKind;
import io.github.leo1024.otrvm.conf.Type;
import io.github.leo1024.otrvm.ex.ParserException;
import io.github.leo1024.otrvm.lexer.Token;

import java.util.Arrays;

public class Helper {
    public static Token expect(TokenSequence tokenSequence, TokenKind... kinds) {
        if (kinds.length == 0) {
            throw new ParserException("Kinds can't empty.");
        }
        String text = Arrays.toString(kinds);
        if (tokenSequence.isEof()) {
            throw new ParserException("Expected token '%s' but reached end of input".formatted(text));
        }

        Token token = tokenSequence.consume();
        if (!token.tokenKindIn(kinds)) {
            throw new ParserException("[%s] Expected token '%s' but found '%s'".formatted(token.getPosition(), text, token.getKind()));
        }
        return token;
    }

    public static Token expect(TokenSequence tokenSequence, char chr) {
        return expect(tokenSequence, String.valueOf(chr));
    }

    public static Token expect(TokenSequence tokenSequence, String value) {
        if (tokenSequence.isEof()) {
            throw new ParserException("Expected token value '%s' but reached end of input.".formatted(value));
        }

        Token token = tokenSequence.consume();
        if (!token.getValue().equals(value)) {
            throw new ParserException("[%s] Expected token value '%s' but found '%s'".formatted(token.getPosition(), value, token.getValue()));
        }
        return token;
    }


    public static Token requireTokenNotNull(Token token, String message) {
        if (token != null) {
            return token;
        }
        throw new ParserException(message);
    }

    public static boolean checkPseudoToken(Token token, Pseudo pseudo) {
        if (token == null || token.getKind() != TokenKind.PSEUDO) {
            if (token == null) {
                throw new ParserException("Expected token kind '%s' but reach eof.".formatted(TokenKind.PSEUDO));
            }
        }
        return token.getValue().equals(pseudo.getCode());
    }

    public static Type convertOperandType(TokenSequence sequence) {
        Token token = sequence.consume();
        if (token == null) {
            throw ParserException.create(token, "Expected a operand size but it reach eof.");
        }
        String suffix = token.getValue();
        return switch (suffix) {
            case "i8" -> Type.BYTE;
            case "i16" -> Type.SHORT;
            case "i32" -> Type.INT;
            case "i64" -> Type.LONG;
            case "f64" -> Type.DOUBLE;
            case "bool" -> Type.BOOL;
            case "ptr" -> Type.REF;
            case "str" -> Type.STRING;
            case "arr" -> Type.ARRAY;
            default -> throw new IllegalStateException("Unexpected value: " + suffix);
        };
    }

    public static Object convertLiteral(Token token) {
        if (token.tokenKindIn(TokenKind.STRING)) {
            return token.getValue();
        } else if (token.tokenKindIn(TokenKind.HEX)) {
            return Long.parseLong(token.getValue().substring(1), 16);
        } else if (token.tokenKindIn(TokenKind.INTEGER)) {
            return Long.parseLong(token.getValue());
        } else if (token.tokenKindIn(TokenKind.FLOAT)) {
            return Double.parseDouble(token.getValue());
        } else {
            throw ParserException.create(token, "Can't convert token '%s' to literal.".formatted(token.getKind()));
        }
    }

    public static short checkOpcodeIdx(final TokenSequence tokenSequence) {
        Token token = Helper.expect(tokenSequence, TokenKind.INTEGER);
        int index = token.toInt();
        if (index < 0 || index > 0xffff) {
            throw ParserException.create(token, "Index out of range.");
        }
        return (short) index;
    }

}
