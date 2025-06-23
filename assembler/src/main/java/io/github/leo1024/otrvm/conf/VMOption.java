package io.github.leo1024.otrvm.conf;

import io.github.leo1024.otrvm.ex.ParserException;
import io.github.leo1024.otrvm.lexer.Token;

import java.util.Arrays;

public enum VMOption {
    LOCALS("%locals"),
    STACK("%stacks");

    private final String code;

    VMOption(String code) {
        this.code = code;
    }

    public String getCode() {
        return code;
    }


    public static VMOption of(String code) {
        return Arrays.stream(values())
                .filter(value -> value.code.equals(code))
                .findAny()
                .orElse(null);
    }

    public static VMOption of(Token token) {
        if (token == null || !token.tokenKindIn(TokenKind.VM_OPTIONS)) {
            throw ParserException.create(token, "Invalid VM option");
        }
        return of(token.getValue());
    }
}
