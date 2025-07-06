package io.github.leo1024.otrvm.parser;

import io.github.leo1024.otrvm.ISerializable;
import io.github.leo1024.otrvm.conf.FuncMeta;
import io.github.leo1024.otrvm.conf.FieldMeta;
import io.github.leo1024.otrvm.conf.Type;
import io.github.leo1024.otrvm.ex.ParserException;
import io.github.leo1024.otrvm.parser.impl.FuncExpr;
import io.github.leo1024.otrvm.util.ByteUtil;

import java.nio.charset.StandardCharsets;
import java.util.*;

public class ASTBuilder extends Context {
    public record Constant(Type type, Object value) implements ISerializable {

        @Override
        public byte[] toBytes() {
            if (type == Type.STRING) {
                byte[] strBytes = ((String) value).getBytes(StandardCharsets.UTF_8);
                int strBytesLen = strBytes.length;
                // Type + Length + data length
                byte[] bytes = new byte[2 + 4 + strBytesLen];
                int pos = ByteUtil.appendType2Bytes(bytes, 0, Type.STRING);
                pos = ByteUtil.appendInt2Bytes(bytes, pos, strBytesLen);
                System.arraycopy(strBytes, 0, bytes, pos, strBytesLen);
                return bytes;
            }
            byte[] bytes = new byte[2 + type.getLength()];
            int pos = ByteUtil.appendType2Bytes(bytes, 0, type);
            byte[] numberBytes = ByteUtil.convertSpecSizeToBytes(type, (Number) value);
            System.arraycopy(numberBytes, 0, bytes, pos, numberBytes.length);
            return bytes;
        }
    }

    final Map<String, FieldMeta> fileMetaMap;
    final String namespace;
    final List<Constant> constants;
    String library;

    public ASTBuilder(final String namespace) {
        super(null);
        this.namespace = namespace;
        this.fileMetaMap = new TreeMap<>();
        this.constants = new ArrayList<>();
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
    public String getNamespace() {
        return namespace;
    }

    @Override
    public void addConstant(Type type, Object value) {
        Constant constant = new Constant(type, value);
        this.constants.add(constant);
    }

    public List<Constant> getConstants() {
        return Collections.unmodifiableList(this.constants);
    }

    public int getFuncIndex(String funcName) {
        List<FuncMeta> funcList = getFuncList();
        for (int i = 0; i < funcList.size(); i++) {
            FuncMeta funcMeta = funcList.get(i);
            if (funcMeta.getFuncName().equals(funcName)) {
                return i;
            }
        }
        throw new ParserException("Function '" + funcName + "' not found.");
    }

    public void setLibrary(String library) {
        this.library = library;
    }

    public String getLibrary() {
        return library;
    }
}
