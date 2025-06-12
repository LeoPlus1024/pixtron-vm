package io.github.leo1024.otrvm.parser;

import io.github.leo1024.otrvm.conf.Constants;
import io.github.leo1024.otrvm.parser.impl.pseudo.End;
import io.github.leo1024.otrvm.parser.impl.pseudo.Func;

import java.util.ArrayList;
import java.util.List;

public class Parser {
    public static Pseudo parse(Context context, String text) {
        if (text.equals(Constants._END)) {
            return End.INSTANCE;
        }
        int index = text.indexOf(" ");
        String pseudo = text.substring(0, index);
        String value = text.substring(index).trim();
        return switch (pseudo) {
            case Constants._FUNC -> parseFunc(context, value);
            default -> throw context.parseError("Illegal pseudo define.");
        };
    }

    private static Func parseFunc(Context context, String value) {
        int left = value.indexOf("(");
        if (left == -1) {
            throw context.parseError("Illegal func define.");
        }
        String name = value.substring(0, left);
        int right = value.indexOf(")");
        if (right == -1) {
            throw context.parseError("Illegal func define.");
        }
        final List<Func.Param> paramList = new ArrayList<>();
        if ((right - left) > 1) {
            String text = value.substring(left + 1, right);
            String[] pairs = text.split(",");
            for (String pair : pairs) {
                int index = pair.indexOf(" ");
                final String type;
                final String paramName;
                if (index == -1) {
                    throw context.parseError("Func param name missing.");
                }
                type = pair.substring(0, index);
                paramName = pair.substring(index).trim();

                paramList.add(new Func.Param(type, paramName));
            }
        }
        String retText = value.substring(right).trim();
        final String retValue;
        if (retText.startsWith(":")) {
            retValue = retText.substring(1).trim();
        } else {
            retValue = null;
        }
        return new Func(name, retValue, paramList);
    }


}
