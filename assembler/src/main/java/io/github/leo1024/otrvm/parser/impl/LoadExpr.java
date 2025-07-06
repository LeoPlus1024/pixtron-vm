package io.github.leo1024.otrvm.parser.impl;

import io.github.leo1024.otrvm.conf.Opcode;
import io.github.leo1024.otrvm.conf.Type;
import io.github.leo1024.otrvm.parser.Expr;
import io.github.leo1024.otrvm.util.ByteUtil;


public class LoadExpr implements Expr {
    /**
     * Operand size
     */
    private final Type type;

    private final Opcode opcode;
    /**
     * Immediate value
     */
    private final Number immValue;

    public LoadExpr(final Opcode opcode, Number immValue) {
        this.opcode = opcode;
        this.immValue = immValue;
        this.type = switch (opcode) {
            case LF64 -> Type.DOUBLE;
            case LI8 -> Type.BYTE;
            case LI32 -> Type.INT;
            case LI64 -> Type.LONG;
            case LI16 -> Type.SHORT;
            default -> throw new RuntimeException("Unsupported opcode.");
        };
    }


    @Override
    public byte[] toBytes() {
        byte[] data = new byte[1 + type.getLength()];
        data[0] = opcode.getValue();
        byte[] immBytes = ByteUtil.convertSpecSizeToBytes(this.type, this.immValue);
        System.arraycopy(immBytes, 0, data, 1, immBytes.length);
        return data;
    }

    @Override
    public String toString() {
        return opcode.getMnemonic();
    }
}
