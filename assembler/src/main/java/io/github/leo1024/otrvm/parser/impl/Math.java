package io.github.leo1024.otrvm.parser.impl;

import io.github.leo1024.otrvm.conf.Opcode;
import io.github.leo1024.otrvm.parser.Expr;

public class Math implements Expr {
    private final Opcode opcode;

    public Math(Opcode opcode) {
        this.opcode = opcode;
    }

    @Override
    public byte[] toBytes() {
        return new byte[]{opcode.getValue()};
    }
}
