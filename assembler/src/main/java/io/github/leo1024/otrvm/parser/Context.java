package io.github.leo1024.otrvm.parser;

import io.github.leo1024.otrvm.conf.VMOption;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

public class Context {
    private final Context parent;
    private final List<Expr> exprList;
    private final Map<VMOption, Object> optionMap;

    public Context(Context parent) {
        this.parent = parent;
        this.optionMap = new HashMap<>();
        this.exprList = new ArrayList<>();
    }

    public Context() {
        this(null);
    }

    public void addExpr(Expr expr) {
        this.exprList.add(expr);
    }

    public void setOption(VMOption option, Object value) {
        this.optionMap.put(option, value);
    }
}
