package io.github.leo1024.otrvm.lexer;

import io.github.leo1024.otrvm.conf.*;
import io.github.leo1024.otrvm.ex.ParserException;
import io.github.leo1024.otrvm.util.LexerUtil;

import java.io.InputStream;
import java.util.ArrayList;
import java.util.List;

public class Tokenizer {


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
            if (chr == Constants.CR || chr == Constants.LF || chr == Constants.TAB || chr == Constants.SPACE || chr == Constants.SEMICOLON) {
                if (chr == Constants.SEMICOLON) {
                    this.charSequence.skipComment();
                }
                continue;
            }
            int line = this.charSequence.getLine();
            int column = this.charSequence.getColumn();
            Token token = switch (chr) {
                case Constants.COMMA,
                     Constants.COLON,
                     Constants.LEFT_PAREN,
                     Constants.DOT,
                     Constants.RIGHT_PAREN -> new Token(TokenKind.DELIMITER, chr, line, column);
                // Hex  literal
                case Constants.COMMENT -> {
                    String hex = this.charSequence.readHexOrVariable();
                    if (!LexerUtil.checkHexLiteral(hex)) {
                        throw ParserException.create(line, column, "Invalid hex literal.", hex);
                    }
                    yield new Token(TokenKind.HEX, hex, line, column);
                }
                // variable reference
                case Constants.DOLLAR -> {
                    String text = this.charSequence.readHexOrVariable();
                    String index = text.substring(1);
                    if (!LexerUtil.isInteger(index)) {
                        throw ParserException.create(line, column, "Invalid variable reference.", text);
                    }
                    yield new Token(TokenKind.REF_VAR, text, line, column);
                }
                // VM options
                case Constants.PERCENT -> {
                    String text = this.charSequence.readUntilDelimiter();
                    if (VMOption.of(text) == null) {
                        throw ParserException.create(line, column, "Invalid VM options.", text);
                    }
                    yield new Token(TokenKind.VM_OPTIONS, text, line, column);
                }
                // String
                case Constants.DOUBLE_QUOTE ->
                        new Token(TokenKind.STRING, this.charSequence.readStrLiteral(), line, column);
                // Pseudo
                case Constants.AT -> {
                    String text = this.charSequence.readUntilDelimiter();
                    Pseudo pseudo = Pseudo.of(text);
                    if (pseudo == null) {
                        throw ParserException.create(line, column, "Invalid pseudo token", text);
                    }
                    yield new Token(TokenKind.PSEUDO, text, line, column);
                }
                // Opcode/Type/Identifier
                default -> {
                    String text = this.charSequence.readUntilDelimiter();
                    if (Character.isDigit(chr)) {
                        yield LexerUtil.createDigitToken(line, column, text);
                    }
                    // Opcode
                    if (Opcode.of(text) != null) {
                        yield new Token(TokenKind.OPCODE, text, line, column);
                    }
                    // Type
                    if (Type.of(text) != null) {
                        yield new Token(TokenKind.TYPE, text, line, column);
                    }
                    yield new Token(TokenKind.IDENTIFIER, text, line, column);
                }
            };
            tokenList.add(token);
        }
        return tokenList;
    }


}
