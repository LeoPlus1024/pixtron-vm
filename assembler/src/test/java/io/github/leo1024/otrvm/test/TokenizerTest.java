package io.github.leo1024.otrvm.test;

import io.github.leo1024.otrvm.lexer.Token;
import io.github.leo1024.otrvm.lexer.Tokenizer;
import org.junit.jupiter.api.Test;

import java.io.ByteArrayInputStream;
import java.io.InputStream;
import java.nio.charset.StandardCharsets;
import java.util.List;

public class TokenizerTest {
    @Test
    public void test() {
        String text = """
                .func main(int,byte)
                    push #10
                    pop $1
                .end
                """;
        List<Token> tokens = parserText(text);
        System.out.println(tokens);
    }

    private List<Token> parserText(String text) {
        InputStream is = new ByteArrayInputStream(text.getBytes(StandardCharsets.UTF_8));
        Tokenizer tokenizer = new Tokenizer(is);
        return tokenizer.tokenize();
    }
}
