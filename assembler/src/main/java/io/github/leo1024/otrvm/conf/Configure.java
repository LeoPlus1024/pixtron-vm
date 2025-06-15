package io.github.leo1024.otrvm.conf;

import io.github.leo1024.otrvm.util.OSUtil;

import java.nio.file.Path;
import java.util.Objects;

public class Configure {
    Path workDir;
    Path outputDir;

    public Path getWorkDir() {
        return workDir;
    }

    public Path getOutputDir() {
        return outputDir;
    }

    public static Builder newBuilder() {
        return new Builder();
    }

    public static class Builder {
        final Configure configure;

        public Builder() {
            configure = new Configure();
        }

        public Builder workDir(Path workDir) {
            configure.workDir = workDir;
            return this;
        }

        public Builder outputDir(Path outputDir) {
            configure.outputDir = outputDir;
            return this;
        }

        public Configure build() {
            Objects.requireNonNull(configure.workDir, "workDir can not be null");
            Objects.requireNonNull(configure.outputDir, "outputDir can not be null");
            if (!OSUtil.isDirectory(configure.workDir)) {
                throw new IllegalArgumentException("workDir must be a directory");
            }
            return configure;
        }
    }
}
