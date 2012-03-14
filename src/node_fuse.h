// Copyright 2012, Camilo Aguilar. Cloudescape, LLC.
#ifndef SRC_NODE_FUSE_H_
#define SRC_NODE_FUSE_H_

#include <node.h>
#include <fuse_lowlevel.h>
#include <errno.h>
#include <string.h>

#define NODE_FUSE_VERSION "0.0.1"

using namespace v8;
using namespace node;

#define THROW_IF_MISSING_PROPERTY(obj, symbol, name)                                \
    if (!obj->Has(symbol)) {                                                        \
        return v8::ThrowException(v8::Exception::TypeError(                         \
        v8::String::New("You must have set the property " #name " in the object")));\
    }                                                                               \

#define FUSEJS_THROW_EXCEPTION(err, fuse_err)                                       \
        v8::Local<v8::Value> exception = v8::Exception::Error(                      \
        v8::String::Concat(v8::String::New(err), v8::String::New(fuse_err)));        \
        ThrowException(exception);

namespace NodeFuse {
    const struct fuse_entry_param* ObjectToFuseEntryParam(Handle<Value> value);
    Handle<Value> FuseEntryParamToObject(const struct fuse_entry_param* entry);
    Handle<Value> RequestContextToObject(const struct fuse_ctx* ctx);
}

#endif  // SRC_NODE_FUSE_H_

