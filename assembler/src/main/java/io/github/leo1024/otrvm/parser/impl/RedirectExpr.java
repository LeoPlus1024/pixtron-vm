package io.github.leo1024.otrvm.parser.impl;


import io.github.leo1024.otrvm.conf.Opcode;
import io.github.leo1024.otrvm.parser.Expr;

public class RedirectExpr implements Expr {
    private final Opcode opcode;
    private final String label;

    public RedirectExpr(Opcode opcode, String label) {
        this.opcode = opcode;
        this.label = label;
    }

    public String getLabel() {
        return label;
    }

    public byte[] toBytes(short offset) {
        byte[] bytes = new byte[3];
        bytes[0] = opcode.getValue();
        bytes[1] = (byte) (offset);
        bytes[2] = (byte) (offset >>> 8);
        return bytes;
    }

    @Override
    public String toString() {
        return String.format("%s %s", opcode.getMnemonic(), label);
    }
}
