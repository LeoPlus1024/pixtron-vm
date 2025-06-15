package io.github.leo1024.otrvm.parser;

import io.github.leo1024.otrvm.ISerializable;
import io.github.leo1024.otrvm.ex.ParserException;

public interface Expr extends ISerializable {
    default byte[] toBytes() {
        throw new ParserException("Not implemented.");
    }
}
