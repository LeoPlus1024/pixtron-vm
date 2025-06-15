package io.github.leo1024.otrvm.conf.impl;

import io.github.leo1024.otrvm.conf.Type;
import io.github.leo1024.otrvm.conf.TypeMeta;

import java.nio.ByteBuffer;
import java.nio.ByteOrder;

public class IShort extends TypeMeta {
    private final short value;

    public IShort(String name, short value) {
        super(Type.SHORT, name);
        this.value = value;
    }


    @Override
    public byte[] getBytes0() {
        return ByteBuffer.allocate(2)
                .order(ByteOrder.LITTLE_ENDIAN)
                .putShort(this.value)
                .array();
    }
}
