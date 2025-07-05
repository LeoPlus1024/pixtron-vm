package io.github.leo1024.otrvm.parser.impl;

import io.github.leo1024.otrvm.conf.Opcode;
import io.github.leo1024.otrvm.conf.Type;
import io.github.leo1024.otrvm.parser.Expr;
import io.github.leo1024.otrvm.util.ByteUtil;

public class NewArray implements Expr {
    private final Type type;
    private final int length;

    public NewArray(final Type type, final int length) {
        this.type = type;
        this.length = length;
    }

    @Override
    public byte[] toBytes() {
        byte[] bytes = new byte[7];
        bytes[0] = Opcode.NEW_ARRAY.getValue();
        ByteUtil.appendType2Bytes(bytes, 1, type);
        ByteUtil.appendInt2Bytes(bytes, 3, length);
        return bytes;
    }
}
