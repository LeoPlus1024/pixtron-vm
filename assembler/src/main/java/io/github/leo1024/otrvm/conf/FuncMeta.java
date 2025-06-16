package io.github.leo1024.otrvm.conf;

import io.github.leo1024.otrvm.ISerializable;
import io.github.leo1024.otrvm.parser.impl.Id;
import io.github.leo1024.otrvm.util.ByteUtil;
import io.github.leo1024.otrvm.util.CLanguageUtil;

import java.util.List;
import java.util.Optional;

public class FuncMeta implements ISerializable {
    public static class Param {
        private final Id name;
        private final Type type;

        public Param(Id name, Type type) {
            this.name = name;
            this.type = type;
        }
    }

    private final Id name;
    private final Type retType;
    private final List<Param> params;
    private final String namespace;
    private int offset;

    public FuncMeta(String namespace, Id name, Type retType, List<Param> params) {
        this.name = name;
        this.retType = retType;
        this.params = params;
        this.namespace = namespace;
    }

    public FuncMeta(Id name, Type retType, List<Param> params) {
        this(null, name, retType, params);
    }

    public int getOffset() {
        return offset;
    }

    public void setOffset(int offset) {
        this.offset = offset;
    }

    @Override
    public byte[] toBytes() {
        // offset(4) + typeId(1)
        int totalLength = 5;
        byte[] nameBytes = CLanguageUtil.toCStyleStr(name.getValue());
        byte[] namespaceBytes = CLanguageUtil.toCStyleStr(Optional.ofNullable(namespace).orElse(""));
        totalLength += nameBytes.length;
        totalLength += namespaceBytes.length;

        // Calculate all params length
        int paramsLength = 0;
        for (Param param : params) {
            // typeId(1) + nameBytes(contain '\0')
            paramsLength += 1 + CLanguageUtil.toCStyleStr(param.name.getValue()).length;
        }

        // alloc precise size
        byte[] data = new byte[totalLength + paramsLength + 2];
        int pos = 0;

        // writer memory offset
        pos = ByteUtil.appendInt2Bytes(data, pos, offset);

        // Writer return type
        data[pos++] = Optional.ofNullable(retType)
                .map(Type::getId)
                .orElse(Type.NIL.getId());

        //Writer namespace
        System.arraycopy(namespaceBytes, 0, data, pos, namespaceBytes.length);
        pos += namespaceBytes.length;

        // Writer func name
        System.arraycopy(nameBytes, 0, data, pos, nameBytes.length);
        pos += nameBytes.length;

        System.arraycopy(ByteUtil.short2Bytes((short) params.size()), 0, data, pos, 2);
        pos += 2;

        // Batch writer params
        for (Param param : params) {
            byte[] paramNameBytes = CLanguageUtil.toCStyleStr(param.name.getValue());
            data[pos++] = param.type.getId();
            System.arraycopy(paramNameBytes, 0, data, pos, paramNameBytes.length);
            pos += paramNameBytes.length;
        }
        return data;
    }
}
