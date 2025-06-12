package io.github.leo1024.otrvm.parser.impl.pseudo;

import io.github.leo1024.otrvm.parser.Instruction;
import io.github.leo1024.otrvm.parser.Pseudo;

import java.util.List;

public class Func implements Pseudo {
    public static class Param {
        private final String type;
        private final String name;

        public Param(String type) {
            this(type, null);
        }

        public Param(String type, String name) {
            this.type = type;
            this.name = name;
        }
    }

    private final String name;
    private final String type;
    private final List<Param> params;
    private List<Instruction> instructions;

    public Func(String name, String type, List<Param> params) {
        this.name = name;
        this.type = type;
        this.params = params;
    }

    public void setInstructions(List<Instruction> instructions) {
        this.instructions = instructions;
    }
}
