package io.github.leo1024.otrvm.parser;

import io.github.leo1024.otrvm.conf.*;
import io.github.leo1024.otrvm.ex.ParserException;
import io.github.leo1024.otrvm.lexer.Token;
import io.github.leo1024.otrvm.parser.impl.*;

import java.util.ArrayList;
import java.util.List;
import java.util.Objects;

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
        final boolean hasFileDef = this.tokenSequence.checkToken(
            it -> it.tokenKindIn(TokenKind.PSEUDO) && it.toPseudo() == Pseudo.FILE);
        final String filename;
        if (hasFileDef) {
            final Token tmp = this.tokenSequence.consume();
            final Object object = Helper.convertLiteral(tmp);
            if (!(object instanceof String)) {
                throw ParserException.create(tmp, "Expected String literal for filename.");
            }
            filename = object.toString();
        } else {
            filename = "??";
        }
        final ASTBuilder context = new ASTBuilder(namespace, filename);
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
                case END -> new EndExpr();
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
            FuncMeta funcMeta = new FuncMeta(true, nameSpaceToken.getValue(), method, Type.VOID, List.of(),
                false, null);
            context.addExpr(new FuncExpr(context, funcMeta));
        }
    }

    private void parseField(Context context) {
        Type type = Helper.expect(this.tokenSequence, TokenKind.TYPE).toType();
        String name = Helper.expect(this.tokenSequence, TokenKind.IDENTIFIER).getValue();
        FieldMeta fieldMeta = new FieldMeta(type, name);
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
        ASTBuilder builder = (ASTBuilder)context;
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
                FuncMeta.Param param = new FuncMeta.Param(paramName.getValue(), paramType.toType());
                paramList.add(param);
                if (!this.tokenSequence.checkToken(token -> token.valEqual(Constants.COMMA))) {
                    emptyParam = true;
                }
            }
            Helper.expect(tokenSequence, Constants.RIGHT_PAREN);
        }
        boolean declareRetType = this.tokenSequence.checkToken(
            token -> token != null && token.valEqual(Constants.COLON));
        Type retType = null;
        if (declareRetType) {
            retType = Helper.expect(tokenSequence, TokenKind.TYPE).toType();
        }
        FuncMeta funcMeta = new FuncMeta(false, null, name.getValue(), retType, paramList, isNativeFunc, libNames);
        FuncExpr func = new FuncExpr(context, funcMeta);
        while (!this.tokenSequence.checkToken(it -> Helper.checkPseudoToken(it, Pseudo.END))) {
            parseExpr(func);
        }

        return func;
    }

    private void convertVMOpts(Context context) {
        VMOption option = VMOption.of(Helper.expect(this.tokenSequence, TokenKind.VM_OPTIONS));
        Object value = Helper.convertLiteral(
            Helper.expect(this.tokenSequence, TokenKind.STRING, TokenKind.INTEGER, TokenKind.HEX, TokenKind.FLOAT));
        context.setOption(option, value);
    }

    private void parseOpcode(final Context context) {
        Token token = this.tokenSequence.consume();
        Opcode opcode = Opcode.of(token);
        Expr expr = switch (opcode) {
            case NEW_ARRAY -> parseTypeExpr(opcode);
            case LI8, LI16, LI32, LI64, LF64 -> parseLoadExpr(opcode);
            case LFIELD, LLOCAL, SFIELD, SLOCAL, SREFDEC, SREFINC, LDC -> parseIndexExpr(opcode);
            case ADD, SUB, MUL, DIV, F2I,
                 F2L, I2L, I2F, L2I, L2F,
                 ICMP, LCMP, DCMP, RET,
                 ISHL, ISHR, IUSHR, LSHL,
                 LSHR, LUSHR, GET_ARRAY, SET_ARRAY, REFINC, REFDEC,
                 ICMP0, ICMP1, ICMPX, LCMP0, LCMP1, LCMPX, DCMP0,
                 DCMP1, DCMPX, LTRUE, LFALSE, IAND, LAND, IOR, LOR,
                 IXOR, LXOR, INOT, LNOT, THROW, I2B, I2S -> new SimpleExpr(opcode);
            case CALL -> parserCallExpr();
            case IINC -> parseIinc();
            case GOTO, IFEQ, IFNE, IFLE, IFGE, IFGT, IFLT, IFTRUE, IFFALSE -> {
                String label = Helper.expect(this.tokenSequence, TokenKind.IDENTIFIER).getValue();
                yield new RedirectExpr(opcode, label);
            }
            default -> throw ParserException.create(token, "Unsupported opcode.");
        };
        context.addExpr(expr);
    }

    private Expr parseIinc() {
        short index = Helper.checkOpcodeIdx(this.tokenSequence);
        Helper.expect(tokenSequence, Constants.COMMA);
        Token token = Helper.expect(this.tokenSequence, TokenKind.INTEGER);
        int value = token.toInt();
        if (value > 127 || value < -128) {
            throw ParserException.create(token, "IINC value must between -128 and 127.");
        }
        return new IincExpr(index, value);
    }

    private Expr parseIndexExpr(final Opcode opcode) {
        return new IndexExpr(opcode, Helper.checkOpcodeIdx(this.tokenSequence));
    }

    private Expr parseLoadExpr(final Opcode opcode) {
        Token token = Helper.requireTokenNotNull(this.tokenSequence.consume(), "Load instruct missing operand.");
        Object value = Helper.convertLiteral(token);
        if (!(value instanceof Number immValue)) {
            throw new ParserException("Load only support number immediate.");
        }
        return new LoadExpr(opcode, immValue);
    }

    private TypeExpr parseTypeExpr(final Opcode opcode) {
        Helper.expect(this.tokenSequence, Constants.DOT);
        Type type = Helper.convertOperandType(this.tokenSequence);
        return new TypeExpr(opcode, type);
    }

    private Expr parserCallExpr() {
        Token methodName = Helper.expect(this.tokenSequence, TokenKind.IDENTIFIER);
        return new CallExpr(methodName.getValue());
    }
}
