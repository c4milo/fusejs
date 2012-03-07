// Copyright 2012, Camilo Aguilar. Cloudescape, LLC.
#include "bindings.h"
namespace NodeFuse {
    Persistent<FunctionTemplate> Fuse::constructor_template;

    static Persistent<String> mountpoint_sym;
    static Persistent<String> operations_sym;
    static Persistent<String> options_sym;

    void Fuse::Initialize(Handle<Object> target) {
        Local<FunctionTemplate> t = FunctionTemplate::New(Fuse::New);

        t->InstanceTemplate()->SetInternalFieldCount(1);

        NODE_SET_PROTOTYPE_METHOD(t, "mount",
                                      Fuse::Mount);
        NODE_SET_PROTOTYPE_METHOD(t, "umount",
                                      Fuse::Umount);

        constructor_template = Persistent<FunctionTemplate>::New(t);
        constructor_template->SetClassName(String::NewSymbol("Fuse"));

        target->Set(String::NewSymbol("Fuse"), constructor_template->GetFunction());

        mountpoint_sym        = NODE_PSYMBOL("mountpoint");
        operations_sym        = NODE_PSYMBOL("operations");
        options_sym           = NODE_PSYMBOL("options");
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
        struct fuse_chan* channel = NULL;
        struct fuse_session* session = NULL;
        struct fuse_args* fargs = NULL;

        struct fuse_args fuse_args = FUSE_ARGS_INIT(0, NULL);

        char* mountpoint = NULL;
        char* options = NULL;
        char* operations = NULL;

        if (args.Length() == 0) {
            return ThrowException(Exception::TypeError(
            String::New("You must specify arguments to invoke this function")));
        }

        if (!args[0]->IsObject()) {
            return ThrowException(Exception::TypeError(
            String::New("You must specify an object as first argument")));
        }

        Local<Object> args_ = args[0]->ToObject();
        THROW_IF_MISSING_PROPERTY(args_, mountpoint_sym, "mountpoint");
        THROW_IF_MISSING_PROPERTY(args_, operations_sym, "operations");
        THROW_IF_MISSING_PROPERTY(args_, options_sym, "options");

        /*THROW_IF_UNEXPECTED_TYPE("mountpoint", mountpoint_sym, "isString");
        THROW_IF_UNEXPECTED_TYPE("operations", args_->Get(operations_sym), "Object");
        THROW_IF_UNEXPECTED_TYPE("options", args_->Get(options_sym), "Array");*/

        //Local<Value>
        //parse options
        /*if (fuse_parse_cmdline(fargs, NULL, NULL, NULL) != 0) {
        }*/

        //channel = fuse_mount(mountpoint, &fargs);
        //session = fuse_lowlevel_new(&fargs, &fuse_ops, sizeof(fuse_ops), NULL)
        //fuse_set_signal_handlers(session)
        //fuse_session_loop()

    }

    Handle<Value> Fuse::Umount(const Arguments& args) {
        //umount filesystem
    }
} //namespace NodeFuse

