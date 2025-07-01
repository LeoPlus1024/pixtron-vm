package io.github.leo1024.otrvm.parser.impl;

import io.github.leo1024.otrvm.conf.Opcode;
import io.github.leo1024.otrvm.conf.Type;
import io.github.leo1024.otrvm.parser.Expr;
import io.github.leo1024.otrvm.util.ByteUtil;

public class NewArray implements Expr {
    private final Type type;

    public NewArray(Type type) {
        this.type = type;
    }

    @Override
    public byte[] toBytes() {
        byte[] bytes = new byte[3];
        bytes[0] = Opcode.NEW_ARRAY.getValue();
        ByteUtil.appendType2Bytes(bytes, 1, type);
        return bytes;
    }
}
