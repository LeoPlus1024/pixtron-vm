package io.github.leo1024.otrvm.parser.impl;

import io.github.leo1024.otrvm.ISerializable;
import io.github.leo1024.otrvm.conf.Opcode;
import io.github.leo1024.otrvm.parser.Expr;
import io.github.leo1024.otrvm.util.ByteUtil;

public class AssertExpr implements Expr, ISerializable {
    final int index;

    public AssertExpr(int index) {
        this.index = index;
    }

    @Override
    public byte[] toBytes() {
        byte[] bytes = new byte[3];
        bytes[0] = Opcode.ASSERT.getValue();
        ByteUtil.appendShort2Bytes(bytes, 1, (short) index);
        return bytes;
    }
}
