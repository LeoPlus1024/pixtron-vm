package io.github.leo1024.otrvm.conf;

public record Symbol(int index, String value) {
    public boolean valueEqual(String value) {
        return this.value.equals(value);
    }

    @Override
    public String toString() {
        return value;
    }

    public boolean equals(Object obj) {
        if (obj instanceof Symbol symbol) {
            return symbol.valueEqual(value) && symbol.index() == index;
        }
        return false;
    }
}
