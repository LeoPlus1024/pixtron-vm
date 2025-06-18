package io.github.leo1024.otrvm;

import io.github.leo1024.otrvm.conf.*;
import io.github.leo1024.otrvm.ex.ParserException;
import io.github.leo1024.otrvm.lexer.Token;
import io.github.leo1024.otrvm.lexer.Tokenizer;
import io.github.leo1024.otrvm.parser.Expr;
import io.github.leo1024.otrvm.parser.Parser;
import io.github.leo1024.otrvm.parser.ASTBuilder;
import io.github.leo1024.otrvm.parser.impl.*;
import io.github.leo1024.otrvm.util.OSUtil;

import java.io.*;
import java.lang.Math;
import java.nio.ByteBuffer;
import java.nio.channels.FileChannel;
import java.nio.file.Path;
import java.util.*;

public class Assembler {

    private final ASTBuilder builder;

    private Assembler(ASTBuilder builder) {
        this.builder = builder;
    }

    public File assemble(Path buildDir) throws IOException {
        OSUtil.mkdirs(buildDir);
        final List<FuncMeta> funcMetas = builder.getFuncList();
        final List<FieldMeta> varList = builder.getFieldList();
        BinaryHeader header = new BinaryHeader(Version.V1_0, builder.getNamespace(), varList, funcMetas);
        byte[] headerBytes = header.toBytes();
        Path path = buildDir.resolve(String.format("%s.klass", builder.getNamespace()));
        try (FileOutputStream outputStream = new FileOutputStream(path.toFile())) {
            outputStream.write(headerBytes);
            int offset = 0;
            for (Expr expr : builder.getExprList()) {
                if (!(expr instanceof Func func)) {
                    throw new ParserException("Only func define in top level.");
                }
                FuncMeta meta = func.getFuncMeta();
                byte[] bytes = parseFunc(func);
                outputStream.write(bytes);
                meta.setOffset(offset);
                meta.setByteCodeSize(bytes.length);
                offset = offset + bytes.length;
            }
            FileChannel channel = outputStream.getChannel();
            channel.position(0);
            channel.write(ByteBuffer.wrap(header.toBytes()));
        }
        return path.toFile();
    }

    private byte[] parseFunc(Func func) {
        byte[] bytes = new byte[1024];
        Map<Integer, LabelMeta> labelCorrectMap = new HashMap<>();
        int index = -1;
        int offset = 0;
        for (Expr expr : func.getExprList()) {
            final byte[] buf;
            index++;
            func.checkAndUpdateLabelPos(index, offset);
            if (expr instanceof Redirect redirect) {
                String label = redirect.getLabel();
                LabelMeta meta = func.getLabelMeta(label);
                if (meta == null) {
                    throw new ParserException("Can not find label: " + label);
                }
                if (meta.isInvalid()) {
                    labelCorrectMap.put(offset + 1, meta);
                }
                int tmp = offset - meta.getPosition();
                buf = redirect.toBytes((short)tmp);
            } else {
                buf = expr.toBytes();
            }
            int bufLength = buf.length;
            if (offset + bufLength >= bytes.length) {
                bytes = Arrays.copyOf(bytes, Math.max(offset + 1024, bufLength + offset));
            }
            System.arraycopy(buf, 0, bytes, offset, buf.length);
            offset += bufLength;
        }
        for (Map.Entry<Integer, LabelMeta> entry : labelCorrectMap.entrySet()) {
            LabelMeta meta = entry.getValue();
            if (meta.isInvalid()) {
                throw new ParserException("Can't find proper label position for label:%s".formatted(meta.getLabel()));
            }
            int i = entry.getKey();
            int tmp = meta.getPosition() - i;
            bytes[i] = (byte)tmp;
            bytes[i + 1] = (byte)(tmp >> 8);
        }
        return Arrays.copyOf(bytes, offset);
    }

    public static List<File> create(Configure configure) {
        Path workDir = configure.getWorkDir();
        File file = workDir.toFile();
        List<File> outputFiles = new ArrayList<>();
        for (File element : file.listFiles()) {
            if (!element.canRead() || element.isDirectory()) {
                continue;
            }
            boolean isByteCodeFile = element.getName().toLowerCase().endsWith(".s");
            if (!isByteCodeFile) {
                continue;
            }
            try (FileInputStream inputStream = new FileInputStream(element)) {
                Tokenizer tokenizer = new Tokenizer(inputStream);
                List<Token> tokenList = tokenizer.tokenize();
                Parser parser = new Parser(tokenList);
                ASTBuilder context = parser.parse();
                Assembler assembler = new Assembler(context);
                File outputFile = assembler.assemble(configure.getOutputDir());
                outputFiles.add(outputFile);
            } catch (IOException e) {
                throw new ParserException(e);
            }
        }
        return outputFiles;
    }
}
