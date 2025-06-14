package io.github.leo1024.otrvm.conf;

import java.util.Arrays;

public enum OperandSize {
    BYTE("b"),
    SHORT("s"),
    INT("i"),
    LONG("l"),
    DOUBLE("d");
    private final String suffix;

    OperandSize(String suffix) {
        this.suffix = suffix;
    }

    public String getSuffix() {
        return suffix;
    }

    public static OperandSize of(final String text) {
        return Arrays.stream(values())
                .filter(it -> it.suffix.equals(text))
                .findAny()
                .orElse(null);
    }
}
