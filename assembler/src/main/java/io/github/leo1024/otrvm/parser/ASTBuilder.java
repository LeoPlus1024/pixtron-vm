package io.github.leo1024.otrvm.parser;

import io.github.leo1024.otrvm.ISerializable;
import io.github.leo1024.otrvm.conf.FuncMeta;
import io.github.leo1024.otrvm.conf.FieldMeta;
import io.github.leo1024.otrvm.ex.ParserException;
import io.github.leo1024.otrvm.parser.impl.FuncExpr;
import io.github.leo1024.otrvm.util.ByteUtil;

import java.nio.charset.StandardCharsets;
import java.util.*;

public class ASTBuilder extends Context {
    public record StrConst(String value) implements ISerializable {

        @Override
        public byte[] toBytes() {
            byte[] strBytes = value.getBytes(StandardCharsets.UTF_8);
            int strBytesLen = strBytes.length;
            // Type + Length + data length
            byte[] bytes = new byte[4 + strBytesLen];
            int pos = ByteUtil.appendInt2Bytes(bytes, 0, strBytesLen);
            System.arraycopy(strBytes, 0, bytes, pos, strBytesLen);
            return bytes;
        }

        @Override
        public boolean equals(Object obj) {
            if (obj instanceof StrConst(String value1)) {
                return value.equals(value1);
            }
            return false;
        }

        @Override
        public String toString() {
            return this.value;
        }
    }

    final Map<String, FieldMeta> fileMetaMap;
    final int filename;
    final int namespace;
    final List<StrConst> constants;
    int library;

    public ASTBuilder(final String namespace, final String filename) {
        super(null);
        this.library = -1;
        this.fileMetaMap = new TreeMap<>();
        this.constants = new ArrayList<>();
        this.filename = this.addConstant(filename);
        this.namespace = this.addConstant(namespace);
    }

    @Override
    public void addField(FieldMeta fieldMeta) {
        final String name = fieldMeta.getName();
        if (fileMetaMap.containsKey(name)) {
            throw new ParserException("Field '" + name + "' already exists.");
        }
        this.fileMetaMap.put(name, fieldMeta);
    }

    public List<FieldMeta> getFieldList() {
        return List.copyOf(this.fileMetaMap.values());
    }

    public List<FuncMeta> getFuncList() {
        return this.getExprList()
                .stream()
                .filter(it -> it instanceof FuncExpr)
                .map(it -> ((FuncExpr) it).getFuncMeta())
                .toList();
    }

    @Override
    public int getNamespace() {
        return namespace;
    }

    @Override
    public int addConstant(String value) {
        StrConst constant = new StrConst(value);
        int index = this.constants.indexOf(constant);
        if (index == -1) {
            index = this.constants.size();
            this.constants.add(constant);
        }
        return index;
    }

    public List<StrConst> getConstants() {
        return Collections.unmodifiableList(this.constants);
    }

    public int getFuncIndex(String funcName) {
        List<FuncMeta> funcList = getFuncList();
        for (int i = 0; i < funcList.size(); i++) {
            FuncMeta funcMeta = funcList.get(i);
            StrConst strConst = getConstant(funcMeta.getName());
            if (strConst.value.equals(funcName)) {
                return i;
            }
        }
        throw new ParserException("Function '" + funcName + "' not found.");
    }

    @Override
    public StrConst getConstant(int index) {
        return this.constants.get(index);
    }

    public void setLibrary(int library) {
        this.library = library;
    }

    public int getLibrary() {
        return library;
    }

    public int getFilename() {
        return filename;
    }
}
