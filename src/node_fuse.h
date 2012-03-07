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

#define THROW_IF_UNEXPECTED_TYPE(symbol, value, expected_type)                      \
    if (!value->expected_type()) {                                                  \
        return v8::ThrowException(v8::Exception::TypeError(                         \
        v8::String::New("Wrong type for " #symbol ", expected " #expected_type)));  \
    }                                                                               \

namespace NodeFuse {

}

#endif  // SRC_NODE_FUSE_H_

