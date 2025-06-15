package io.github.leo1024.otrvm.parser;

import io.github.leo1024.otrvm.conf.Type;
import io.github.leo1024.otrvm.conf.VMOption;
import io.github.leo1024.otrvm.ex.ParserException;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

public abstract class Context {
    final Context parent;
    final List<Expr> exprList;
    final Map<VMOption, Object> optionMap;

    public Context(Context parent) {
        this.parent = parent;
        this.optionMap = new HashMap<>();
        this.exprList = new ArrayList<>();
    }

    public void addExpr(Expr expr) {
        this.exprList.add(expr);
    }

    public void setOption(VMOption option, Object value) {
        this.optionMap.put(option, value);
    }

    public void addVar(Type type, String name, Object value) {
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

    public String getNamespace() {
        if (parent != null) {
            return parent.getNamespace();
        }
        return null;
    }
}
