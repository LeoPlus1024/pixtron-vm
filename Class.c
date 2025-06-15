#include "Class.h"
#include <gio/gio.h>

#include "Memory.h"

#define MAGIC (0xFFAABBCC)

static Class *PixtronVM_class_new(const PixtronVM *vm, const gchar *className, GFile *file) {
    GError *error = NULL;
    GFileInputStream *inputStream = g_file_read(file,NULL, &error);
    if (error != NULL) {
        g_printerr("Open class file fail:%s.", error->message);
        g_error_free(error);
        exit(-1);
    }
    GFileInfo *fileInfo = g_file_query_info(file,G_FILE_ATTRIBUTE_STANDARD_SIZE, G_FILE_QUERY_INFO_NONE, NULL, &error);
    if (error != NULL) {
        g_printerr("Can't get file size: %s\n", error->message);
        g_error_free(error);
        g_input_stream_close(G_INPUT_STREAM(inputStream), NULL, NULL);
        g_object_unref(inputStream);
        exit(-1);
    }
    const goffset fileSize = g_file_info_get_size(fileInfo);
    guint8 buf[fileSize];
    g_input_stream_read_all(G_INPUT_STREAM(inputStream), buf, fileSize, NULL,NULL, &error);
    g_input_stream_close(G_INPUT_STREAM(inputStream), NULL, NULL);
    g_object_unref(inputStream);

    if (error != NULL) {
        g_printerr("Read class file fail:%s.", error->message);
        g_error_free(error);
        exit(-1);
    }
    const guint magic = *((guint *) buf);
    if (magic != MAGIC) {
        g_printerr("Class magic compare faile.");
        exit(-1);
    }
    Class *class = PixotronVM_calloc(sizeof(Class));
    class->magic = magic;
    class->version = buf[4];
    class->className = g_strdup(className);
    gint vlen = *((gint *) (buf + 5));
    gint position = 9;
    Variant *varr = PixotronVM_calloc(sizeof(Variant) * vlen);
    gint index = 0;
    while (index < vlen) {
        Variant *variant = (varr + index);
        variant->name = g_strdup((gchar*)(buf+position));
        variant->type = *((Type *) (buf + position));
        position = position + 1 + (gint) strlen(variant->name);
        const uint8_t n = TYPE_SIZE[variant->type];
        memcpy(&variant->value, buf + position, n);
        position = position + n;
        index++;
    }
    class->varr = varr;
    class->vlen = vlen;

    return class;
}

static Class *PixtronVM_class_load(const PixtronVM *vm, const gchar *className) {
    GFile *dir = g_file_new_for_path(vm->workdir);
    GError *error = NULL;
    GFileEnumerator *enumerator = g_file_enumerate_children(dir,
                                                            G_FILE_ATTRIBUTE_STANDARD_NAME,
                                                            G_FILE_QUERY_INFO_NONE,
                                                            NULL,
                                                            &error);
    if (error != NULL) {
        g_printerr("Work directory can't reach.");
        g_error_free(error);
        g_object_unref(dir);
        exit(-1);
    }
    GFileInfo *fileInfo;
    Class *class = NULL;
    while ((fileInfo = g_file_enumerator_next_file(enumerator, NULL, &error)) != NULL) {
        const gchar *name = g_file_info_get_name(fileInfo);
        if (!g_str_has_suffix(name, ".clazz")) {
            continue;
        }
        gchar *tmp = g_strndup(name, strlen(name) - 6);
        if (g_str_equal(name, className) == 0) {
            GFile *classFile = g_file_get_child(dir, name);
            class = PixtronVM_class_new(vm, tmp, classFile);
            g_hash_table_insert(vm->classes, tmp, class);
            g_object_unref(classFile);
            g_object_unref(fileInfo);
            break;
        } else {
            g_free(tmp);
        }
        g_object_unref(fileInfo);
    }
    g_object_unref(enumerator);
    if (class == NULL) {
        g_printerr("Class %s not found.", className);
        exit(-1);
    }
    return class;
}

extern Class *PixtronVM_class_get(const PixtronVM *vm, const gchar *className) {
    Class *class = g_hash_table_lookup(vm->classes, className);
    if (class != NULL) {
        return class;
    }
    return PixtronVM_class_load(vm, className);
}
