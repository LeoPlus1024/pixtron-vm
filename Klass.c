#include "Klass.h"
#include <gio/gio.h>

#include "Memory.h"
#include "IError.h"

#define MAGIC (0xFFAABBCC)

static void PixtronVM_klass_free(Klass **klass) {
    if (klass == NULL || *klass == NULL) {
        return;
    }
    const Klass *self = *klass;
    PixotronVM_free(TO_REF(self->name));
    PixotronVM_free(TO_REF(self->codes));
    gint vlen = (gint) self->vlen;
    while ((vlen--) >= 0) {
        FieldMeta *fieldMetas = self->fieldMetas + vlen;
        PixotronVM_free(TO_REF(FieldMeta->name));
    }
    PixotronVM_free(TO_REF(self->varr));
    PixotronVM_free(TO_REF(klass));
}

static Klass *PixtronVM_klass_new(const gchar *klassName, GFile *file, GError **error) {
    g_return_val_if_fail(klassName != NULL, NULL);
    g_return_val_if_fail(file != NULL, NULL);
    g_return_val_if_fail(error != NULL && *error == NULL, NULL);
    Klass *klass = NULL;
    GFileInfo *fileInfo = NULL;
    GFileInputStream *inputStream = NULL;
    inputStream = g_file_read(file, NULL, error);
    if (*error != NULL) {
        goto finally;
    }
    fileInfo = g_file_query_info(file, G_FILE_ATTRIBUTE_STANDARD_SIZE, G_FILE_QUERY_INFO_NONE, NULL, &error);
    if (*error != NULL) {
        goto finally;
    }
    const goffset fileSize = g_file_info_get_size(fileInfo);
    guint8 buf[fileSize];
    g_input_stream_read_all(G_INPUT_STREAM(inputStream), buf, fileSize, NULL, NULL, error);
    g_input_stream_close(G_INPUT_STREAM(inputStream), NULL, NULL);
    g_object_unref(inputStream);

    if (*error != NULL) {
        goto finally;
    }
    const guint magic = *((guint *) buf);
    if (magic != MAGIC) {
        g_set_error(error, KLASS_DOMAIN, MAGIC_ERROR, "Magic mistake.");
        goto finally;
    }
    klass = PixotronVM_calloc(sizeof(Klass));
    klass->magic = magic;
    klass->version = buf[4];
    klass->name = g_strdup(klassName);
    gint vlen = *((gint *) (buf + 5));
    gint position = 9;
    FieldMeta *fieldMetas = PixotronVM_calloc(sizeof(FieldMeta) * vlen);
    gint index = 0;
    while (index < vlen) {
        FieldMeta *fileMeta = (fieldMetas + index);
        fileMeta->name = g_strdup((gchar *)(buf + position));
        Type type = *((Type *) (buf + position));
        position = position + 1 + (gint) strlen(variant->name);
        const uint8_t n = TYPE_SIZE[variant->type];
        VMValue value;
        memcpy(&value, buf + position, n);
        position = position + n;
        if(type != DOUBLE) {
            value |= (type <<< 48);
        }
        index++;
    }
    klass->varr = varr;
    klass->vlen = vlen;

finally:
    if (*error != NULL) {
        PixtronVM_klass_free(&klass);
    }
    if (inputStream != NULL) {
        g_input_stream_close(G_INPUT_STREAM(inputStream), NULL, NULL);
        g_clear_object(&inputStream);
    }
    g_clear_object(&fileInfo);
    return klass;
}

static Klass *PixtronVM_Klass_load(const PixtronVM *vm, const gchar *KlassName, GError **error) {
    GFile *dir = NULL;
    Klass *klass = NULL;
    GFile *KlassFile = NULL;
    GFileInfo *fileInfo = NULL;
    GFileEnumerator *enumerator = NULL;

    dir = g_file_new_for_path(vm->workdir);
    enumerator = g_file_enumerate_children(dir,
                                           G_FILE_ATTRIBUTE_STANDARD_NAME,
                                           G_FILE_QUERY_INFO_NONE,
                                           NULL,
                                           &error);
    if (*error != NULL) {
        goto finally;
    }
    while ((fileInfo = g_file_enumerator_next_file(enumerator, NULL, error)) != NULL) {
        const gchar *name = g_file_info_get_name(fileInfo);
        if (!g_str_has_suffix(name, ".clazz")) {
            continue;
        }
        gchar *tmp = g_strndup(name, strlen(name) - 6);
        if (g_str_equal(name, KlassName) == 0) {
            KlassFile = g_file_get_child(dir, name);
            klass = PixtronVM_klass_new(tmp, KlassFile, error);
            if (*error != NULL) {
                goto finally;
            }
            g_hash_table_insert(vm->klassTable, tmp, klass);
            break;
        }
        g_free(tmp);
        g_clear_object(&fileInfo);
    }
finally:
    if (*error != NULL) {
        PixtronVM_klass_free(&klass);
    }
    g_clear_object(&dir);
    g_clear_object(&KlassFile);
    g_clear_object(&fileInfo);
    g_clear_object(&enumerator);

    return klass;
}

extern Klass *PixtronVM_Klass_get(const PixtronVM *vm, const gchar *KlassName) {
    Klass *klass = g_hash_table_lookup(vm->klassTable, KlassName);
    if (klass != NULL) {
        return klass;
    }
    GError *error = NULL;
    klass = PixtronVM_Klass_load(vm, KlassName, &error);
    if (error != NULL || klass == NULL) {
        if (error != NULL) {
            g_printerr("Find Klass:%s fail:%s", KlassName, error->message);
            g_clear_error(&error);
        }
        g_thread_exit(NULL);
    }
    return klass;
}
