package io.github.leo1024.otrvm.parser.impl;

import io.github.leo1024.otrvm.ISerializable;
import io.github.leo1024.otrvm.conf.Opcode;
import io.github.leo1024.otrvm.parser.Expr;
import io.github.leo1024.otrvm.util.ByteUtil;
import io.github.leo1024.otrvm.util.CLanguageUtil;

public class Call implements Expr, ISerializable {
    private final String methodName;

    public Call(final String methodName) {
        this.methodName = methodName;
    }

    public String getMethodName() {
        return methodName;
    }

    public byte[] toBytes(int index) {
        byte[] data = new byte[3];
        data[0] = Opcode.CALL.getValue();
        ByteUtil.appendShort2Bytes(data, 1, (short) index);
        return data;
    }
}
