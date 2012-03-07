// Copyright 2012, Camilo Aguilar. Cloudescape, LLC.
#include "bindings.h"
namespace NodeFuse {
     void Fuse::Initialize(Handle<Object> target) {
        Local<FunctionTemplate> t = FunctionTemplate::New(Fuse::New);

        t->InstanceTemplate()->SetInternalFieldCount(1);

        NODE_SET_PROTOTYPE_METHOD(t, "mount",
                                      Fuse::Mount);
        NODE_SET_PROTOTYPE_METHOD(t, "umount",
                                      Fuse::Umount);

        t->SetClassName(String::NewSymbol("Fuse"));
        target->Set(String::NewSymbol("Fuse"), t->GetFunction());
    }

    Fuse::Fuse() : ObjectWrap() {}
    Fuse::~Fuse() {}

    Handle<Value> Fuse::New(const Arguments& args) {
        HandleScope scope;

        Fuse *fuse = new Fuse();
        Local<Object> obj = args.This();
        fuse->Wrap(obj);

        return obj;
    }

    Handle<Value> Fuse::Mount(const Arguments& args) {
        HandleScope scope;
        //handles parameters and mounts the filesystem
    }

    Handle<Value> Fuse::Umount(const Arguments& args) {
        //umount filesystem
    }
} //namespace NodeFuse

