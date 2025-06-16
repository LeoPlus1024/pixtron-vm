#ifndef IERROR_H
#define IERROR_H

typedef enum {
    KLASS_DOMAIN = 1000
} VMErrorDomain;

typedef enum {
    MAGIC_ERROR = 2000,
    TYPE_ERROR,
} VMError;


#endif //IERROR_H
