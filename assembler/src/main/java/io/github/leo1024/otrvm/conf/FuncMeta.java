package io.github.leo1024.otrvm.conf;

import io.github.leo1024.otrvm.ISerializable;
import io.github.leo1024.otrvm.parser.impl.Id;
import io.github.leo1024.otrvm.util.ByteUtil;
import io.github.leo1024.otrvm.util.CLanguageUtil;

import java.util.List;
import java.util.Objects;
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
    private final String libNames;
    private final boolean nativeFunc;
    private int offset;
    // Bytecode size
    private int byteCodeSize;
    private int locals;
    private int stacks;


    public FuncMeta(String namespace, Id name, Type retType, List<Param> params, boolean nativeFunc, String libNames) {
        this.name = name;
        this.retType = retType;
        this.params = params;
        this.namespace = namespace;
        this.libNames = libNames;
        this.nativeFunc = nativeFunc;
    }

    public FuncMeta(Id name, Type retType, List<Param> params) {
        this(null, name, retType, params, false, null);
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
        // offset(4) +byteCodeSize(4)+ typeId(2)+locals(2)+stacks(2)
        int totalLength = 14;
        byte[] nameBytes = CLanguageUtil.toCStyleStr(name.getValue());
        byte[] namespaceBytes = CLanguageUtil.toCStyleStr(Optional.ofNullable(namespace).orElse(""));
        totalLength += nameBytes.length;
        totalLength += namespaceBytes.length;

        // Calculate all params length
        int paramsLength = 0;
        for (Param param : params) {
            // typeId(2) + nameBytes(contain '\0')
            paramsLength += (2 + CLanguageUtil.toCStyleStr(param.name.getValue()).length);
        }

        int libNameLen = 0;
        byte[] libNameBytes = null;
        if (this.nativeFunc) {
            final String tmp;
            tmp = Objects.requireNonNullElse(libNames, "");
            libNameBytes = CLanguageUtil.toCStyleStr(tmp);
            libNameLen = libNameBytes.length;
        }

        // alloc precise size
        byte[] data = new byte[1 + totalLength + libNameLen + paramsLength + 2];
        int pos = 0;
        // Is native func
        data[pos++] = (byte) (this.nativeFunc ? 1 : 0);
        if (this.nativeFunc) {
            System.arraycopy(libNameBytes, 0, data, pos, libNameLen);
            pos += libNameLen;
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
            pos = ByteUtil.appendType2Bytes(data, pos, param.type);
            System.arraycopy(paramNameBytes, 0, data, pos, paramNameBytes.length);
            pos += paramNameBytes.length;
        }
        return data;
    }
}
