// Copyright 2010, Camilo Aguilar. Cloudescape, LLC.
#include "bindings.h"

namespace NodeFuse {
    void Initialize(Handle<Object> target) {
        HandleScope scope;

        Fuse::Initialize(target);

        target->Set(String::NewSymbol("version"),
                    String::New(NODE_FUSE_VERSION));

        Handle<ObjectTemplate> global = ObjectTemplate::New();
        Handle<Context> context = Context::New(NULL, global);
        Context::Scope context_scope(context);

        context->Global()->Set(String::NewSymbol("Fuse"), target);
    }

    NODE_MODULE(fuse, Initialize)
} //namespace NodeFuse

