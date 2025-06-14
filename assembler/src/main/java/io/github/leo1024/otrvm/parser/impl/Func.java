package io.github.leo1024.otrvm.parser.impl;

import io.github.leo1024.otrvm.conf.Type;
import io.github.leo1024.otrvm.parser.Context;
import io.github.leo1024.otrvm.parser.Expr;

import java.util.List;

public class Func extends Context implements Expr {
    public static class Param {
        private final Id name;
        private final Type type;

        public Param(Id name, Type type) {
            this.name = name;
            this.type = type;
        }
    }

    private final Id name;
    private final List<Param> params;
    private final Type retType;

    public Func(final Context parent, Id name, List<Param> params, Type retType) {
        super(parent);
        this.name = name;
        this.params = params;
        this.retType = retType;
    }
}
