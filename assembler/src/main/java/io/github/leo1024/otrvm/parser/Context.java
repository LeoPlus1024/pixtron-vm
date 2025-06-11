package io.github.leo1024.otrvm.parser;

import io.github.leo1024.otrvm.ex.ParserException;

import java.io.InputStream;
import java.util.HashMap;
import java.util.Map;

public class Context {
    private final CharSequencer charSequencer;
    private final Map<String, Integer> labelTable;

    public Context(final InputStream is) {
        this.labelTable = new HashMap<>();
        this.charSequencer = new CharSequencer(is);
    }

    public String read() {
        var text = this.charSequencer.read();
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

    private void insertLabelTable(String text) {
        var label = text.substring(0, text.length() - 1).trim();
        if (this.labelTable.containsKey(label)) {
            parseError("label %s repeat define.".formatted(label));
        }
        this.labelTable.put(label, this.charSequencer.getLineNum());
    }

    public void parseError(final String text) {
        String line = charSequencer.getLine();
        int lineNum = charSequencer.getLineNum();
        String message = String.format("line:%d %s\n  %s".formatted(lineNum, text, line));
        throw new ParserException(message);
    }
}
