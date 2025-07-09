package io.github.leo1024.otrvm.conf;

import io.github.leo1024.otrvm.ISerializable;
import io.github.leo1024.otrvm.util.ByteUtil;
import io.github.leo1024.otrvm.util.CLanguageUtil;

import java.util.List;
import java.util.Map;
import java.util.Objects;
import java.util.Optional;

public class FuncMeta implements ISerializable {
    public static class Param {
        final int name;
        final Type type;


        public Param(int name, Type type) {
            this.type = type;
            this.name = name;
        }
    }

    final int name;
    final Type retType;
    final List<Param> params;
    final int namespace;
    final boolean nativeFunc;
    final boolean importFunc;
    int offset;
    // Bytecode size
    int byteCodeSize;
    int locals;
    int stacks;
    Map<Integer, Integer> nameValueMap;

    public FuncMeta(boolean importFunc, int namespace, int name, Type retType, List<Param> params, boolean nativeFunc, Map<Integer, Integer> nameValueMap) {
        this.name = name;
        this.retType = retType;
        this.params = params;
        this.namespace = namespace;
        this.nativeFunc = nativeFunc;
        this.importFunc = importFunc;
        this.nameValueMap = nameValueMap;
    }

    public int getOffset() {
        return offset;
    }

    public void setOffset(int offset) {
        this.offset = offset;
    }

    public void setByteCodeSize(int byteCodeSize) {
        this.byteCodeSize = byteCodeSize;
    }

    public void setLocals(int locals) {
        this.locals = locals;
    }

    public void setStacks(int stacks) {
        this.stacks = stacks;
    }


    @Override
    public byte[] toBytes() {
        if (this.importFunc) {
            byte[] bytes = new byte[4];
            ByteUtil.appendShort2Bytes(bytes, 0, (short) namespace);
            ByteUtil.appendShort2Bytes(bytes, 2, (short) name);
            return bytes;
        }

        int paramsLength = 4 * params.size();
        int nAttrCount = nameValueMap.size();
        // alloc precise size
        byte[] data = new byte[/*Namespace*/2
                // Function name
                + 2
                // Offset(4) +byteCodeSize(4)+ typeId(2)+locals(2)+stacks(2)
                + 14
                // Native flag
                + 1
                // Native attribute count
                + 2
                // Native attribute bytes length
                + nAttrCount * 4
                // Param length and param count
                + paramsLength + 2
                ];
        //Writer namespace
        int pos = ByteUtil.appendShort2Bytes(data, 0, (short) this.namespace);

        // Writer function name
        pos = ByteUtil.appendShort2Bytes(data, pos, (short) this.name);

        // Is native func
        data[pos++] = (byte) (this.nativeFunc ? 1 : 0);
        pos = ByteUtil.appendShort2Bytes(data, pos, (short) nAttrCount);
        for (Map.Entry<Integer, Integer> entry : nameValueMap.entrySet()) {
            pos = ByteUtil.appendShort2Bytes(data, pos, entry.getKey().shortValue());
            pos = ByteUtil.appendShort2Bytes(data, pos, entry.getValue().shortValue());
        }

        // Writer locals
        pos = ByteUtil.appendShort2Bytes(data, pos, (short) this.locals);
        // Writer stacks
        pos = ByteUtil.appendShort2Bytes(data, pos, (short) this.stacks);
        // writer memory offset
        pos = ByteUtil.appendInt2Bytes(data, pos, offset);
        pos = ByteUtil.appendInt2Bytes(data, pos, byteCodeSize);


        // Writer return type
        Type realRetType = Optional.ofNullable(retType).orElse(Type.NIL);
        pos = ByteUtil.appendType2Bytes(data, pos, realRetType);

        pos = ByteUtil.appendShort2Bytes(data, pos, (short) params.size());

        // Batch writer params
        for (Param param : params) {
            pos = ByteUtil.appendType2Bytes(data, pos, param.type);
            pos = ByteUtil.appendShort2Bytes(data, pos, (byte) param.name);
        }
        return data;
    }

    public int getName() {
        return name;
    }
}
