package io.github.leo1024.otrvm.util;

import java.nio.charset.StandardCharsets;

public class CLanguageUtil {
    public static final char C_STR_END = '\0';

    public static byte[] toCStyleStr(String value) {
        byte[] bytes = value.getBytes(StandardCharsets.UTF_8);
        int length = bytes.length;
        byte[] data = new byte[length + 1];
        System.arraycopy(bytes, 0, data, 0, length);
        data[length] = C_STR_END;
        return data;
    }
}
