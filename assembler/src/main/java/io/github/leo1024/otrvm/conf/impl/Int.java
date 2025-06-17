package io.github.leo1024.otrvm.conf.impl;

import io.github.leo1024.otrvm.conf.Type;
import io.github.leo1024.otrvm.conf.FieldMeta;

import java.nio.ByteBuffer;
import java.nio.ByteOrder;


public class Int extends FieldMeta {
    private final int value;

    public Int(String name, int value) {
        super(Type.INT, name);
        this.value = value;
    }


    @Override
    public byte[] getBytes0() {
        return ByteBuffer.allocate(4)
                .order(ByteOrder.LITTLE_ENDIAN)
                .putInt(this.value)
                .array();
    }
}
