package io.github.leo1024.otrvm.parser;

import io.github.leo1024.otrvm.conf.*;
import io.github.leo1024.otrvm.ex.ParserException;
import io.github.leo1024.otrvm.lexer.Token;
import io.github.leo1024.otrvm.parser.impl.*;
import io.github.leo1024.otrvm.parser.impl.Math;

import java.util.ArrayList;
import java.util.List;

public class Parser {
    final TokenSequence tokenSequence;

    public Parser(List<Token> tokenList) {
        this.tokenSequence = new TokenSequence(tokenList);
    }

    public Context parse() {
        Context context = new Context();
        while (!tokenSequence.isEof()) {
            parseExpr(context);
        }
        return context;
    }


    private void parseExpr(Context context) {
        if (tokenSequence.isEof()) {
            throw ParserException.create(tokenSequence.consume(), "Unexpected end of file.");
        }
        // VM options
        if (tokenSequence.currentKind() == TokenKind.VM_OPTIONS) {
            this.convertVMOpts(context);
        }
        // Expr
        else {
            Expr expr = switch (tokenSequence.currentKind()) {
                case PSEUDO -> parsePseudo(context);
                case OPCODE -> parseOpcode(context);
                default -> throw ParserException.create(tokenSequence.consume(), "Unexpected token.");
            };
            context.addExpr(expr);
        }
    }

    private Expr parsePseudo(Context context) {
        Token token = this.tokenSequence.consume();
        return switch (token.toPseudo()) {
            case FUNC -> parseFunc(context);
            case END -> new End();
            default -> null;
        };
    }

    private Expr parseFunc(final Context context) {
        Token name = Helper.expect(tokenSequence, TokenKind.IDENTIFIER);
        Helper.expect(tokenSequence, Constants.LEFT_PAREN);
        boolean emptyParam = this.tokenSequence.checkToken(token -> {
            Helper.requireTokenNotNull(token, "Func param can't norm enclose.");
            return token.valEqual(Constants.RIGHT_PAREN);
        });
        List<Func.Param> paramList = new ArrayList<>();
        while (!emptyParam) {
            Token paramType = Helper.expect(tokenSequence, TokenKind.TYPE);
            Token paramName = Helper.expect(tokenSequence, TokenKind.IDENTIFIER);
            Func.Param param = new Func.Param(paramName.toId(), paramType.toType());
            paramList.add(param);
            if (!this.tokenSequence.checkToken(token -> token.valEqual(Constants.COMMA))) {
                emptyParam = true;
            }
        }
        Helper.expect(tokenSequence, Constants.RIGHT_PAREN);
        boolean declareRetType = this.tokenSequence.checkToken(token -> token != null && token.valEqual(Constants.COLON));
        Type retType = null;
        if (declareRetType) {
            retType = Helper.expect(tokenSequence, TokenKind.TYPE).toType();
        }
        Func func = new Func(context, name.toId(), paramList, retType);
        while (!this.tokenSequence.checkToken(it -> Helper.checkPseudoToken(it, Pseudo.END))) {
            parseExpr(func);
        }

        return func;
    }

    private void convertVMOpts(Context context) {
        VMOption option = VMOption.of(Helper.expect(this.tokenSequence, TokenKind.VM_OPTIONS));
        Object value = Helper.convertLiteral(Helper.expect(this.tokenSequence, TokenKind.STRING, TokenKind.INTEGER, TokenKind.HEX, TokenKind.FLOAT));
        context.setOption(option, value);
    }


    private Expr parseOpcode(final Context context) {
        Token token = this.tokenSequence.consume();
        Opcode opcode = Opcode.of(token);
        Expr expr = switch (opcode) {
            case LOAD, GLOAD -> parseLoadExpr(opcode);
            case STORE, GSTORE -> parseStoreExpr(opcode);
            case ADD, SBC, MUL, DIV -> new Math(opcode);
            default -> throw ParserException.create(token, "Unsupported opcode.");
        };
        return expr;
    }

    private Expr parseLoadExpr(final Opcode opcode) {
        Helper.expect(this.tokenSequence, Constants.DOT);
        OperandSize size = Helper.convertOperandSize(this.tokenSequence);
        Token token = this.tokenSequence.consume();
        Helper.requireTokenNotNull(token, "Load instruct missing operand.");
        boolean immediate = token.isImmediate();
        if (!immediate && !token.tokenKindIn(TokenKind.REF_VAR)) {
            throw ParserException.create(token, "Load instruct can't use immediate or ref var.");
        }
        final DataFrom from;
        if (opcode == Opcode.GLOAD) {
            if (immediate) {
                throw new ParserException("Global variable can't use immediate.");
            }
            from = DataFrom.GL;
        } else {
            from = token.isImmediate() ? DataFrom.IMM : DataFrom.LC;
        }
        int index = 0;
        Number immValue = null;
        if (immediate) {
            Object value = Helper.convertLiteral(token);
            if (!(value instanceof Number)) {
                throw new ParserException("Load only support number immediate.");
            }
            immValue = (Number) value;
        } else {
            index = Helper.convertVarRefIndex(token);
        }
        return new Load(size, from, immValue, index);
    }

    public Expr parseStoreExpr(Opcode opcode) {
        Helper.expect(this.tokenSequence, Constants.DOT);
        OperandSize size = Helper.convertOperandSize(this.tokenSequence);
        DataFrom from = opcode == Opcode.STORE ? DataFrom.LC : DataFrom.GL;
        int index = Helper.convertVarRefIndex(this.tokenSequence.consume());
        return new Store(from, index, size);
    }
}
