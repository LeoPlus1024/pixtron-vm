package io.github.leo1024.otrvm.lexer;


import io.github.leo1024.otrvm.conf.Constants;
import io.github.leo1024.otrvm.ex.ParserException;

import java.io.IOException;
import java.io.InputStream;
import java.util.Arrays;
import java.util.Objects;

public class CharSequence {
    static final char EOF_CHR = '\u001A';

    final InputStream inputStream;

    char chr;
    boolean eof;
    boolean cached;
    int line;
    int column;


    public CharSequence(InputStream inputStream) {
        Objects.requireNonNull(inputStream, "Input stream must can't null.");
        this.inputStream = inputStream;
        this.line = 1;
        this.column = 0;
        this.chr = EOF_CHR;
    }

    public char read() {
        try {
            return read0();
        } catch (IOException e) {
            throw new ParserException(e);
        }
    }

    /**
     * Reads characters from the input source until encountering a delimiter or end-of-file (EOF).
     * <p>
     * Reading rules:
     * <li>
     * Starts reading from the current character (this.chr)
     * </li>
     * <li>
     * Continuously appends characters until either:
     * </li>
     * <pre>
     * - A delimiter defined in Constants.DELIMITERS is encountered, or
     * - The end of input stream (EOF) is reached
     * </pre>
     * <li>
     * When encountering a delimiter:
     * </li>
     * <pre>
     * - Sets the cached flag to true (indicating the delimiter is cached)
     * - The delimiter is NOT included in the returned string
     * </pre>
     *
     * @return Continuous string of characters (excluding the terminating delimiter)
     */
    public String readUntilDelimiter() {
        return readUtilEncounter(Constants.DELIMITERS);
    }

    public void skipComment() {
        while (!this.isEof()) {
            char chr = this.read();
            if (chr == Constants.LF || chr == Constants.CR) {
                break;
            }
        }
    }

    public String readStrLiteral() {
        byte[] bytes = new byte[1024];
        int index = 0;
        for (; ; ) {
            boolean isEscape = this.chr == Constants.ESCAPE;
            char chr = this.read();
            if (isEof()) {
                throw new ParserException("String literal can't normal end.");
            }
            if (chr == Constants.DOUBLE_QUOTE && !isEscape) {
                break;
            }
            bytes[index++] = (byte) chr;
            if (index == bytes.length) {
                bytes = Arrays.copyOf(bytes, bytes.length * 2);
            }
        }
        return new String(bytes, 0, index);
    }

    public String readHexOrVariable() {
        return readUtilEncounter(Constants.CR, Constants.LF, Constants.SPACE, Constants.TAB, Constants.COMMA);
    }


    private char read0() throws IOException {
        if (cached) {
            this.cached = false;
        } else {
            int value = inputStream.read();
            if (value == Constants.EOF) {
                this.eof = true;
                this.chr = EOF_CHR;
            } else {
                this.eof = false;
                char tmp = (char) value;
                boolean isNewLine = tmp == Constants.CR || (tmp == Constants.LF && this.chr != Constants.CR);
                if (isNewLine) {
                    this.line++;
                    this.column = 0;
                } else {
                    this.column++;
                }
                this.chr = tmp;
            }
        }
        return chr;
    }

    public String readUtilEncounter(char... cc) {
        StringBuilder sb = new StringBuilder();
        sb.append(this.chr);
        loop:
        for (; ; ) {
            char c = read();
            if (isEof()) {
                break;
            }
            for (char chr : cc) {
                if (c == chr) {
                    this.cached = true;
                    break loop;
                }
            }
            sb.append(c);
        }
        return sb.toString();
    }


    public final boolean isEof() {
        return eof;
    }

    public int getLine() {
        return line;
    }

    public int getColumn() {
        return column;
    }
}
