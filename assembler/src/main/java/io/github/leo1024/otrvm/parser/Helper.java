package io.github.leo1024.otrvm.parser;

import io.github.leo1024.otrvm.conf.Pseudo;
import io.github.leo1024.otrvm.conf.TokenKind;
import io.github.leo1024.otrvm.conf.Type;
import io.github.leo1024.otrvm.ex.ParserException;
import io.github.leo1024.otrvm.lexer.Token;

import java.util.Arrays;
import java.util.Objects;

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
        if (tokenSequence.isEof()) {
            throw new ParserException("Expected token value '%c' but reached end of input.".formatted(chr));
        }

        Token token = tokenSequence.consume();
        if (!token.getValue().equals(String.valueOf(chr))) {
            throw new ParserException("[%s] Expected token value '%c' but found '%s'".formatted(token.getPosition(), chr, token.getValue()));
        }
        return token;
    }


    public static void requireTokenNotNull(Token token, String message) {
        if (token != null) {
            return;
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
            case "u" -> Type.BYTE;
            case "s" -> Type.SHORT;
            case "i" -> Type.INT;
            case "l" -> Type.LONG;
            case "f" -> Type.FLOAT;
            case "b" -> Type.BOOL;
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

    public static int convertVarRefIndex(Token token) {
        if (token == null || !token.tokenKindIn(TokenKind.REF_VAR)) {
            throw ParserException.create(token, "Can't convert token  to var ref index.");
        }
        String value = token.getValue().substring(1);
        int index = Integer.parseInt(value);
        if ((index & 0xFFFF0000) != 0) {
            throw ParserException.create(token, "Exceed variable refence max value.");
        }
        return index;
    }

}
