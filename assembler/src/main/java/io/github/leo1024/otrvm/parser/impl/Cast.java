package io.github.leo1024.otrvm.parser.impl;

import io.github.leo1024.otrvm.ISerializable;
import io.github.leo1024.otrvm.conf.Opcode;
import io.github.leo1024.otrvm.parser.Expr;

public class Cast implements Expr, ISerializable {
    private final Opcode opcode;

    public Cast(Opcode opcode) {
        this.opcode = opcode;
    }

    @Override
    public byte[] toBytes() {
        return new byte[]{opcode.getValue()};
    }
}
