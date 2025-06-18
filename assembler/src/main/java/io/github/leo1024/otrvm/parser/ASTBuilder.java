package io.github.leo1024.otrvm.parser;

import io.github.leo1024.otrvm.conf.FuncMeta;
import io.github.leo1024.otrvm.conf.FieldMeta;
import io.github.leo1024.otrvm.ex.ParserException;
import io.github.leo1024.otrvm.parser.impl.Func;

import java.util.*;

public class ASTBuilder extends Context {
    final Map<String, FieldMeta> fileMetaMap;
    final String namespace;

    public ASTBuilder(final String namespace) {
        super(null);
        this.namespace = namespace;
        this.fileMetaMap = new TreeMap<>();
    }

    @Override
    public void addField(FieldMeta fieldMeta) {
        final String name = fieldMeta.getName();
        if (fileMetaMap.containsKey(name)) {
            throw new ParserException("Variable '" + name + "' already exists.");
        }
        this.fileMetaMap.put(name, fieldMeta);
    }

    public List<FieldMeta> getFieldList() {
        return List.copyOf(this.fileMetaMap.values());
    }

    public List<FuncMeta> getFuncList() {
        return this.getExprList()
                .stream()
                .filter(it -> it instanceof Func)
                .map(it -> ((Func) it).getFuncMeta())
                .toList();
    }

    @Override
    public String getNamespace() {
        return namespace;
    }
}
