package io.github.leo1024.otrvm;

import io.github.leo1024.otrvm.conf.*;
import io.github.leo1024.otrvm.ex.ParserException;
import io.github.leo1024.otrvm.lexer.Token;
import io.github.leo1024.otrvm.lexer.Tokenizer;
import io.github.leo1024.otrvm.parser.Expr;
import io.github.leo1024.otrvm.parser.Parser;
import io.github.leo1024.otrvm.parser.ASTBuilder;
import io.github.leo1024.otrvm.parser.impl.*;

import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.lang.Math;
import java.nio.ByteBuffer;
import java.nio.channels.FileChannel;
import java.util.Arrays;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

public class Assembler {

    private final ASTBuilder builder;

    public Assembler(ASTBuilder builder) {
        this.builder = builder;
    }

    public void assemble(FileOutputStream outputStream) throws IOException {
        final List<FuncMeta> funcMetas = builder.getFuncList();
        final List<TypeMeta> varList = builder.getVarList();
        BinaryHeader header = new BinaryHeader(Version.V1_0, builder.getNamespace(), varList, funcMetas);
        byte[] headerBytes = header.toBytes();
        outputStream.write(headerBytes);
        int offset = headerBytes.length;
        for (Expr expr : builder.getExprList()) {
            if (!(expr instanceof Func func)) {
                throw new ParserException("Only func define in top level.");
            }
            FuncMeta meta = func.getFuncMeta();
            meta.setOffset(offset);
            byte[] bytes = parseFunc(func);
            outputStream.write(bytes);
            offset = offset + bytes.length;
        }
        FileChannel channel = outputStream.getChannel();
        channel.position(0);
        channel.write(ByteBuffer.wrap(header.toBytes()));
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
                buf = redirect.toBytes((short) tmp);
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
            bytes[i] = (byte) tmp;
            bytes[i + 1] = (byte) (tmp >> 8);
        }
        return Arrays.copyOf(bytes, offset);
    }


    public static Assembler create(InputStream inputStream) {
        Tokenizer tokenizer = new Tokenizer(inputStream);
        List<Token> tokenList = tokenizer.tokenize();
        Parser parser = new Parser(tokenList);
        ASTBuilder context = parser.parse();
        return new Assembler(context);
    }
}
