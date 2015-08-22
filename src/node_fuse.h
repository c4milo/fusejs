/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef SRC_NODE_FUSE_H_
#define SRC_NODE_FUSE_H_

#include <node.h>
#include <node_buffer.h>
#include <fuse_lowlevel.h>
#include <errno.h>
#include <string.h>
#include <nan.h>

#define NODE_FUSE_VERSION "1.2.0"

using namespace v8;
using namespace node;

#define THROW_IF_MISSING_PROPERTY(obj, name)                                \
    if (Nan::Has(obj,Nan::New<String>(name).ToLocalChecked()) == Nan::Just(false) ) {                                                        \
        Nan::ThrowError("You must have set the property " #name " in the object");    \
    }                                                                               \

#define FUSEJS_THROW_EXCEPTION(err, fuse_err)                                       \
        Nan::ThrowError( v8::String::Concat(Nan::New<String>(err).ToLocalChecked(), Nan::New<String>(fuse_err).ToLocalChecked()));        \

namespace NodeFuse {
    int ObjectToFuseEntryParam(Handle<Value> value, struct fuse_entry_param* entry);
    int ObjectToStat(Handle<Value> value, struct stat* stat);
    int ObjectToStatVfs(Handle<Value> value, struct statvfs* statvfs);
    int ObjectToFlock(Handle<Value> value, struct flock* lock);
    Handle<Value> RequestContextToObject(const struct fuse_ctx* ctx);
    Handle<Value> GetAttrsToBeSet(int to_set, struct stat* stat);
    Handle<Value> FileInfoToObject(struct fuse_file_info* fi);
    Handle<Value> FlockToObject(const struct flock* lock);
}

#endif  // SRC_NODE_FUSE_H_

