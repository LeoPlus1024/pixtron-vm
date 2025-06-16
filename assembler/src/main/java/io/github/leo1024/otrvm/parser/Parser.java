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

    public ASTBuilder parse() {
        // Parse namespace
        Token token = this.tokenSequence.consume();
        boolean hasNamespace = Helper.checkPseudoToken(token, Pseudo.NAMESPACE);
        if (!hasNamespace) {
            throw ParserException.create(token, "Expect a namespace.");
        }
        String namespace = Helper.expect(this.tokenSequence, TokenKind.IDENTIFIER).getValue();
        final ASTBuilder context = new ASTBuilder(namespace);
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
            TokenKind tokenKind = this.tokenSequence.currentKind();
            switch (tokenKind) {
                case PSEUDO -> parsePseudo(context);
                case OPCODE -> parseOpcode(context);
                default -> {
                    Token token = this.tokenSequence.consume();
                    if (tokenKind == TokenKind.IDENTIFIER
                            && this.tokenSequence.checkToken(it -> it != null && it.valEqual(Constants.COLON))) {
                        context.addLabel(token.getValue());
                    } else {
                        throw ParserException.create(token, "Unexpected token.");
                    }
                }
            }
        }
    }

    private void parsePseudo(Context context) {
        Token token = this.tokenSequence.consume();
        Pseudo pseudo = token.toPseudo();
        if (pseudo == Pseudo.VAR) {
            this.parseVar(context);
        } else {
            Expr expr = switch (pseudo) {
                case FUNC -> parseFunc(context);
                case END -> new End();
                default -> null;
            };
            context.addExpr(expr);
        }
    }


    private void parseVar(Context context) {
        Type type = Helper.expect(this.tokenSequence, TokenKind.TYPE).toType();
        String name = Helper.expect(this.tokenSequence, TokenKind.IDENTIFIER).getValue();
        Object value = Helper.convertLiteral(Helper.expect(this.tokenSequence, TokenKind.immediate()));
        context.addVar(type, name, value);
    }


    private Expr parseFunc(final Context context) {
        Token name = Helper.expect(tokenSequence, TokenKind.IDENTIFIER);
        Helper.expect(tokenSequence, Constants.LEFT_PAREN);
        boolean emptyParam = this.tokenSequence.checkToken(token -> {
            Helper.requireTokenNotNull(token, "Func param can't norm enclose.");
            return token.valEqual(Constants.RIGHT_PAREN);
        });
        List<FuncMeta.Param> paramList = new ArrayList<>();
        while (!emptyParam) {
            Token paramType = Helper.expect(tokenSequence, TokenKind.TYPE);
            Token paramName = Helper.expect(tokenSequence, TokenKind.IDENTIFIER);
            FuncMeta.Param param = new FuncMeta.Param(paramName.toId(), paramType.toType());
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
        FuncMeta funcMeta = new FuncMeta(null, name.toId(), retType, paramList);
        Func func = new Func(context, funcMeta);
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


    private void parseOpcode(final Context context) {
        Token token = this.tokenSequence.consume();
        Opcode opcode = Opcode.of(token);
        Expr expr = switch (opcode) {
            case LOAD, GLOAD -> parseLoadExpr(opcode);
            case STORE, GSTORE -> parseStoreExpr(opcode);
            case ADD, SBC, MUL, DIV -> new Math(opcode);
            case GOTO -> {
                String label = Helper.expect(this.tokenSequence, TokenKind.IDENTIFIER).getValue();
                yield new Redirect(opcode, label);
            }
            default -> throw ParserException.create(token, "Unsupported opcode.");
        };
        context.addExpr(expr);
    }

    private Expr parseLoadExpr(final Opcode opcode) {
        Helper.expect(this.tokenSequence, Constants.DOT);
        Type type = Helper.convertOperandType(this.tokenSequence);
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
        return new Load(type, from, immValue, index);
    }

    public Expr parseStoreExpr(Opcode opcode) {
        Helper.expect(this.tokenSequence, Constants.DOT);
        Type type = Helper.convertOperandType(this.tokenSequence);
        DataFrom from = opcode == Opcode.STORE ? DataFrom.LC : DataFrom.GL;
        int index = Helper.convertVarRefIndex(this.tokenSequence.consume());
        return new Store(from, index, type);
    }
}
