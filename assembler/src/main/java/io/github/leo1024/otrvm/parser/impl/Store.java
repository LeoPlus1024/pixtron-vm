package io.github.leo1024.otrvm.parser.impl;

import io.github.leo1024.otrvm.conf.DataFrom;
import io.github.leo1024.otrvm.conf.Opcode;
import io.github.leo1024.otrvm.conf.Type;
import io.github.leo1024.otrvm.parser.Expr;
import io.github.leo1024.otrvm.util.ByteUtil;

public class Store implements Expr {
    private final DataFrom from;
    private final int index;
    private final Type type;

    public Store(DataFrom from, int index, Type type) {
        this.from = from;
        this.index = index;
        this.type = type;
    }

    @Override
    public byte[] toBytes() {
        byte[] data = new byte[4];
        data[0] = Opcode.STORE.getValue();
        data[1] = (byte) ((type.getId() << 4) | from.getCode());
        byte[] indexData = ByteUtil.short2Bytes((short) this.index);
        data[2] = indexData[0];
        data[3] = indexData[1];
        return data;
    }
}
