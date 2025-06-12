package io.github.leo1024.otrvm.parser;

import io.github.leo1024.otrvm.conf.Opcode;

public class Expr implements Instruction {
    private final Opcode opcode;
    private final ISParam param;

    public Expr(final Opcode opcode, final ISParam param) {
        this.opcode = opcode;
        this.param = param;
    }
}
