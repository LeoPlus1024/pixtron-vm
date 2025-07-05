package io.github.leo1024.otrvm.parser.impl;

import io.github.leo1024.otrvm.conf.Opcode;
import io.github.leo1024.otrvm.parser.Expr;
import io.github.leo1024.otrvm.util.ByteUtil;

public class Index implements Expr {
    private final Opcode opcode;
    private final short index;

    public Index(Opcode opcode, short index) {
        this.opcode = opcode;
        this.index = index;
    }

    @Override
    public byte[] toBytes() {
        byte[] bytes = new byte[3];
        bytes[0] = opcode.getValue();
        ByteUtil.appendShort2Bytes(bytes, 1, index);
        return bytes;
    }
}
