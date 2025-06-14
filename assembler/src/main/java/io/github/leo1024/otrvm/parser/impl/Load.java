package io.github.leo1024.otrvm.parser.impl;

import io.github.leo1024.otrvm.conf.DataFrom;
import io.github.leo1024.otrvm.conf.OperandSize;
import io.github.leo1024.otrvm.parser.Expr;

public class Load implements Expr {
    /**
     * Operand size
     */
    private final OperandSize size;
    /**
     * Data from
     */
    private final DataFrom from;
    /**
     * Immediate value
     */
    private final Number immValue;
    /**
     * Local/Global variable index
     */
    private final int index;

    public Load(OperandSize size, DataFrom from, Number immValue) {
        this.index = 0;
        this.size = size;
        this.from = from;
        this.immValue = immValue;
    }

    public Load(OperandSize size, DataFrom from, Number immValue, int index) {
        this.size = size;
        this.from = from;
        this.immValue = immValue;
        this.index = index;
    }
}
