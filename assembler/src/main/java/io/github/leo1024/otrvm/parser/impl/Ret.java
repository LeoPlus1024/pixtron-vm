package io.github.leo1024.otrvm.parser.impl;

import io.github.leo1024.otrvm.conf.Opcode;
import io.github.leo1024.otrvm.parser.Expr;

public class Ret implements Expr {
    @Override
    public byte[] toBytes() {
        return new byte[]{Opcode.RET.getValue()};
    }
}
