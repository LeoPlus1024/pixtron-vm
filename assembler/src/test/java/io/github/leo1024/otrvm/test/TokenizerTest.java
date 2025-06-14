package io.github.leo1024.otrvm.test;

import io.github.leo1024.otrvm.lexer.Token;
import io.github.leo1024.otrvm.lexer.Tokenizer;
import io.github.leo1024.otrvm.parser.Context;
import io.github.leo1024.otrvm.parser.Parser;
import org.junit.jupiter.api.Test;

import java.io.ByteArrayInputStream;
import java.io.InputStream;
import java.nio.charset.StandardCharsets;
import java.util.List;

public class TokenizerTest {
    @Test
    public void test() {
        String text = """
                ; main
                @func main(int i,byte b)
                    %locals 10
                    %stack 10
                    ; load first param
                    load.i $0
                    ; load global variable
                    gload.i $10
                    ; add
                    add
                    ; store first variable slot
                    store.i $0
                @end
                """;
        List<Token> tokens = parserText(text);
        Parser parser = new Parser(tokens);
        Context context = parser.parse();
        System.out.println(context);
    }

    private List<Token> parserText(String text) {
        InputStream is = new ByteArrayInputStream(text.getBytes(StandardCharsets.UTF_8));
        Tokenizer tokenizer = new Tokenizer(is);
        return tokenizer.tokenize();
    }
}
