package io.github.leo1024.otrvm.parser;

import io.github.leo1024.otrvm.ex.ParserException;

import java.io.InputStream;
import java.util.HashMap;
import java.util.Map;
import java.util.function.Function;

public class Context {
    private final CharSequencer charSequencer;
    private final Map<String, Integer> labelTable;

    public Context(final InputStream is) {
        this.labelTable = new HashMap<>();
        this.charSequencer = new CharSequencer(is);
    }

    public String read() {
        String text = this.charSequencer.read();
        if (text == null) {
            return null;
        }
        text = text.trim();
        if (text.endsWith(":")) {
            this.insertLabelTable(text);
            return read();
        }
        return text;
    }

    public boolean checkNextLine(Function<String, Boolean> func) {
        String text = this.read();
        if (text == null) {
            return false;
        }
        boolean match = func.apply(text);
        if (!match) {
            this.charSequencer.setCache(true);
        }
        return match;
    }

    private void insertLabelTable(String text) {
        String label = text.substring(0, text.length() - 1).trim();
        if (this.labelTable.containsKey(label)) {
            throw parseError("label %s repeat define.".formatted(label));
        }
        this.labelTable.put(label, this.charSequencer.getLineNum());
    }

    public ParserException parseError(final String text) {
        String line = charSequencer.getLine();
        int lineNum = charSequencer.getLineNum();
        String message = String.format("line:%d %s\n> %s".formatted(lineNum, text, line));
        return new ParserException(message);
    }
}
