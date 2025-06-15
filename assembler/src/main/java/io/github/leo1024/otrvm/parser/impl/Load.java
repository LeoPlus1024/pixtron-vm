package io.github.leo1024.otrvm.parser.impl;

import io.github.leo1024.otrvm.conf.DataFrom;
import io.github.leo1024.otrvm.conf.Opcode;
import io.github.leo1024.otrvm.conf.Type;
import io.github.leo1024.otrvm.parser.Expr;
import io.github.leo1024.otrvm.util.ByteUtil;


public class Load implements Expr {
    /**
     * Operand size
     */
    private final Type type;
    /**
     * Data from
     */
    private final DataFrom from;
    /**
     * Immediate value
     */
    private final Number immValue;
    /**
     * Local/Global variable index
     */
    private final int index;

    public Load(Type type, DataFrom from, Number immValue) {
        this.index = 0;
        this.type = type;
        this.from = from;
        this.immValue = immValue;
    }

    public Load(Type type, DataFrom from, Number immValue, int index) {
        this.type = type;
        this.from = from;
        this.immValue = immValue;
        this.index = index;
    }

    @Override
    public byte[] toBytes() {
        int length = 2 + (this.from == DataFrom.IMM ? this.type.getLength() : 2);
        byte[] data = new byte[length];
        data[0] = Opcode.LOAD.getValue();
        data[1] = (byte) (type.getId() << 4 | from.getCode());
        if (this.from == DataFrom.IMM) {
            byte[] immData = ByteUtil.convertSpecSizeToBytes(this.type, this.immValue);
            System.arraycopy(immData, 0, data, 2, immData.length);
        } else {
            System.arraycopy(ByteUtil.short2Bytes((short) index), 0, data, 2, 2);
        }
        return data;
    }
}
