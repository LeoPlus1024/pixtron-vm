package io.github.leo1024.otrvm.parser;

import io.github.leo1024.otrvm.conf.TokenKind;
import io.github.leo1024.otrvm.lexer.Token;

import java.util.List;
import java.util.function.Predicate;

public class TokenSequence {
    final List<Token> tokenList;
    int index;

    public TokenSequence(List<Token> tokenList) {
        this.index = 0;
        this.tokenList = tokenList;
    }

    public Token consume() {
        Token token = tokenList.get(index);
        if (token == null) {
            return null;
        }
        index = index + 1;
        return token;
    }

    public TokenKind currentKind() {
        if (isEof()) {
            throw new ArrayIndexOutOfBoundsException("");
        }
        return tokenList.get(index).getKind();
    }

    public boolean checkToken(Predicate<Token> predicate) {
        return checkToken(predicate, true);
    }

    public boolean checkToken(Predicate<Token> predicate, boolean move2Next) {
        final Token token;
        if (isEof()) {
            token = null;
        } else {
            token = this.tokenList.get(index);
        }
        boolean success = predicate.test(token);
        if (success && move2Next) {
            this.index++;
        }
        return success;
    }

    public boolean isEof() {
        return this.index >= tokenList.size();
    }

}
