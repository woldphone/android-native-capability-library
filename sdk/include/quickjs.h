#ifndef QUICKJS_H
#define QUICKJS_H

#include <stdio.h>
#include <stdint.h>
#include <stddef.h>

typedef struct JSRuntime JSRuntime;
typedef struct JSContext JSContext;
typedef struct JSValue {
    uint64_t tag;
    int64_t val;
} JSValue;

typedef JSValue JSCFunction(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv);
typedef struct JSCFunctionListEntry {
    const char *name;
    uint8_t prop_flags;
    uint8_t def_type;
    int16_t magic;
} JSCFunctionListEntry;

#endif
