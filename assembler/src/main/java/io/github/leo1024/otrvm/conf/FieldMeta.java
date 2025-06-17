package io.github.leo1024.otrvm.conf;

import io.github.leo1024.otrvm.ISerializable;
import io.github.leo1024.otrvm.conf.impl.*;
import io.github.leo1024.otrvm.ex.ParserException;
import io.github.leo1024.otrvm.util.ByteUtil;

import java.nio.charset.StandardCharsets;

public abstract class FieldMeta implements ISerializable {
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
        byte[] nameBytes = name.getBytes(StandardCharsets.UTF_8);
        byte[] initValueBytes = getBytes0();

        final int totalLength = 2 + nameBytes.length + 1 + initValueBytes.length;
        byte[] bytes = new byte[totalLength];

        int offset = 0;

        // Writer type id
        offset = ByteUtil.appendType2Bytes(bytes, offset, type);

        // Copy name bytes(without end char)
        System.arraycopy(nameBytes, 0, bytes, offset, nameBytes.length);
        offset += nameBytes.length;

        // Add end char
        bytes[offset++] = '\0';

        // Copy Init value bytes
        System.arraycopy(initValueBytes, 0, bytes, offset, initValueBytes.length);

        return bytes;
    }


    protected abstract byte[] getBytes0();


    public final String getName() {
        return name;
    }

    public static FieldMeta of(final Type type, final String name, Object value) {
        if (!type.isPrimitive()) {
            throw new ParserException("Invalid type.");
        }
        Number number = (Number) value;
        return switch (type) {
            case INT -> new Int(name, number.intValue());
            case LONG -> new ILong(name, number.longValue());
            case SHORT -> new IShort(name, number.shortValue());
            case FLOAT -> new IFloat(name, number.floatValue());
            case BYTE -> new IByte(name, number.byteValue());
            default -> throw new ParserException("Invalid type.");
        };
    }
}
