// Copyright 2012, Camilo Aguilar. Cloudescape, LLC.
#include "bindings.h"

namespace NodeFuse {
    Persistent<FunctionTemplate> Fuse::constructor_template;

    //static Persistent<String> mountpoint_sym;
    static Persistent<String> filesystem_sym;
    static Persistent<String> options_sym;

    void Fuse::Initialize(Handle<Object> target) {
        Local<FunctionTemplate> t = FunctionTemplate::New(Fuse::New);

        t->InstanceTemplate()->SetInternalFieldCount(1);

        NODE_SET_PROTOTYPE_METHOD(t, "mount",
                                      Fuse::Mount);
        //NODE_SET_PROTOTYPE_METHOD(t, "unmount",
        //                              Fuse::Unmount);

        constructor_template = Persistent<FunctionTemplate>::New(t);
        constructor_template->SetClassName(String::NewSymbol("Fuse"));

        target->Set(String::NewSymbol("fuse_version"), Integer::New(fuse_version()));
        target->Set(String::NewSymbol("Fuse"), constructor_template->GetFunction());

        //mountpoint_sym        = NODE_PSYMBOL("mountpoint");
        filesystem_sym        = NODE_PSYMBOL("filesystem");
        options_sym           = NODE_PSYMBOL("options");
    }

    Fuse::Fuse() : ObjectWrap() {}
    Fuse::~Fuse() {
        if (fargs != NULL) {
            fuse_opt_free_args(fargs);
        }

        if (session != NULL) {
            fuse_remove_signal_handlers(session);
        }

        if (channel != NULL) {
            fuse_unmount(mountpoint, channel);
            fuse_session_remove_chan(channel);
        }

        if (mountpoint != NULL) {
            free(mountpoint);
        }
    }

    Handle<Value> Fuse::New(const Arguments& args) {
        HandleScope scope;

        Fuse *fuse = new Fuse();
        Local<Object> obj = args.This();
        fuse->Wrap(obj);

        return obj;
    }

    Handle<Value> Fuse::Mount(const Arguments& args) {
        HandleScope scope;

        int argslen = args.Length();

        if (argslen == 0) {
            return ThrowException(Exception::TypeError(
            String::New("You must specify arguments to invoke this function")));
        }

        if (!args[0]->IsObject()) {
            return ThrowException(Exception::TypeError(
            String::New("You must specify an Object as first argument")));
        }

        Local<Object> argsObj = args[0]->ToObject();
        //THROW_IF_MISSING_PROPERTY(argsObj, mountpoint_sym, "mountpoint");
        THROW_IF_MISSING_PROPERTY(argsObj, filesystem_sym, "filesystem");
        THROW_IF_MISSING_PROPERTY(argsObj, options_sym, "options");

        //Local<Value> vmountpoint = argsObj->Get(mountpoint_sym);
        Local<Value> vfilesystem = argsObj->Get(filesystem_sym);
        Local<Value> voptions = argsObj->Get(options_sym);

        /*if (!vmountpoint->IsString()) {
            return ThrowException(Exception::TypeError(
                String::New("Wrong type for property 'mountpoint', a String is expected")));

        }*/

        if (!vfilesystem->IsFunction()) {
            return ThrowException(Exception::TypeError(
                String::New("Wrong type for property 'filesystem', a Function is expected")));
        }

        if (!voptions->IsArray()) {
            return ThrowException(Exception::TypeError(
                String::New("Wrong type for property 'options', an Array is expected")));
        }

        Local<Array> options = Local<Array>::Cast(voptions);
        int argc = options->Length();

        //If no mountpoint is provided, show usage.
        if (argc <= 2) {
            options->Set(Integer::New(2), String::New("--help"));
            argc++;
        }

        Local<Object> currentInstance = args.This();
        Fuse *fuse = ObjectWrap::Unwrap<Fuse>(currentInstance);

        struct fuse_args fargs = FUSE_ARGS_INIT(0, NULL);
        fuse->fargs = &fargs;

        for (int i = 1; i < argc; i++) {
            String::Utf8Value option(options->Get(Integer::New(i))->ToString());

            if (fuse_opt_add_arg(fuse->fargs, (const char *) *option) == -1) {
                FUSEJS_THROW_EXCEPTION("Unable to allocate memory, fuse_opt_add_arg failed: ", strerror(errno));
                return Null();
            }
        }

        //String::Utf8Value mountpoint(vmountpoint->ToString());
        //fuse->mountpoint = *mountpoint;

        int ret = fuse_parse_cmdline(fuse->fargs, &fuse->mountpoint,
                                        &fuse->multithreaded, &fuse->foreground);
        if (ret == -1) {
            FUSEJS_THROW_EXCEPTION("Error parsing fuse options: ", strerror(errno));
            return Null();
        }

        fuse->channel = fuse_mount((const char*) fuse->mountpoint, fuse->fargs);
        if (fuse->channel == NULL) {
            FUSEJS_THROW_EXCEPTION("Unable to mount filesystem: ", strerror(errno));
            return Null();
        }

        Local<Value> argv[2] = {
            currentInstance,
            options
        };

        Local<Function> filesystem = Local<Function>::Cast(vfilesystem);
        fuse->fsobj = Persistent<Object>::New(filesystem->NewInstance(2, argv));

        assert(fuse->fsobj->IsObject());
        assert(fuse->fsobj->Get(String::NewSymbol("init"))->IsFunction());

        struct fuse_lowlevel_ops *operations = FileSystem::GetOperations();

        fuse->session = fuse_lowlevel_new(fuse->fargs, operations,
                                            sizeof(*operations), fuse);

        if (fuse->session == NULL) {
            fuse_unmount(fuse->mountpoint, fuse->channel);
            fuse_opt_free_args(fuse->fargs);
            //FUSEJS_THROW_EXCEPTION("Error creating fuse session: ", strerror(errno));
            return Null();
        }

        ret = fuse_set_signal_handlers(fuse->session);
        if (ret == -1) {
            fuse_session_destroy(fuse->session);
            fuse_unmount(fuse->mountpoint, fuse->channel);
            fuse_opt_free_args(fuse->fargs);
            FUSEJS_THROW_EXCEPTION("Error setting fuse signal handlers: ", strerror(errno));
            return Null();
        }

        fuse_session_add_chan(fuse->session, fuse->channel);

        ret = fuse_session_loop(fuse->session); //blocks here

        //It continues executing if user unmounts the fs
        fuse_remove_signal_handlers(fuse->session);
        fuse_unmount(fuse->mountpoint, fuse->channel);
        fuse_session_remove_chan(fuse->channel);
        fuse_session_destroy(fuse->session);
        fuse_opt_free_args(fuse->fargs);

        if (ret == -1) {
            FUSEJS_THROW_EXCEPTION("Error starting fuse session loop: ", strerror(errno));
            return Null();
        }

        return currentInstance;
    }

    /*Handle<Value> Fuse::Unmount(const Arguments& args) {
        HandleScope scope;

        Local<Object> currentInstance = args.This();
        Fuse *fuse = ObjectWrap::Unwrap<Fuse>(currentInstance);

        fuse_session_remove_chan(fuse->channel);
        fuse_remove_signal_handlers(fuse->session);
        fuse_session_destroy(fuse->session);
        fuse_unmount(fuse->mountpoint, fuse->channel);

        return currentInstance;
    }*/
} //namespace NodeFuse

