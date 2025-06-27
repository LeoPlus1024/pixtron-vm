package io.github.leo1024.otrvm.conf;

import io.github.leo1024.otrvm.ISerializable;
import io.github.leo1024.otrvm.parser.ASTBuilder;
import io.github.leo1024.otrvm.util.ByteUtil;
import io.github.leo1024.otrvm.util.CLanguageUtil;

import java.util.Collections;
import java.util.List;

public class BinaryHeader implements ISerializable {

    static final int MAGIC = 0xFFAABBCC;

    final int magic;
    final Version version;
    final List<FieldMeta> fieldMetas;
    final List<FuncMeta> funcMetas;
    final ASTBuilder builder;

    public BinaryHeader(Version version, ASTBuilder builder, List<FieldMeta> fieldMetas, List<FuncMeta> funcMetas) {
        this.magic = MAGIC;
        this.builder = builder;
        this.version = version;
        this.fieldMetas = fieldMetas;
        this.funcMetas = Collections.unmodifiableList(funcMetas);
    }

    @Override
    public byte[] toBytes() {
        // Magic(4byte)+Version(2byte)
        int length = 6;
        int fieldSize = this.fieldMetas.size();
        byte[][] fieldBytes = new byte[fieldSize][];
        int maxFieldSize = 0;
        if (fieldSize > 0) {
            for (int i = 0; i < fieldSize; i++) {
                byte[] array = fieldMetas.get(i).toBytes();
                fieldBytes[i] = array;
                maxFieldSize += array.length;
            }
        }
        int funcSize = this.funcMetas.size();
        byte[][] funcBytes = new byte[funcSize][];
        int maxFuncSize = 0;
        if (funcSize > 0) {
            for (int i = 0; i < funcSize; i++) {
                byte[] array = funcMetas.get(i).toBytes();
                funcBytes[i] = array;
                maxFuncSize += array.length;
            }
        }
        final List<ASTBuilder.Constant> constants = builder.getConstants();
        final byte[][] constBytes = new byte[constants.size()][];
        int maxConstSize = 0;
        // Writer constant data
        for (int i = 0; i < constBytes.length; i++) {
            byte[] bytes = constants.get(i).toBytes();
            maxConstSize += bytes.length;
            constBytes[i] = bytes;
        }
        int libraryLength = 0;
        byte[] libraryBytes = null;
        if (builder.getLibrary() != null) {
            libraryBytes = CLanguageUtil.toCStyleStr(builder.getLibrary());
            libraryLength = libraryBytes.length;
        }
        int pos = 0;
        int constSize = constants.size();
        byte[] data = new byte[length + maxFuncSize + maxFieldSize
            + 8
            // Constant size
            + 2
            // library flag
            + 1
            + maxConstSize
            + libraryLength
            ];

        pos = ByteUtil.appendInt2Bytes(data, pos, magic);
        pos = ByteUtil.appendShort2Bytes(data, pos, version.getVersion());
        data[pos++] = libraryLength == 0 ? (byte)0 : (byte)1;
        if (libraryLength > 0) {
            System.arraycopy(libraryBytes, 0, data, pos, libraryLength);
            pos += libraryLength;
        }
        pos = ByteUtil.appendShort2Bytes(data, pos, (short)constSize);

        for (byte[] constByte : constBytes) {
            System.arraycopy(constByte, 0, data, pos, constByte.length);
            pos += constByte.length;
        }

        pos = ByteUtil.appendInt2Bytes(data, pos, fieldSize);

        for (byte[] array : fieldBytes) {
            System.arraycopy(array, 0, data, pos, array.length);
            pos += array.length;
        }

        pos = ByteUtil.appendInt2Bytes(data, pos, funcSize);

        for (byte[] array : funcBytes) {
            System.arraycopy(array, 0, data, pos, array.length);
            pos += array.length;
        }
        return data;
    }
}
