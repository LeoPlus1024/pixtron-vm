package io.github.leo1024.otrvm.conf;

public enum Version {
    V1_0(1),
    ;
    private final byte version;

    Version(int version) {
        this.version = (byte) version;
    }

    public byte getVersion() {
        return version;
    }
}
