package io.github.leo1024.otrvm.parser.impl;

import io.github.leo1024.otrvm.ISerializable;
import io.github.leo1024.otrvm.conf.Opcode;
import io.github.leo1024.otrvm.parser.Expr;

public class XCmp implements Expr, ISerializable {
    private final Opcode opcode;

    public XCmp(Opcode opcode) {
        this.opcode = opcode;
    }

    @Override
    public byte[] toBytes() {
        return new byte[]{this.opcode.getValue()};
    }
}
