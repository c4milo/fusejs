// Copyright 2012, Camilo Aguilar. Cloudescape, LLC.
#include "bindings.h"
#include "reply.h"

namespace NodeFuse {
    static Persistent<String> uid_sym = NODE_PSYMBOL("uid");
    static Persistent<String> gid_sym = NODE_PSYMBOL("gid");
    static Persistent<String> pid_sym = NODE_PSYMBOL("pid");

    void InitializeFuse(Handle<Object> target) {
        HandleScope scope;

        Fuse::Initialize(target);
        Reply::Initialize();

        target->Set(String::NewSymbol("version"),
                    String::New(NODE_FUSE_VERSION));

        target->Set(String::NewSymbol("fuse_version"),
                    Integer::New(fuse_version()));
    }

    const struct fuse_entry_param* ObjectToFuseEntryParam(Handle<Value> value) {
        HandleScope scope;

        Local<Object> e = value->ToObject();
        return NULL;
    }

    Handle<Value> FuseEntryParamToObject(const struct fuse_entry_param* entry) {
        HandleScope scope;

    }

    Handle<Value> RequestContextToObject(const struct fuse_ctx* ctx) {
        HandleScope scope;
        Local<Object> context = Object::New();

        context->Set(uid_sym, Number::New(ctx->uid));
        context->Set(gid_sym, Number::New(ctx->gid));
        context->Set(pid_sym, Number::New(ctx->pid));

        return scope.Close(context);
    }

    NODE_MODULE(fuse, InitializeFuse)
} //namespace NodeFuse

