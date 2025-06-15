package io.github.leo1024.otrvm.test;

import io.github.leo1024.otrvm.lexer.Token;
import io.github.leo1024.otrvm.lexer.Tokenizer;
import io.github.leo1024.otrvm.parser.Context;
import io.github.leo1024.otrvm.parser.Parser;
import org.junit.jupiter.api.Test;

import java.io.IOException;
import java.io.InputStream;
import java.net.URL;
import java.util.List;

public class TokenizerTest {
    @Test
    public void testHelloWorld() {
        List<Token> tokens = parserText("HelloWorld.s");
        Parser parser = new Parser(tokens);
        Context context = parser.parse();
        System.out.println(context);
    }

    private List<Token> parserText(final String fileName) {
        URL url = TokenizerTest.class.getResource(fileName);
        assert url != null;
        try (InputStream is = url.openStream()) {
            Tokenizer tokenizer = new Tokenizer(is);
            return tokenizer.tokenize();
        } catch (IOException e) {
            throw new RuntimeException(e);
        }
    }
}
