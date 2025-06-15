package io.github.leo1024.otrvm.conf.impl;

import io.github.leo1024.otrvm.conf.Type;
import io.github.leo1024.otrvm.conf.TypeMeta;

public class IByte extends TypeMeta {
    private final byte value;

    public IByte(String name, byte value) {
        super(Type.BYTE, name);
        this.value = value;
    }


    @Override
    public byte[] getBytes0() {
        return new byte[]{value};
    }
}
