package io.github.leo1024.otrvm.parser.impl;

import io.github.leo1024.otrvm.ISerializable;
import io.github.leo1024.otrvm.conf.Opcode;
import io.github.leo1024.otrvm.parser.Expr;
import io.github.leo1024.otrvm.util.CLanguageUtil;

public class Call implements Expr, ISerializable {
    private final String methodName;

    public Call(final String methodName) {
        this.methodName = methodName;
    }

    public String getMethodName() {
        return methodName;
    }

    @Override
    public byte[] toBytes() {
        byte[] bytes = CLanguageUtil.toCStyleStr(this.methodName);
        byte[] data = new byte[bytes.length + 1];
        data[0] = Opcode.CALL.getValue();
        System.arraycopy(bytes, 0, data, 1, bytes.length);
        return data;
    }
}
