package io.github.leo1024.otrvm.conf;

import io.github.leo1024.otrvm.ISerializable;
import io.github.leo1024.otrvm.util.ByteUtil;
import io.github.leo1024.otrvm.util.CLanguageUtil;

import java.nio.charset.StandardCharsets;

public class FieldMeta implements ISerializable {
    private final Type type;
    private final String name;

    public FieldMeta(final Type type, final String name) {
        this.type = type;
        this.name = name;
    }

    public final Type getType() {
        return this.type;
    }

    @Override
    public final byte[] toBytes() {
        byte[] nameBytes = CLanguageUtil.toCStyleStr(this.name);

        final int totalLength = 2 + nameBytes.length;
        byte[] bytes = new byte[totalLength];

        int offset = 0;

        // Writer type id
        offset = ByteUtil.appendType2Bytes(bytes, offset, type);

        // Copy name bytes(without end char)
        System.arraycopy(nameBytes, 0, bytes, offset, nameBytes.length);

        return bytes;
    }


    public final String getName() {
        return name;
    }
}
