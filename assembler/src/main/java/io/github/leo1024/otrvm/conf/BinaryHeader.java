package io.github.leo1024.otrvm.conf;

import io.github.leo1024.otrvm.ISerializable;
import io.github.leo1024.otrvm.util.ByteUtil;

import java.util.Collections;
import java.util.List;

public class BinaryHeader implements ISerializable {

    static final int MAGIC = 0xFFAABBCC;

    final int magic;
    final Version version;
    final String namespace;
    final List<TypeMeta> varList;
    final List<FuncMeta> funcMetas;

    public BinaryHeader(Version version, String namespace, List<TypeMeta> varList, List<FuncMeta> funcMetas) {
        this.magic = MAGIC;
        this.version = version;
        this.varList = varList;
        this.namespace = namespace;
        this.funcMetas = Collections.unmodifiableList(funcMetas);
    }

    @Override
    public byte[] toBytes() {
        // Magic(4byte)+Version(1byte)
        int length = 5;
        int varSize = this.varList.size();
        byte[][] varArrays = new byte[varSize][];
        int maxVarSize = 0;
        if (varSize > 0) {
            for (int i = 0; i < varSize; i++) {
                byte[] array = varList.get(i).toBytes();
                varArrays[i] = array;
                maxVarSize += array.length;
            }
        }
        int funcSize = this.funcMetas.size();
        byte[][] funcArrays = new byte[funcSize][];
        int maxFuncSize = 0;
        if (funcSize > 0) {
            for (int i = 0; i < funcSize; i++) {
                byte[] array = funcMetas.get(i).toBytes();
                funcArrays[i] = array;
                maxFuncSize += array.length;
            }
        }


        int pos = 0;
        byte[] data = new byte[length + maxFuncSize + maxVarSize + 8];

        pos = ByteUtil.appendInt2Bytes(data, pos, magic);
        data[pos++] = version.getVersion();
        pos = ByteUtil.appendInt2Bytes(data, pos, varSize);

        for (byte[] array : varArrays) {
            System.arraycopy(array, 0, data, pos, array.length);
            pos += array.length;
        }

        pos = ByteUtil.appendInt2Bytes(data, pos, funcSize);

        for (byte[] array : funcArrays) {
            System.arraycopy(array, 0, data, pos, array.length);
            pos += array.length;
        }
        return data;
    }
}
