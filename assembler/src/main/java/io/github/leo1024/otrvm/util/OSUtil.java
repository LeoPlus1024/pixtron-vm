package io.github.leo1024.otrvm.util;

import java.io.File;
import java.nio.file.Path;
import java.nio.file.Paths;

public class OSUtil {
    public static boolean isDirectory(Path path) {
        if (path == null) {
            return false;
        }
        File file = path.toFile();
        return file.isDirectory();
    }

    public static boolean mkdirs(Path path) {
        if (path == null) {
            return false;
        }
        File file = path.toFile();
        return file.mkdirs();
    }
}
