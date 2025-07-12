#include "pcore.h"

#include <assert.h>
#include <config.h>
#include <stdbool.h>

#include "pbcread.h"
#include "pklass.h"
#include "pmem.h"
#include "pstack.h"
#include <dlfcn.h>

#include "phelper.h"
#include "pstr.h"
#include "parr.h"
#include "pobj.h"
#include <glib.h>
#if VM_DEBUG_ENABLE
#include "op_gen.h"
#endif


static GPrivate pri_key = G_PRIVATE_INIT(g_free);

static inline void pvm_add(RuntimeContext *context) {
    const VMValue *source_operand = pvm_pop_operand(context);
    VMValue *target_operand = pvm_get_operand(context);
    APPLY_COMPOUND_OPERATOR(target_operand, source_operand, +, context);
}

static inline void pvm_sub(RuntimeContext *context) {
    const VMValue *source_operand = pvm_pop_operand(context);
    VMValue *target_operand = pvm_get_operand(context);
    APPLY_COMPOUND_OPERATOR(target_operand, source_operand, -, context);
}

static inline void pvm_mul(RuntimeContext *context) {
    const VMValue *source_operand = pvm_pop_operand(context);
    VMValue *target_operand = pvm_get_operand(context);
    APPLY_COMPOUND_OPERATOR(target_operand, source_operand, *, context);
}

static inline void pvm_div(RuntimeContext *context) {
    const VMValue *source_operand = pvm_pop_operand(context);
    VMValue *target_operand = pvm_get_operand(context);
    const Type t1 = source_operand->type;
    if (TYPE_SMALL_INTEGER(t1) && source_operand->i32 == 0 || TYPE_BIGGER_INTEGER(t1) && source_operand->i64 == 0) {
        context->throw_exception(context, "Divisor cannot be zero.");
    }
    APPLY_COMPOUND_OPERATOR(target_operand, source_operand, /, context);
}

static inline void pvm_goto(RuntimeContext *context) {
    VirtualStackFrame *frame = context->frame;
    const int16_t offset = pvm_bytecode_read_int16(context);
    const uint32_t pc = frame->pc;
    frame->pc = pc - offset - 3;
}


static inline void pvm_i2f_l2f(RuntimeContext *context) {
    VMValue *value = pvm_get_operand(context);
    switch (value->type) {
#define HANDLE_CASE(type,field) \
case type: value->f64 = (double)(value->field);  break;
        HANDLE_CASE(TYPE_INT, i32)
        HANDLE_CASE(TYPE_LONG, i64)
        default: assert(0);
#undef HANDLE_CASE
    }
    value->type = TYPE_DOUBLE;
}

static inline void pvm_i2l_f2l(RuntimeContext *context) {
    VMValue *value = pvm_get_operand(context);
    switch (value->type) {
#define HANDLE_CASE(type,field) \
case type: value->i64 = (int64_t)(value->field);  break;
        HANDLE_CASE(TYPE_SHORT, i32);
        HANDLE_CASE(TYPE_DOUBLE, f64);
        default: assert(0);
#undef HANDLE_CASE
    }
    value->type = TYPE_LONG;
}

static inline void pvm_f2i_l2i(RuntimeContext *context) {
    VMValue *value = pvm_get_operand(context);
    switch (value->type) {
#define HANDLE_CASE(type,field) \
case type: value->i32 = (int32_t)(value->field);  break;
        HANDLE_CASE(TYPE_LONG, i64);
        HANDLE_CASE(TYPE_DOUBLE, f64);
        default: assert(0);
#undef HANDLE_CASE
    }
    value->type = TYPE_INT;
}


static inline void pvm_slocal(RuntimeContext *context) {
    const VMValue *value = pvm_pop_operand(context);
    const uint16_t index = pvm_bytecode_read_int16(context);
    pvm_set_local_value(context, index, value);
}

static inline void pvm_llocal(RuntimeContext *context) {
    const uint16_t index = pvm_bytecode_read_int16(context);
    VMValue *value = pvm_next_operand(context);
    pvm_copy_local_value(context, index, value);
}


static inline void pvm_lint8(RuntimeContext *context) {
    VMValue *value = pvm_next_operand(context);
    value->i8 = pvm_bytecode_read_int8(context);
    value->type = TYPE_BYTE;
}

static inline void pvm_lint16(RuntimeContext *context) {
    VMValue *value = pvm_next_operand(context);
    value->i16 = pvm_bytecode_read_int16(context);
    value->type = TYPE_SHORT;
}

static inline void pvm_lint32(RuntimeContext *context) {
    VMValue *value = pvm_next_operand(context);
    value->i32 = pvm_bytecode_read_int32(context);
    value->type = TYPE_INT;
}

static inline void pvm_lint64(RuntimeContext *context) {
    VMValue *value = pvm_next_operand(context);
    value->i64 = pvm_bytecode_read_int64(context);
    value->type = TYPE_LONG;
}

static inline void pvm_lf64(RuntimeContext *context) {
    VMValue *value = pvm_next_operand(context);
    value->f64 = pvm_bytecode_read_f64(context);
    value->type = TYPE_DOUBLE;
}


static inline VMValue *pvm_ret(RuntimeContext *context, VirtualStackFrame *call_frame) {
    const VirtualStackFrame *frame = context->frame;
    const VMValue *value = NULL;
    const bool hasRetVal = frame->method->ret != TYPE_VOID;
    if (hasRetVal) {
        value = pvm_pop_operand(context);
    }
    VMValue *ret_val = NULL;
    pvm_pop_stack_frame(context);
    if (frame->pre == call_frame) {
        context->exit = exit;
        ret_val = pvm_mem_cpy(value,VM_VALUE_SIZE);
    } else if (value != NULL) {
        pvm_push_operand(context, value);
    }
    return ret_val;
}

static inline void pvm_call(RuntimeContext *context) {
    const uint16_t index = pvm_bytecode_read_int16(context);
    const Method *method = pvm_get_method(context, index);
    if (method->native_func) {
        pvm_ffi_call(context, method);
    } else {
#if VM_DEBUG_ENABLE
        const Klass *klass = method->klass;
        if (klass->constructor == method) {
            context->throw_exception(
                context,
                "Class initialization methods cannot be called directly by user code. They are automatically invoked by the virtual machine during class loading and execute only once per class lifetime.");
        }
#endif
        pvm_push_stack_frame(context, method);
    }
}

static void inline pvm_throw(RuntimeContext *context) {
    const VMValue *value = pvm_pop_operand(context);
#if VM_DEBUG_ENABLE
    if (value->type != TYPE_STRING) {
        context->throw_exception(context, "Throwable must be a string (got %s)", TYPE_NAME[value->type]);
    }
#endif
    const PStr *pstr = value->obj;
    context->throw_exception(context, pstr->value);
}

static void inline pvm_ishr(const RuntimeContext *context) {
    const VMValue *source_operand = pvm_pop_operand(context);
    VMValue *target_operand = pvm_get_operand(context);
    const int32_t bit = source_operand->i32 & 0x1F;
    target_operand->i32 = target_operand->i32 >> bit;
}

static void inline pvm_iushr(RuntimeContext *context) {
    const VMValue *source_operand = pvm_pop_operand(context);
    VMValue *target_operand = pvm_get_operand(context);
    const int32_t bit = source_operand->i32 & 0x1F;
    target_operand->i32 = target_operand->i32 << bit;
}

static void inline pvm_ishl(RuntimeContext *context) {
    const VMValue *source_operand = pvm_pop_operand(context);
    VMValue *target_operand = pvm_get_operand(context);
    const int32_t bit = source_operand->i32 & 0x1F;
    const uint32_t value = (uint32_t) target_operand->i32;
    target_operand->i32 = (int32_t) (value >> bit);
}

static void inline pvm_lshr(RuntimeContext *context) {
    const VMValue *source_operand = pvm_pop_operand(context);
    VMValue *target_operand = pvm_get_operand(context);
    const int32_t bit = source_operand->i32 & 0x1F;
    target_operand->i64 = target_operand->i64 >> bit;
}

static void inline pvm_lushr(RuntimeContext *context) {
    const VMValue *source_operand = pvm_pop_operand(context);
    VMValue *target_operand = pvm_get_operand(context);
    const int32_t bit = source_operand->i32 & 0x1F;
    target_operand->i64 = target_operand->i64 << bit;
}

static void inline pvm_lshl(RuntimeContext *context) {
    const VMValue *source_operand = pvm_pop_operand(context);
    VMValue *target_operand = pvm_get_operand(context);
    const int32_t bit = source_operand->i32 & 0x1F;
    const uint32_t value = (uint32_t) target_operand->i64;
    target_operand->i64 = (int32_t) (value >> bit);
}


static inline RuntimeContext *pvm_init_runtime_context() {
    RuntimeContext *context = pvm_mem_calloc(sizeof(RuntimeContext));
    context->sp = 0;
    context->vm = NULL;
    context->frame = NULL;
    context->exit = false;
    context->stack_size = VM_STACK_SIZE;
    context->throw_exception = pvm_thrown_exception;
    context->stack = pvm_mem_calloc(VM_STACK_SIZE);
    return context;
}

static inline void pvm_free_runtime_context(RuntimeContext **context) {
    if (context == NULL || *context == NULL) {
        return;
    }
    RuntimeContext *self = *context;
    pvm_mem_free(TO_REF(self->stack));
    pvm_mem_free(CAST_REF(context));
}

static inline void pvm_newarray(RuntimeContext *context) {
    const Type type = pvm_bytecode_read_int16(context);
    VMValue *value = pvm_get_operand(context);
#if VM_DEBUG_ENABLE
    if (!TYPE_INTEGER(value->type)) {
        context->throw_exception(context, "Except a integer type but it is '%s'", TYPE_NAME[value->type]);
    }
#endif
    PArr *obj = pvm_new_array(type, value->i32);
    value->obj = obj;
    value->type = TYPE_ARRAY;
}

static inline void pvm_setarray(RuntimeContext *context) {
    const VMValue *idx_value = pvm_pop_operand(context);
    const VMValue *arr_value = pvm_pop_operand(context);
    const VMValue *value = pvm_pop_operand(context);
    const PArr *array = (PArr *) (arr_value->obj);
    const int32_t index = idx_value->i32;
#if VM_DEBUG_ENABLE
    if (ARRAY_INDEX_OUT_OF_BOUND_CHECK(array, index)) {
        context->throw_exception(context, "Array index out of bounds.");
    }
#endif
    pvm_array_value_set(array, index, value);
}


static inline void pvm_getarray(RuntimeContext *context) {
    const VMValue *idx_val = pvm_pop_operand(context);
    VMValue *arr_val = pvm_get_operand(context);
    const PArr *array = (PArr *) (arr_val->obj);
    const int32_t index = idx_val->i32;
#if VM_DEBUG_ENABLE
    if (ARRAY_INDEX_OUT_OF_BOUND_CHECK(array, index)) {
        context->throw_exception(context, "Array index out of bounds.");
    }
#endif
    void *ptr = pvm_array_value_get(array, index);
    const Type t_ele = array->type;
    if (TYPE_SMALL_INTEGER(t_ele)) {
        int32_t tmp = 0;
        memcpy(&tmp, ptr, TYPE_SIZE[t_ele]);
        arr_val->i32 = tmp;
    } else if (TYPE_BIGGER_INTEGER(t_ele) || t_ele == TYPE_DOUBLE) {
        memcpy(&(arr_val->i64), ptr, TYPE_SIZE[t_ele]);
    } else {
        arr_val->obj = ptr;
    }
    arr_val->type = array->type;
}

static inline void pvm_iinc(RuntimeContext *context) {
    const uint16_t index = pvm_bytecode_read_int16(context);
    const int8_t offset = pvm_bytecode_read_int8(context);
    VMValue *value = pvm_get_local_value(context, index);
    switch (value->type) {
        case TYPE_BYTE:
        case TYPE_SHORT:
        case TYPE_INT:
            value->i32 += offset;
            break;
        case TYPE_LONG:
            value->i64 += offset;
            break;
        case TYPE_DOUBLE:
            value->f64 += offset;
            break;
        default:
            context->throw_exception(context, "iinc instruction only support number type.");
    }
}

static inline void pvm_lfield(RuntimeContext *context) {
    const uint32_t index = pvm_bytecode_read_int16(context);
    VMValue *value = pvm_next_operand(context);
    pvm_get_klass_field(context, index, value);
}

static inline void pvm_sfield(RuntimeContext *context) {
    const uint16_t index = pvm_bytecode_read_int16(context);
    const VMValue *value = pvm_pop_operand(context);
    pvm_set_klass_field(context, index, value);
}

static inline void pvm_srefinc(RuntimeContext *context) {
    const uint16_t index = pvm_bytecode_read_int16(context);
    const VMValue *value = pvm_get_local_value(context, index);
#if VM_DEBUG_ENABLE
    if (value->type != TYPE_STRING && value->type != TYPE_HANDLE) {
        context->throw_exception(context, "refinc only support reference.");
    }
#endif
    pvm_object_refinc(value->obj);
}

static inline void pvm_srefdec(RuntimeContext *context) {
    const uint16_t index = pvm_bytecode_read_int16(context);
    const VMValue *value = pvm_get_local_value(context, index);
#if VM_DEBUG_ENABLE
    const Type type = value->type;
    if (!TYPE_REFERENCE(type)) {
        context->throw_exception(context, "refdec only support reference.");
    }
#endif
    pvm_object_refdec(value->obj);
}

static inline void pvm_refinc(RuntimeContext *context) {
    const VMValue *value = pvm_pop_operand(context);
#if VM_DEBUG_ENABLE
    const Type type = value->type;
    if (!TYPE_REFERENCE(type)) {
        context->throw_exception(context, "refinc only support reference.");
    }
#endif
    pvm_object_refinc(value->obj);
}

static inline void pvm_refdec(RuntimeContext *context) {
    const VMValue *value = pvm_pop_operand(context);
#if VM_DEBUG_ENABLE
    const Type type = value->type;
    if (!TYPE_REFERENCE(type)) {
        context->throw_exception(context, "refinc only support reference.");
    }
#endif
    pvm_object_refdec(value->obj);
}

static inline void pvm_ldc(RuntimeContext *context) {
    const uint16_t index = pvm_bytecode_read_int16(context);
    VMValue *value = pvm_next_operand(context);
    pvm_get_klass_constant(context, index, value);
}

static inline void pvm_iand(RuntimeContext *context) {
    const VMValue *source_operand = pvm_pop_operand(context);
    VMValue *target_operand = pvm_get_operand(context);
#if VM_DEBUG_ENABLE
    const Type t0 = source_operand->type;
    const Type t1 = target_operand->type;
    if (t0 != t1 || t0 != TYPE_INT) {
        context->throw_exception(
            context, "Bitwise AND operator '&' requires both operands to be the same integer type");
    }
#endif
    target_operand->i32 &= source_operand->i32;
}

static inline void pvm_land(RuntimeContext *context) {
    const VMValue *source_operand = pvm_pop_operand(context);
    VMValue *target_operand = pvm_get_operand(context);
#if VM_DEBUG_ENABLE
    const Type t0 = source_operand->type;
    const Type t1 = target_operand->type;
    if (t0 != t1 || t0 != TYPE_LONG) {
        context->throw_exception(
            context, "Bitwise AND operator '&' requires both operands to be the same long integer type");
    }
#endif
    target_operand->i64 &= source_operand->i64;
}

static inline void pvm_ixor(RuntimeContext *context) {
    const VMValue *source_operand = pvm_pop_operand(context);
    VMValue *target_operand = pvm_get_operand(context);
#if VM_DEBUG_ENABLE
    const Type t0 = source_operand->type;
    const Type t1 = target_operand->type;
    if (t0 != t1 || t0 != TYPE_INT) {
        context->throw_exception(
            context, "Bitwise XOR operator '^' requires both operands to be the same integer type");
    }
#endif
    target_operand->i32 ^= source_operand->i32;
}

static inline void pvm_lxor(RuntimeContext *context) {
    const VMValue *source_operand = pvm_pop_operand(context);
    VMValue *target_operand = pvm_get_operand(context);
#if VM_DEBUG_ENABLE
    const Type t0 = source_operand->type;
    const Type t1 = target_operand->type;
    if (t0 != t1 || t0 != TYPE_LONG) {
        context->throw_exception(
            context, "Bitwise XOR operator '^' requires both operands to be the same long type");
    }
#endif
    target_operand->i64 ^= source_operand->i64;
}

static inline void pvm_ior(RuntimeContext *context) {
    const VMValue *source_operand = pvm_pop_operand(context);
    VMValue *target_operand = pvm_get_operand(context);
#if VM_DEBUG_ENABLE
    const Type t0 = source_operand->type;
    const Type t1 = target_operand->type;
    if (t0 != t1 || t0 != TYPE_INT) {
        context->throw_exception(
            context, "Bitwise OR operator '|' requires both operands to be the same integer type");
    }
#endif
    target_operand->i32 |= source_operand->i32;
}

static inline void pvm_lor(RuntimeContext *context) {
    const VMValue *source_operand = pvm_pop_operand(context);
    VMValue *target_operand = pvm_get_operand(context);
#if VM_DEBUG_ENABLE
    const Type t0 = source_operand->type;
    const Type t1 = target_operand->type;
    if (t0 != t1 || t0 != TYPE_LONG) {
        context->throw_exception(
            context, "Bitwise OR operator '|' requires both operands to be the same long type");
    }
#endif
    target_operand->i64 |= source_operand->i64;
}

static inline void pvm_inot(RuntimeContext *context) {
    VMValue *value = pvm_get_operand(context);
#if VM_DEBUG_ENABLE
    if (value->type != TYPE_INT) {
        context->throw_exception(context, "Bitwise NOT operator '~' requires operand to be an integer type");
    }
#endif
    value->i32 = ~value->i32;
}

static inline void pvm_lnot(RuntimeContext *context) {
    VMValue *value = pvm_get_operand(context);
#if VM_DEBUG_ENABLE
    if (value->type != TYPE_LONG) {
        context->throw_exception(context, "Bitwise NOT operator '~' requires operand to be a long type");
    }
#endif
    value->i32 = ~value->i32;
}

static inline void pvm_i2b(RuntimeContext *context) {
    VMValue *value = pvm_get_operand(context);
#if VM_DEBUG_ENABLE
    if (value->type != TYPE_INT) {
        context->throw_exception(context, "i2b require a int type.");
    }
#endif
    value->i8 = (int8_t) value->i32;
}

static inline void pvm_i2s(RuntimeContext *context) {
    VMValue *value = pvm_get_operand(context);
#if VM_DEBUG_ENABLE
    if (value->type != TYPE_INT) {
        context->throw_exception(context, "i2s require a int type.");
    }
#endif
    value->i16 = (int16_t) value->i32;
}

static inline void pvm_b2s(RuntimeContext *context) {
    VMValue *value = pvm_get_operand(context);
#if VM_DEBUG_ENABLE
    if (value->type != TYPE_BYTE) {
        context->throw_exception(context, "b2s require a byte type.");
    }
#endif
    value->i32 = (int32_t) value->i8;
    value->type = TYPE_SHORT;
}

static inline void pvm_b2i(RuntimeContext *context) {
    VMValue *value = pvm_get_operand(context);
#if VM_DEBUG_ENABLE
    if (value->type != TYPE_BYTE) {
        context->throw_exception(context, "b2i require a byte type.");
    }
#endif
    value->i32 = (int32_t) value->i8;
    value->type = TYPE_INT;
}

static inline void pvm_b2l(RuntimeContext *context) {
    VMValue *value = pvm_get_operand(context);
#if VM_DEBUG_ENABLE
    if (value->type != TYPE_BYTE) {
        context->throw_exception(context, "b2l require a byte type.");
    }
#endif
    value->i64 = (int64_t) value->i8;
    value->type = TYPE_LONG;
}

static inline void pvm_b2d(RuntimeContext *context) {
    VMValue *value = pvm_get_operand(context);
#if VM_DEBUG_ENABLE
    if (value->type != TYPE_BYTE) {
        context->throw_exception(context, "b2d require a byte type.");
    }
#endif
    value->f64 = (double) value->i8;
    value->type = TYPE_DOUBLE;
}

static inline void pvm_s2b(RuntimeContext *context) {
    VMValue *value = pvm_get_operand(context);
#if VM_DEBUG_ENABLE
    if (value->type != TYPE_SHORT) {
        context->throw_exception(context, "s2b require a short type.");
    }
#endif
    value->i32 = (int32_t) value->i16;
    value->type = TYPE_BYTE;
}

static inline void pvm_s2i(RuntimeContext *context) {
    VMValue *value = pvm_get_operand(context);
#if VM_DEBUG_ENABLE
    if (value->type != TYPE_SHORT) {
        context->throw_exception(context, "s2i require a short type.");
    }
#endif
    value->i32 = (int32_t) value->i16;
    value->type = TYPE_INT;
}

static inline void pvm_s2l(RuntimeContext *context) {
    VMValue *value = pvm_get_operand(context);
#if VM_DEBUG_ENABLE
    if (value->type != TYPE_SHORT) {
        context->throw_exception(context, "s2l require a short type.");
    }
#endif
    value->i64 = (int64_t) value->i16;
    value->type = TYPE_LONG;
}

static inline void pvm_s2d(RuntimeContext *context) {
    VMValue *value = pvm_get_operand(context);
#if VM_DEBUG_ENABLE
    if (value->type != TYPE_SHORT) {
        context->throw_exception(context, "s2d require a short type.");
    }
#endif
    value->f64 = (double) value->i16;
    value->type = TYPE_DOUBLE;
}

extern RuntimeContext *pvm_execute_context_get() {
    return g_private_get(&pri_key);
}

extern VMValue *pvm_execute_context(RuntimeContext *context, VirtualStackFrame *call_frame) {
    static const void *opcode_table[] = {
        [LI8] = &&li8,
        [LI16] = &&li16,
        [LI32] = &&li32,
        [LI64] = &&li64,
        [LF64] = &&lf64,
        [SLOCAL] = &&slocal,
        [LLOCAL] = &&llocal,
        [ADD] = &&add,
        [SUB] = &&sub,
        [MUL] = &&mul,
        [DIV] = &&div,
        [GOTO] = &&goto0,
        [RET] = &&ret,
        [L2F] = &&i2f_l2f,
        [I2F] = &&i2f_l2f,
        [L2I] = &&f2i_l2i,
        [F2I] = &&f2i_l2i,
        [I2L] = &&i2l_f2l,
        [F2L] = &&i2l_f2l,
        [CALL] = &&call,
        [IFEQ] = &&ifeq,
        [IFNE] = &&ifne,
        [THROW] = &&thrown,
        [POP] = &&pop,
        [NEWARRAY] = &&newarray,
        [GETARRAY] = &&getarray,
        [SETARRAY] = &&setarray,
        [IINC] = &&iinc,
        [IFLE] = &&ifle,
        [IFLT] = &&iflt,
        [IFGE] = &&ifge,
        [IFGT] = &&ifgt,
        [ICMP] = &&icmp,
        [DCMP] = &&dcmp,
        [LCMP] = &&lcmp,
        [ISHR] = &&ishr,
        [ISHL] = &&ishl,
        [IUSHR] = &&iushr,
        [LUSHR] = &&lushr,
        [LSHL] = &&lshl,
        [LSHR] = &&lshr,
        [LFIELD] = &&lfield,
        [SFIELD] = &&sfield,
        [SREFDEC] = &&srefdec,
        [SREFINC] = &&srefinc,
        [REFINC] = &&refinc,
        [REFDEC] = &&refdec,
        [LDC] = &&ldc,
        [ICMP0] = &&icmp0,
        [ICMP1] = &&icmp1,
        [ICMPX] = &&icmpx,
        [LCMP0] = &&lcmp0,
        [LCMP1] = &&lcmp1,
        [LCMPX] = &&lcmpx,
        [DCMP0] = &&dcmp0,
        [DCMP1] = &&dcmp1,
        [DCMPX] = &&dcmpx,
        [IFTRUE] = &&iftrue,
        [IFFALSE] = &&iffalse,
        [LTRUE] = &&ltrue,
        [LFALSE] = &&lfalse,
        [IAND] = &&iand,
        [LAND] = &&land,
        [IXOR] = &&ixor,
        [LXOR] = &&lxor,
        [IOR] = &&ior,
        [LOR] = &&lor,
        [INOT] = &&inot,
        [LNOT] = &&lnot,
        [I2B] = &&i2b,
        [I2S] = &&i2s,
        [B2S] = &&b2s,
        [B2I] = &&b2i,
        [B2L] = &&b2l,
        [B2D] = &&b2d,
        [S2B] = &&s2b,
        [S2I] = &&s2i,
        [S2L] = &&s2l,
        [S2D] = &&s2d,

    };
    VMValue *ret_val = NULL;
    DISPATCH;
li8:
    pvm_lint8(context);
    DISPATCH;
li16:
    pvm_lint16(context);
    DISPATCH;
li32:
    pvm_lint32(context);
    DISPATCH;
li64:
    pvm_lint64(context);
    DISPATCH;
lf64:
    pvm_lf64(context);
    DISPATCH;
slocal:
    pvm_slocal(context);
    DISPATCH;
llocal:
    pvm_llocal(context);
    DISPATCH;
add:
    pvm_add(context);
    DISPATCH;
sub:
    pvm_sub(context);
    DISPATCH;
mul:
    pvm_mul(context);
    DISPATCH;
div:
    pvm_div(context);
    DISPATCH;
goto0:
    pvm_goto(context);
    DISPATCH;
ret:
    ret_val = pvm_ret(context, call_frame);
    if (context->exit) {
        goto finally;
    }
    DISPATCH;
i2f_l2f:
    pvm_i2f_l2f(context);
    DISPATCH;
f2i_l2i:
    pvm_f2i_l2i(context);
    DISPATCH;
i2l_f2l:
    pvm_i2l_f2l(context);
    DISPATCH;
call:
    pvm_call(context);
    DISPATCH;
ifeq:
    IFXX(context, ==);
    DISPATCH;
ifne:
    IFXX(context, !=);
    DISPATCH;
thrown:
    pvm_throw(context);
    DISPATCH;
pop:
    pvm_pop_operand(context);
    DISPATCH;
getarray:
    pvm_getarray(context);
    DISPATCH;
setarray:
    pvm_setarray(context);
    DISPATCH;
newarray:
    pvm_newarray(context);
    DISPATCH;
iinc:
    pvm_iinc(context);
    DISPATCH;
ifle:
    IFXX(context, <=);
    DISPATCH;
iflt:
    IFXX(context, <);
    DISPATCH;
ifgt:
    IFXX(context, >);
    DISPATCH;
ifge:
    IFXX(context, >=);
    DISPATCH;
icmp:
    XCMP(context, i32);
    DISPATCH;
dcmp:
    XCMP(context, f64);
    DISPATCH;
lcmp:
    XCMP(context, i64);
    DISPATCH;
icmp0:
    CMPX(context, i32, 0);
    DISPATCH;
icmp1:
    CMPX(context, i32, 1);
    DISPATCH;
icmpx:
    CMPX(context, i32, -1);
    DISPATCH;
lcmp0:
    CMPX(context, i64, 0);
    DISPATCH;
lcmp1:
    CMPX(context, i64, 1);
    DISPATCH;
lcmpx:
    CMPX(context, i64, -1);
    DISPATCH;
dcmp0:
    CMPX(context, f64, 0);
    DISPATCH;
dcmp1:
    CMPX(context, f64, 1);
    DISPATCH;
dcmpx:
    CMPX(context, f64, -1);
    DISPATCH;
ishr:
    pvm_ishr(context);
    DISPATCH;
iushr:
    pvm_iushr(context);
    DISPATCH;
ishl:
    pvm_ishl(context);
    DISPATCH;
lshl:
    pvm_lshl(context);
    DISPATCH;
lshr:
    pvm_lshr(context);
    DISPATCH;
lushr:
    pvm_lushr(context);
    DISPATCH;
lfield:
    pvm_lfield(context);
    DISPATCH;
sfield:
    pvm_sfield(context);
    DISPATCH;
srefinc:
    pvm_srefinc(context);
    DISPATCH;
srefdec:
    pvm_srefdec(context);
    DISPATCH;
refdec:
    pvm_refdec(context);
    DISPATCH;
refinc:
    pvm_refinc(context);
    DISPATCH;
ldc:
    pvm_ldc(context);
    DISPATCH;
iftrue:
    IFXX(context, !=);
    DISPATCH;
iffalse:
    IFXX(context, ==);
    DISPATCH;
ltrue:
    LOAD_BOOL_CONST(context, 1);
    DISPATCH;
lfalse:
    LOAD_BOOL_CONST(context, 0);
    DISPATCH;
iand:
    pvm_iand(context);
    DISPATCH;
land:
    pvm_land(context);
    DISPATCH;
ixor:
    pvm_ixor(context);
    DISPATCH;
lxor:
    pvm_lxor(context);
    DISPATCH;
ior:
    pvm_ior(context);
    DISPATCH;
lor:
    pvm_lor(context);
    DISPATCH;
inot:
    pvm_inot(context);
    DISPATCH;
lnot:
    pvm_lnot(context);
    DISPATCH;
i2b:
    pvm_i2b(context);
    DISPATCH;
i2s:
    pvm_i2s(context);
    DISPATCH;
b2s:
    pvm_b2s(context);
    DISPATCH;
b2i:
    pvm_b2i(context);
    DISPATCH;
b2l:
    pvm_b2l(context);
    DISPATCH;
b2d:
    pvm_b2d(context);
    DISPATCH;
s2b:
    pvm_s2b(context);
    DISPATCH;
s2i:
    pvm_s2i(context);
    DISPATCH;
s2l:
    pvm_s2l(context);
    DISPATCH;
s2d:
    pvm_s2d(context);
    DISPATCH;
finally:
    return ret_val;
}

extern void pvm_execute(const CallMethodParam *callMethodParam) {
    RuntimeContext *context = pvm_init_runtime_context();
    context->vm = (PixtronVM *) callMethodParam->vm;
    const Method *method = callMethodParam->method;
    const VMValue **argv = callMethodParam->argv;
    pvm_create_stack_frame(context, method, callMethodParam->argc, argv);
    g_private_set(&pri_key, context);
    pvm_execute_context(context,NULL);
}
