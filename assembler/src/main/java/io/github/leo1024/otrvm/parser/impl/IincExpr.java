package io.github.leo1024.otrvm.parser.impl;

import io.github.leo1024.otrvm.conf.Opcode;
import io.github.leo1024.otrvm.parser.Expr;
import io.github.leo1024.otrvm.util.ByteUtil;

public class IincExpr implements Expr {
    private final int value;
    private final short index;

    public IincExpr(short index, int value) {
        this.index = index;
        this.value = value;
    }

    @Override
    public byte[] toBytes() {
        byte[] bytes = new byte[4];
        bytes[0] = Opcode.IINC.getValue();
        ByteUtil.appendShort2Bytes(bytes, 1, index);
        bytes[3] = (byte) value;
        return bytes;
    }
}