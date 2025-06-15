package io.github.leo1024.otrvm.test;

import io.github.leo1024.otrvm.Assembler;
import org.junit.jupiter.api.Test;

import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;

public class AssemblerTest {
    @Test
    public void testHelloWorld() {
        try (InputStream is = AssemblerTest.class.getResourceAsStream("HelloWorld.s");
             FileOutputStream outputStream = new FileOutputStream("test.bin")) {
            Assembler assembler = Assembler.create(is);
            assembler.assemble(outputStream);
        } catch (IOException e) {
            e.printStackTrace();
        }
    }
}
