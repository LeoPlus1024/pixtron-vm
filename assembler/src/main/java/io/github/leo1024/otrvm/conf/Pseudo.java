package io.github.leo1024.otrvm.conf;

import java.util.Arrays;

public enum Pseudo {
    FUNC("@func"),
    END("@end"),
    FIELD("@field"),
    IMPORT("@import"),
    NATIVE("@native"),
    CONSTANT("@constant"),
    NAMESPACE("@namespace"),
    LIBRARY("@library"),
    ;
    private final String code;

    Pseudo(String code) {
        this.code = code;
    }

    public String getCode() {
        return code;
    }

    public static Pseudo of(String code) {
        return Arrays.stream(values())
                .filter(it -> it.getCode().equals(code))
                .findAny()
                .orElse(null);
    }
}
