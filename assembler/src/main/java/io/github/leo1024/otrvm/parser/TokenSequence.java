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
        final Token token;
        if (isEof()) {
            token = null;
        } else {
            token = this.tokenList.get(index);
        }
        boolean move2Next = predicate.test(token);
        if (move2Next) {
            this.index++;
        }
        return move2Next;
    }

    public boolean isEof() {
        return this.index >= tokenList.size();
    }

}
