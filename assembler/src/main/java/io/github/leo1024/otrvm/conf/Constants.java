package io.github.leo1024.otrvm.conf;


public class Constants {
    public static final int EOF = -1;
    public static final char CR = '\r';
    public static final char LF = '\n';
    public static final char TAB = '\t';
    public static final char SPACE = ' ';
    public static final char DOT = '.';
    public static final char COMMA = ',';
    public static final char LEFT_PAREN = '(';
    public static final char RIGHT_PAREN = ')';
    public static final char COMMENT = '#';
    public static final char DOLLAR = '$';
    public static final char AT = '@';
    public static final char ESCAPE = '\\';
    public static final char DOUBLE_QUOTE = '"';
    public static final char COLON = ':';

    public static char[] DELIMITERS = new char[]{
            CR, LF, TAB, SPACE, DOT, COMMA, LEFT_PAREN, RIGHT_PAREN, DOUBLE_QUOTE
    };
}
