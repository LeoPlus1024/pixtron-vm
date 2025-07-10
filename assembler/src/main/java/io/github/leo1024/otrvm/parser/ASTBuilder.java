package io.github.leo1024.otrvm.parser;

import io.github.leo1024.otrvm.ISerializable;
import io.github.leo1024.otrvm.conf.FuncMeta;
import io.github.leo1024.otrvm.conf.FieldMeta;
import io.github.leo1024.otrvm.conf.Symbol;
import io.github.leo1024.otrvm.ex.ParserException;
import io.github.leo1024.otrvm.parser.impl.FuncExpr;
import io.github.leo1024.otrvm.util.ByteUtil;

import java.nio.charset.StandardCharsets;
import java.util.*;

public class ASTBuilder extends Context {
    public record Const(String value) implements ISerializable {

        @Override
        public byte[] toBytes() {
            byte[] strBytes = value.getBytes(StandardCharsets.UTF_8);
            int strBytesLen = strBytes.length;
            // Length + data length
            byte[] bytes = new byte[4 + strBytesLen];
            int pos = ByteUtil.appendInt2Bytes(bytes, 0, strBytesLen);
            System.arraycopy(strBytes, 0, bytes, pos, strBytesLen);
            return bytes;
        }

        @Override
        public boolean equals(Object obj) {
            if (obj instanceof Const(String value1)) {
                return value.equals(value1);
            }
            return false;
        }

        @Override
        public String toString() {
            return this.value;
        }
    }

    final Symbol filename;
    final Symbol namespace;
    final List<Const> constants;
    final List<FieldMeta> fieldMetaList;
    Symbol library;

    public ASTBuilder(final String namespace, final String filename) {
        super(null);
        this.library = null;
        this.constants = new ArrayList<>();
        this.fieldMetaList = new ArrayList<>();
        this.filename = this.addConstant(filename);
        this.namespace = this.addConstant(namespace);
    }

    @Override
    public void addField(FieldMeta fieldMeta) {
        final Symbol name = fieldMeta.getName();
        if (this.fieldMetaList.contains(fieldMeta)) {
            throw new ParserException("Field '" + name + "' already exists.");
        }
        this.fieldMetaList.add(fieldMeta);
    }

    public List<FieldMeta> getFieldList() {
        return Collections.unmodifiableList(this.fieldMetaList);
    }

    public List<FuncMeta> getFuncList() {
        return this.getExprList()
            .stream()
            .filter(it -> it instanceof FuncExpr)
            .map(it -> ((FuncExpr)it).getFuncMeta())
            .toList();
    }

    @Override
    public Symbol getNamespace() {
        return namespace;
    }

    @Override
    public Symbol addConstant(String value) {
        Const constant = new Const(value);
        int index = this.constants.indexOf(constant);
        if (index == -1) {
            index = this.constants.size();
            this.constants.add(constant);
        }
        return new Symbol(index, value);
    }

    public List<Const> getConstants() {
        return Collections.unmodifiableList(this.constants);
    }

    public int getFuncIndex(String funcName) {
        List<FuncMeta> funcList = getFuncList();
        for (int i = 0; i < funcList.size(); i++) {
            FuncMeta funcMeta = funcList.get(i);
            Symbol symbol = funcMeta.getName();
            if (symbol.valueEqual(funcName)) {
                return i;
            }
        }
        throw new ParserException("Function '" + funcName + "' not found.");
    }

    @Override
    public Const getConstant(int index) {
        return this.constants.get(index);
    }

    public void setLibrary(Symbol library) {
        this.library = library;
    }

    public Symbol getLibrary() {
        return library;
    }

    public Symbol getFilename() {
        return filename;
    }
}
