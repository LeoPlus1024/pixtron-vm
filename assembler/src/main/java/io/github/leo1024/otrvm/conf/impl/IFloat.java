package io.github.leo1024.otrvm.conf.impl;

import io.github.leo1024.otrvm.conf.Type;
import io.github.leo1024.otrvm.conf.FieldMeta;

import java.nio.ByteBuffer;
import java.nio.ByteOrder;

public class IFloat extends FieldMeta {
    private final double value;

    public IFloat(String name, double value) {
        super(Type.FLOAT, name);
        this.value = value;
    }

    @Override
    public byte[] getBytes0() {
        return ByteBuffer.allocate(4)
                .order(ByteOrder.LITTLE_ENDIAN)
                .putDouble(this.value)
                .array();
    }
}
