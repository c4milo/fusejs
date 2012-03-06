// Copyright 2012, Camilo Aguilar. Cloudescape, LLC.
#include "bindings.h"

namespace NodeFuse {
    void InitializeFuse(Handle<Object> target) {
        HandleScope scope;

        Fuse::Initialize(target);
        /*Session::Initialize(target);
        Channel::Initialize(target);
        Request::Initialize(target);
        Reply::Initialize(target);*/

        target->Set(String::NewSymbol("version"),
                    String::New(NODE_FUSE_VERSION));

        target->Set(String::NewSymbol("fuse_version"),
                    Integer::New(fuse_version()));
    }

    NODE_MODULE(fuse, InitializeFuse)
} //namespace NodeFuse

