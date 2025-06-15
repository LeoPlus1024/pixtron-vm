package io.github.leo1024.otrvm.conf;

public class LabelMeta {
    public static final int INVALID_INDEX = -1;
    private final int index;
    private int position;
    private final String label;

    public LabelMeta(String label, int index) {
        this.index = index;
        this.label = label;
        this.position = INVALID_INDEX;
    }

    public int getIndex() {
        return index;
    }

    public int getPosition() {
        return position;
    }

    public void setPosition(int offset) {
        this.position = offset;
    }

    public boolean isInvalid() {
        return position == INVALID_INDEX;
    }

    public String getLabel() {
        return label;
    }
}
