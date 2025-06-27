package io.github.leo1024.otrvm.parser.impl;

import io.github.leo1024.otrvm.ISerializable;
import io.github.leo1024.otrvm.conf.Opcode;
import io.github.leo1024.otrvm.parser.Expr;

/**
 * @author yangkui
 * @date 2025/6/27 12:02
 */
public class Simple implements Expr, ISerializable {
    private final Opcode opcode;

    public Simple(Opcode opcode) {
        this.opcode = opcode;
    }

    @Override
    public byte[] toBytes() {
        return new byte[] {opcode.getValue()};
    }
}
