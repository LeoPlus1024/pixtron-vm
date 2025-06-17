package io.github.leo1024.otrvm.conf;

public enum Version {
    V1_0(1),
    ;
    private final short version;

    Version(int version) {
        this.version = (short) version;
    }

    public short getVersion() {
        return version;
    }
}
