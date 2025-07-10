package io.github.leo1024.otrvm.conf;

import io.github.leo1024.otrvm.ISerializable;
import io.github.leo1024.otrvm.util.ByteUtil;

public class FieldMeta implements ISerializable {
    private final Type type;
    private final Symbol name;

    public FieldMeta(final Type type, final Symbol name) {
        this.type = type;
        this.name = name;
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
        offset = ByteUtil.appendSymbol(bytes, offset, this.name);

        return bytes;
    }

    public final Symbol getName() {
        return name;
    }

    @Override
    public boolean equals(Object obj) {
        if (!(obj instanceof FieldMeta fieldMeta)) {
            return false;
        }
        if (obj == this) {
            return true;
        }
        return fieldMeta.type == this.type && fieldMeta.name.equals(this.name);
    }
}
