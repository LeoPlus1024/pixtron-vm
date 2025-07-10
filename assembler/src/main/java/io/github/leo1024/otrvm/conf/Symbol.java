package io.github.leo1024.otrvm.conf;

import java.util.Objects;

public record Symbol(int index, String name) {

    @Override
    public String toString() {
        return name;
    }

    public boolean equals(Object obj) {
        if (obj instanceof Symbol symbol) {
            return Objects.equals(symbol.name, name);
        }
        return false;
    }
}
