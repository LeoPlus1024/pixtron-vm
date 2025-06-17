package io.github.leo1024.otrvm.parser;

import io.github.leo1024.otrvm.conf.FuncMeta;
import io.github.leo1024.otrvm.conf.Type;
import io.github.leo1024.otrvm.conf.FieldMeta;
import io.github.leo1024.otrvm.ex.ParserException;
import io.github.leo1024.otrvm.parser.impl.Func;

import java.util.*;

public class ASTBuilder extends Context {
    final Map<String, FieldMeta> typeMetaInfoMap;
    final String namespace;

    public ASTBuilder(final String namespace) {
        super(null);
        this.namespace = namespace;
        this.typeMetaInfoMap = new TreeMap<>();
    }

    @Override
    public void addVar(Type type, String name, Object value) {
        if (typeMetaInfoMap.containsKey(name)) {
            throw new ParserException("Variable '" + name + "' already exists.");
        }
        FieldMeta meta = FieldMeta.of(type, name, value);
        this.typeMetaInfoMap.put(name, meta);
    }

    public List<FieldMeta> getVarList() {
        return List.copyOf(this.typeMetaInfoMap.values());
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
