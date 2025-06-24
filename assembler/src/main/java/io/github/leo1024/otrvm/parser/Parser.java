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
        if (pseudo == Pseudo.FIELD) {
            this.parseField(context);
        } else if (pseudo == Pseudo.IMPORT) {
            this.parseImport(context);
        } else if (pseudo == Pseudo.CONSTANT) {
            this.parseConstant(context);
        } else if (pseudo == Pseudo.LIBRARY) {
            this.parseLibrary(context);
        } else {
            Expr expr = switch (pseudo) {
                case FUNC -> parseFunc(context);
                case END -> new End();
                default -> null;
            };
            context.addExpr(expr);
        }
    }

    public void parseImport(Context context) {
        Helper.expect(this.tokenSequence, Constants.LEFT_BRACKET);
        List<String> methods = new ArrayList<>();
        for (; ; ) {
            Token token = this.tokenSequence.consume();
            Helper.requireTokenNotNull(token, "Import can't normal enclose.");
            if (token.valEqual(Constants.RIGHT_BRACKET)) {
                break;
            }
            if (token.getKind() != TokenKind.IDENTIFIER) {
                throw ParserException.create(token, "Import body only support identifier.");
            }
            methods.add(token.getValue());
            this.tokenSequence.checkToken(it -> {
                Helper.requireTokenNotNull(it, "Except a ',' or '}'");
                return it.valEqual(Constants.COMMA);
            });
        }
        Helper.expect(this.tokenSequence, Constants.FROM);
        Token nameSpaceToken = Helper.expect(this.tokenSequence, TokenKind.IDENTIFIER);
        for (String method : methods) {
            FuncMeta funcMeta = new FuncMeta(true, nameSpaceToken.getValue(), new Id(method), Type.VOID, List.of(), false, null);
            context.addExpr(new Func(context, funcMeta));
        }
    }


    private void parseField(Context context) {
        Type type = Helper.expect(this.tokenSequence, TokenKind.TYPE).toType();
        String name = Helper.expect(this.tokenSequence, TokenKind.IDENTIFIER).getValue();
        Object value = Helper.convertLiteral(Helper.expect(this.tokenSequence, TokenKind.immediate()));
        FieldMeta fieldMeta = FieldMeta.of(type, name, value);
        context.addField(fieldMeta);
    }

    /**
     * Parse constant
     *
     * @apiNote Current only support string constant.
     */
    private void parseConstant(Context context) {
        TokenKind tokenKind = this.tokenSequence.currentKind();
        if (tokenKind != TokenKind.STRING) {
            throw ParserException.create(this.tokenSequence.consume(), "Only support string constant.");
        }
        Object value = Helper.convertLiteral(this.tokenSequence.consume());
        context.addConstant(Type.STRING, value);
    }

    private void parseLibrary(Context context) {
        Helper.expect(this.tokenSequence, Constants.LEFT_PAREN);
        Token libraryToken = Helper.expect(this.tokenSequence, TokenKind.STRING);
        Helper.expect(this.tokenSequence, Constants.RIGHT_PAREN);
        ASTBuilder builder = (ASTBuilder) context;
        builder.setLibrary(libraryToken.getValue());
    }


    private Expr parseFunc(final Context context) {
        boolean isNativeFunc = false;
        String libNames = null;
        if (this.tokenSequence.currentKind() == TokenKind.PSEUDO) {
            Token token = this.tokenSequence.consume();
            isNativeFunc = Helper.checkPseudoToken(token, Pseudo.NATIVE);
            if (!isNativeFunc) {
                throw ParserException.create(token, "Except a @native in function define.");
            }
            boolean hasParam = this.tokenSequence.checkToken(it -> it != null && it.valEqual(Constants.LEFT_PAREN));
            if (hasParam) {
                if (this.tokenSequence.currentKind() == TokenKind.STRING) {
                    libNames = Helper.expect(this.tokenSequence, TokenKind.STRING).getValue();
                }
                Helper.expect(this.tokenSequence, Constants.RIGHT_PAREN);
            }
        }
        Token name = Helper.expect(tokenSequence, TokenKind.IDENTIFIER);
        Helper.expect(tokenSequence, Constants.LEFT_PAREN);
        boolean emptyParam = this.tokenSequence.checkToken(token -> {
            Helper.requireTokenNotNull(token, "Func param can't norm enclose.");
            return token.valEqual(Constants.RIGHT_PAREN);
        });
        List<FuncMeta.Param> paramList = new ArrayList<>();
        if (!emptyParam) {
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
        }
        boolean declareRetType = this.tokenSequence.checkToken(token -> token != null && token.valEqual(Constants.COLON));
        Type retType = null;
        if (declareRetType) {
            retType = Helper.expect(tokenSequence, TokenKind.TYPE).toType();
        }
        FuncMeta funcMeta = new FuncMeta(false, null, name.toId(), retType, paramList, isNativeFunc, libNames);
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
            case RET -> new Ret();
            case ASSERT -> parserAssert();
            case LOAD, GET_FIELD, LOAD_CONST -> parseLoadExpr(opcode);
            case STORE, SET_FIELD -> parseStoreExpr(opcode);
            case ADD, SUB, MUL, DIV -> new Math(opcode);
            case F2I, F2L, I2L, I2F, L2I, L2F -> new Cast(opcode);
            case ICMP, LCMP, DCMP -> new XCmp(opcode);
            case CALL -> parserCallExpr();
            case GOTO, IFEQ, IFNE, IFLE, IFGE, IFGT, IFLT -> {
                String label = Helper.expect(this.tokenSequence, TokenKind.IDENTIFIER).getValue();
                yield new Redirect(opcode, label);
            }
            default -> throw ParserException.create(token, "Unsupported opcode.");
        };
        context.addExpr(expr);
    }

    private Expr parserAssert() {
        Token token = Helper.expect(this.tokenSequence, TokenKind.INTEGER);
        int index = Integer.parseInt(token.getValue());
        return new Assert(index);
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
        if (opcode == Opcode.GET_FIELD) {
            if (immediate) {
                throw new ParserException("Global variable can't use immediate.");
            }
            from = DataFrom.GL;
        } else if (opcode == Opcode.LOAD_CONST) {
            from = DataFrom.CONSTANT;
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

    private Expr parseStoreExpr(Opcode opcode) {
        Helper.expect(this.tokenSequence, Constants.DOT);
        Type type = Helper.convertOperandType(this.tokenSequence);
        DataFrom from = opcode == Opcode.STORE ? DataFrom.LC : DataFrom.GL;
        int index = Helper.convertVarRefIndex(this.tokenSequence.consume());
        return new Store(from, index, type);
    }

    private Expr parserCallExpr() {
        Token methodName = Helper.expect(this.tokenSequence, TokenKind.IDENTIFIER);
        return new Call(methodName.getValue());
    }
}
