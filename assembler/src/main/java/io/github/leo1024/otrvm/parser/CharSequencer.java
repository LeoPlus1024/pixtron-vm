package io.github.leo1024.otrvm.parser;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;


public class CharSequencer {
    private final BufferedReader bufferedReader;
    private int lineNum;
    private String line;
    private boolean cache;

    public CharSequencer(InputStream inputStream) {
        this.lineNum = 0;
        this.cache = false;
        this.bufferedReader = new BufferedReader(new InputStreamReader(inputStream));
    }

    public String read() {
        try {
            if (cache) {
                this.cache = false;
                return this.line;
            }
            this.lineNum++;
            this.line = this.bufferedReader.readLine();
            return this.line;
        } catch (IOException e) {
            throw new RuntimeException(e);
        }
    }

    public int getLineNum() {
        return lineNum;
    }

    public String getLine() {
        return line;
    }

    public void setCache(boolean cache) {
        this.cache = cache;
    }
}
