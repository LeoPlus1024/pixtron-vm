package io.github.leo1024.otrvm.test;

import io.github.leo1024.otrvm.Assembler;
import io.github.leo1024.otrvm.conf.Configure;
import org.junit.jupiter.api.Test;

import java.nio.file.Path;

public class AssemblerTest {
    @Test
    public void testHelloWorld() {
        Configure configure = Configure.newBuilder()
                .workDir(Path.of("example"))
                .outputDir(Path.of("example/build"))
                .build();
        Assembler.create(configure);
    }
}
