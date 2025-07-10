package io.github.leo1024.otrvm.util;

import io.github.leo1024.otrvm.conf.Symbol;
import io.github.leo1024.otrvm.conf.Type;
import io.github.leo1024.otrvm.ex.ParserException;

import java.nio.ByteBuffer;
import java.nio.ByteOrder;

public class ByteUtil {
    public static int appendInt2Bytes(byte[] bytes, int pos, int value) {
        if (pos + 4 > bytes.length) {
            throw new IndexOutOfBoundsException(
                "Can't append to bytes because bytes capacity is %d but it require %d".formatted(bytes.length,
                    pos + 4));
        }
        bytes[pos++] = (byte)value;
        bytes[pos++] = (byte)(value >> 8);
        bytes[pos++] = (byte)(value >> 16);
        bytes[pos++] = (byte)(value >> 24);
        return pos;
    }

    public static int appendType2Bytes(byte[] bytes, int pos, Type type) {
        int value = type.getId();
        return appendShort2Bytes(bytes, pos, (short)value);
    }

    public static int appendShort2Bytes(byte[] bytes, int pos, short value) {
        bytes[pos++] = (byte)value;
        bytes[pos++] = (byte)(value >> 8);
        return pos;
    }

    public static int appendLong2Bytes(byte[] bytes, int pos, long value) {
        byte[] longBytes = ByteUtil.long2Bytes(value);
        System.arraycopy(longBytes, 0, bytes, pos, longBytes.length);
        return pos + longBytes.length;
    }

    public static byte[] long2Bytes(long value) {
        return new byte[] {
            (byte)value,
            (byte)(value >> 8),
            (byte)(value >> 16),
            (byte)(value >> 24),
            (byte)(value >> 32),
            (byte)(value >> 40),
            (byte)(value >> 48),
            (byte)(value >> 56)
        };
    }

    public static byte[] short2Bytes(short value) {
        return new byte[] {
            (byte)value,
            (byte)(value >> 8),
        };
    }

    public static byte[] convertSpecSizeToBytes(Type type, Number imm) {
        return switch (type) {
            case BYTE -> new byte[] {imm.byteValue()};
            case INT -> {
                byte[] bytes = new byte[4];
                appendInt2Bytes(bytes, 0, imm.intValue());
                yield bytes;
            }
            case SHORT -> short2Bytes(imm.shortValue());
            case LONG -> long2Bytes(imm.longValue());
            case DOUBLE -> ByteBuffer.allocate(type.getLength())
                .order(ByteOrder.LITTLE_ENDIAN)
                .putDouble(imm.doubleValue())
                .array();
            default -> throw new ParserException("Invalid literal type.");
        };
    }

    public static int appendSymbol(byte[] bytes, int pos, final Symbol symbol) {
        return appendShort2Bytes(bytes, pos, (short)symbol.index());
    }
}
