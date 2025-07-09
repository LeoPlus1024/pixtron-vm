package io.github.leo1024.otrvm.conf;

import io.github.leo1024.otrvm.ISerializable;
import io.github.leo1024.otrvm.util.ByteUtil;
import io.github.leo1024.otrvm.util.CLanguageUtil;

import java.nio.charset.StandardCharsets;

public class FieldMeta implements ISerializable {
    private final Type type;
    private final int index;
    private final String name;

    public FieldMeta(final Type type, final int index, final String name) {
        this.type = type;
        this.name = name;
        this.index = index;
    }

    public final Type getType() {
        return this.type;
    }

    @Override
    public final byte[] toBytes() {
        byte[] bytes = new byte[4];

        int offset = 0;

        // Writer type id
        offset = ByteUtil.appendType2Bytes(bytes, offset, type);
        // Writer name index
        offset = ByteUtil.appendShort2Bytes(bytes, offset, (short) index);

        return bytes;
    }


    public final String getName() {
        return name;
    }
}
