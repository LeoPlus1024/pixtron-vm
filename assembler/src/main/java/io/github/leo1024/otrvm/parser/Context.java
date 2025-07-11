package io.github.leo1024.otrvm.parser;

import io.github.leo1024.otrvm.conf.FieldMeta;
import io.github.leo1024.otrvm.conf.Symbol;
import io.github.leo1024.otrvm.conf.Type;
import io.github.leo1024.otrvm.conf.VMOption;
import io.github.leo1024.otrvm.ex.ParserException;

import java.util.ArrayList;
import java.util.List;

public abstract class Context {
    final Context parent;
    final List<Expr> exprList;

    public Context(Context parent) {
        this.parent = parent;
        this.exprList = new ArrayList<>();
    }

    public void addExpr(Expr expr) {
        this.exprList.add(expr);
    }

    public void setOption(VMOption option, Object value) {
        throw new ParserException("Not support set option.");
    }

    public void addField(FieldMeta fieldMeta) {
        throw new ParserException("Not support add var.");
    }

    public void addLabel(String label) {
        throw new ParserException("Not support add label.");
    }

    public final int getExprListSize() {
        return this.exprList.size();
    }

    public List<Expr> getExprList() {
        return exprList;
    }

    public Symbol getNamespace() {
        throw new ParserException("Not support get namespace.");
    }

    public Symbol addConstant(String value) {
        throw new ParserException("Not support add constant.");
    }

    public ASTBuilder.Const getConstant(int index) {
        throw new ParserException("Not support get constant.");
    }

    public Context getParent() {
        return parent;
    }
}
