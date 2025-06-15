package io.github.leo1024.otrvm.conf.impl;

import io.github.leo1024.otrvm.conf.Type;
import io.github.leo1024.otrvm.conf.TypeMeta;

import java.nio.ByteBuffer;
import java.nio.ByteOrder;

public class ILong extends TypeMeta {
    private final long value;

    public ILong(String name, long value) {
        super(Type.LONG, name);
        this.value = value;
    }


    @Override
    public byte[] getBytes0() {
        return ByteBuffer.allocate(6)
                .order(ByteOrder.LITTLE_ENDIAN)
                .putLong(this.value)
                .array();
    }
}
