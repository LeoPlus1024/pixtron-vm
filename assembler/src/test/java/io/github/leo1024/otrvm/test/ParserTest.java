package io.github.leo1024.otrvm.test;

import io.github.leo1024.otrvm.parser.Parser;
import org.junit.jupiter.api.Test;

import java.io.ByteArrayInputStream;
import java.io.InputStream;
import java.nio.charset.StandardCharsets;

public class ParserTest {
    @Test
    public void test() {
        String text = """
                push #10
                pop $1
                """;
        parserText(text);
    }

    private void parserText(String text) {
        InputStream is = new ByteArrayInputStream(text.getBytes(StandardCharsets.UTF_8));
        Parser.parse(is);
    }
}
