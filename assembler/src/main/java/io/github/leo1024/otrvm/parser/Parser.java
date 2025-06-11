package io.github.leo1024.otrvm.parser;


import io.github.leo1024.otrvm.conf.DataLength;
import io.github.leo1024.otrvm.conf.Opcode;
import io.github.leo1024.otrvm.parser.impl.Imm;

import java.io.InputStream;
import java.util.ArrayList;
import java.util.List;

public class Parser {
    public static List<Expr> parse(InputStream is) {
        var context = new Context(is);
        var list = new ArrayList<Expr>();
        for (; ; ) {
            String text = context.read();
            if (text == null) {
                break;
            }
            var expr = parseIns(context, text);
            list.add(expr);
        }
        return list;
    }


    private static Expr parseIns(final Context context, final String text) {
        var index = text.indexOf(" ");
        final String ins;
        final String params;
        if (index != -1) {
            ins = text.substring(0, index);
            params = text.substring(index + 1);
        } else {
            ins = text;
            params = null;
        }
        Opcode opcode = Opcode.of(ins);
        if (opcode == null) {
            context.parseError("Illegal opcode:%s".formatted(ins));
        }
        DataLength length = null;
        if (opcode == Opcode.PUSH) {
            length = convertDataLength(context, ins);
        }
        return new Expr(opcode, convertISParam(context, params, length));
    }

    private static ISParam convertISParam(final Context context, final String param, final DataLength length) {
        if (param == null) {
            return null;
        }
        if (param.startsWith("#")) {
            return Imm.of(context, param, length);
        }
        return null;
    }

    private static DataLength convertDataLength(final Context context, final String is) {
        int index = is.indexOf(".");
        if (index == -1) {
            return DataLength.INT;
        }
        String text = is.substring(index + 1);
        DataLength length = DataLength.of(text);
        if (length == null) {
            context.parseError("Unknow data suffix:%s".formatted(text));
        }
        return length;
    }
}
