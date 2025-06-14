package io.github.leo1024.otrvm.parser.impl;

import io.github.leo1024.otrvm.conf.DataFrom;
import io.github.leo1024.otrvm.conf.OperandSize;
import io.github.leo1024.otrvm.parser.Expr;

public class Store implements Expr {
    private final DataFrom from;
    private final int index;
    private final OperandSize size;

    public Store(DataFrom from, int index, OperandSize size) {
        this.from = from;
        this.index = index;
        this.size = size;
    }
}
