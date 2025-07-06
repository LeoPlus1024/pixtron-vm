package io.github.leo1024.otrvm.parser.impl;

import io.github.leo1024.otrvm.conf.Opcode;
import io.github.leo1024.otrvm.conf.Type;
import io.github.leo1024.otrvm.parser.Expr;
import io.github.leo1024.otrvm.util.ByteUtil;

public class TypeExpr implements Expr {
    private final Type type;
    private final Opcode opcode;

    public TypeExpr(final Opcode opcode, final Type type) {
        this.type = type;
        this.opcode = opcode;
    }

    @Override
    public byte[] toBytes() {
        byte[] bytes = new byte[3];
        bytes[0] = this.opcode.getValue();
        ByteUtil.appendType2Bytes(bytes, 1, type);
        return bytes;
    }
}
