package io.github.leo1024.otrvm.conf;

public enum DataFrom {
    /**
     * Local
     */
    LC(1),
    /**
     * Field
     */
    GL(2),
    /**
     * Immediate
     */
    IMM(3),
    /**
     * Constant pool
     */
    CONSTANT(4);

    private final byte code;

    DataFrom(int code) {
        this.code = (byte) code;
    }

    public byte getCode() {
        return code;
    }
}
