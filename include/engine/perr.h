#ifndef IERROR_H
#define IERROR_H

typedef enum {
    KLASS_DOMAIN = 1000
} VMErrorDomain;

typedef enum {
    MAGIC_ERROR = 2000,
    TYPE_ERROR,
    KLASS_NOT_FOUND,
    METHOD_NOT_FOUND,
    FIELD_NOT_FOUND,
    CONSTRUCTOR_REPEATED,
    LIBRARY_NOT_FOUND,
} VMError;


#endif //IERROR_H
