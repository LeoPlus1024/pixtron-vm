package io.github.leo1024.otrvm.parser.impl;

import io.github.leo1024.otrvm.parser.Expr;


public class Id implements Expr {
    private final String value;

    public Id(String value) {
        this.value = value;
    }

    public String getValue() {
        return value;
    }
}
