package io.github.leo1024.otrvm.parser.impl;

import io.github.leo1024.otrvm.conf.FuncMeta;
import io.github.leo1024.otrvm.conf.LabelMeta;
import io.github.leo1024.otrvm.conf.VMOption;
import io.github.leo1024.otrvm.ex.ParserException;
import io.github.leo1024.otrvm.parser.Context;
import io.github.leo1024.otrvm.parser.Expr;

import java.util.HashMap;
import java.util.Map;

public class Func extends Context implements Expr {

    private final FuncMeta funcMeta;
    private final Map<String, LabelMeta> labelMap;

    public Func(final Context parent, final FuncMeta funcMeta) {
        super(parent);
        this.funcMeta = funcMeta;
        this.labelMap = new HashMap<>();
    }

    @Override
    public void addLabel(String label) {
        if (labelMap.containsKey(label)) {
            throw new ParserException("Repeat define same label.");
        }
        LabelMeta meta = new LabelMeta(label, this.getExprListSize());
        this.labelMap.put(label, meta);
    }

    public LabelMeta getLabelMeta(String label) {
        return labelMap.get(label);
    }

    public FuncMeta getFuncMeta() {
        return funcMeta;
    }

    public void checkAndUpdateLabelPos(int index, int position) {
        for (LabelMeta value : this.labelMap.values()) {
            if (value.getIndex() == index) {
                value.setPosition(position);
                break;
            }
        }
    }

    @Override
    public void setOption(VMOption option, Object value) {
        Number number = (Number) value;
        switch (option) {
            case STACK -> funcMeta.setStacks(number.intValue());
            case LOCALS -> funcMeta.setLocals(number.intValue());
            default -> throw new ParserException("Invalid option.");
        }
    }

    @Override
    public String toString() {
        return funcMeta.getFuncName();
    }
}
