/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */
#include <stdlib.h>

#include "bindings.h"
#include "filesystem.h"
uv_async_t uv_async_handle;
namespace NodeFuse {
    Nan::Persistent<Function> Fuse::constructor;

    void Fuse::Initialize(Handle<Object> target) {
        Local<FunctionTemplate> t = Nan::New<FunctionTemplate>(Fuse::New);
        t->SetClassName(Nan::New<String>("Fuse").ToLocalChecked());
        t->InstanceTemplate()->SetInternalFieldCount(1);

        Nan::SetPrototypeMethod(t, "mount",
                                      Fuse::Mount);

        Nan::SetPrototypeMethod(t, "unmount",
                                     Fuse::Unmount);

        constructor.Reset(t->GetFunction());

        Nan::Set(target, Nan::New("fuse_version").ToLocalChecked(), Nan::New<Integer>( static_cast<uint32_t>(fuse_version())) );
        Nan::Set(target, Nan::New("Fuse").ToLocalChecked(), t->GetFunction());


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

    NAN_METHOD( Fuse::New) {
        if (info.IsConstructCall()) {
          Fuse *fuse = new Fuse();
          Local<Object> obj = info.This();
          fuse->Wrap(obj);
              info.GetReturnValue().Set( obj );
        } else {
          Local<Function> cons = Nan::New<Function>(constructor);
          info.GetReturnValue().Set(cons->NewInstance());
        }


    }

    void Fuse::RemoteMount(void *_args_) {
        Fuse *fuse  = (Fuse *) _args_; 
        int ret;
        // fuse->channel = fuse_mount((const char*) fuse->mountpoint, fuse->fargs);
        // if (fuse->channel == NULL) {
        //     FUSEJS_THROW_EXCEPTION("Unable to mount filesystem: ", strerror(errno));
        //     return;
        // }

        struct fuse_lowlevel_ops *operations = FileSystem::GetOperations();

        fuse->session = fuse_lowlevel_new(fuse->fargs, operations,
                                            sizeof(*operations), fuse);

        if (fuse->session == NULL) {
            fuse_unmount(fuse->mountpoint, fuse->channel);
            fuse_opt_free_args(fuse->fargs);
            //FUSEJS_THROW_EXCEPTION("Error creating fuse session: ", strerror(errno));
            return;
        }

        ret = fuse_set_signal_handlers(fuse->session);
        if (ret == -1) {
            fuse_session_destroy(fuse->session);
            fuse_unmount(fuse->mountpoint, fuse->channel);
            fuse_opt_free_args(fuse->fargs);
            FUSEJS_THROW_EXCEPTION("Error setting fuse signal handlers: ", strerror(errno));
            return;
        }

        fuse_session_add_chan(fuse->session, fuse->channel);
        if(fuse->multithreaded){
            ret = fuse_session_loop_mt(fuse->session); //blocks here
        }else{
            ret = fuse_session_loop(fuse->session); //blocks here            
        }

        //Continues executing if user unmounts the fs
        fuse_remove_signal_handlers(fuse->session);
        fuse_session_remove_chan(fuse->channel);
        fuse_session_destroy(fuse->session);
        fuse_unmount(fuse->mountpoint, fuse->channel);
        fuse_opt_free_args(fuse->fargs);
        uv_thread_join(&(fuse->fuse_thread));
        //close the event loop
        uv_close( (uv_handle_t*) &uv_async_handle, NULL );

        if (ret == -1) {
            FUSEJS_THROW_EXCEPTION("Error starting fuse session loop: ", strerror(errno));
            return;
        }

        return;
    }
    
    NAN_METHOD(Fuse::Mount) {
        Nan::EscapableHandleScope scope;

        int argslen = info.Length();

        if (argslen == 0) {
            Nan::ThrowError(
            "You must specify arguments to invoke this function");
            return;
        }

        if (!info[0]->IsObject()) {
            Nan::ThrowError(
            "You must specify an Object as first argument");
            return;
        }

        Local<Object> argsObj = info[0]->ToObject();
        // THROW_IF_MISSING_PROPERTY(argsObj, "mountpoint");
        THROW_IF_MISSING_PROPERTY(argsObj, "filesystem");
        THROW_IF_MISSING_PROPERTY(argsObj, "options");

        //Local<Value> vmountpoint = Nan::Get(argsObj, mountpoint_sym);
        Local<Value> vfilesystem = Nan::Get(argsObj, Nan::New<String>("filesystem").ToLocalChecked()).ToLocalChecked();
        Local<Value> voptions = Nan::Get(argsObj, Nan::New<String>("options").ToLocalChecked()).ToLocalChecked();

        if (!vfilesystem->IsFunction()) {
            Nan::ThrowError(
                "Wrong type for property 'filesystem', a Function is expected");
            return;
        }

        if (!voptions->IsArray()) {
            Nan::ThrowError(
                "Wrong type for property 'options', an Array is expected");
            return;
        }

        Local<Array> options = Local<Array>::Cast(voptions);
        int argc = options->Length();

        //If no mountpoint is provided, show usage.
        if (argc < 1) {
            Nan::Set(options, Nan::New<Integer>( static_cast<uint32_t>(1) ), Nan::New<String>("--help").ToLocalChecked());
            argc++;
        }

        Local<Object> currentInstance = info.This();
        Fuse *fuse  = ObjectWrap::Unwrap<Fuse>(currentInstance);
        struct fuse_args fargs = FUSE_ARGS_INIT(0, NULL);
        fuse->fargs = (struct fuse_args *)malloc(sizeof(fargs));
        memcpy( fuse->fargs, &fargs, sizeof(fargs) );   

        for (int i = 0; i < argc; i++) {
            v8::String::Utf8Value option(Nan::Get(options, Nan::New<Integer>( static_cast<uint32_t>(i)) ).ToLocalChecked()->ToString() );
            char *fopt = strdup(*option);
            if (fuse_opt_add_arg(fuse->fargs, (const char *) fopt) == -1) {
                FUSEJS_THROW_EXCEPTION("Unable to allocate memory, fuse_opt_add_arg failed: ", strerror(errno));
                return;
            }
        }

        int ret = fuse_parse_cmdline(fuse->fargs, &fuse->mountpoint,
                                        &fuse->multithreaded, &fuse->foreground);
        if (ret == -1) {
            FUSEJS_THROW_EXCEPTION("Error parsing fuse options: ", strerror(errno));
            return;
        }

        if (!fuse->mountpoint) {
            FUSEJS_THROW_EXCEPTION("Mount point argument was not found", "");
            return;
        }

        fuse->channel = fuse_mount((const char*) fuse->mountpoint, fuse->fargs);
        if (fuse->channel == NULL) {
            FUSEJS_THROW_EXCEPTION("Unable to mount filesystem: ", strerror(errno));
            return;
        }

        Local<Value> argv[2] = {
            currentInstance,
            options
        };

        Local<Function> filesystem = Local<Function>::Cast(vfilesystem);
        
        //NanAssignPersistent(fuse->fsobj, NanNew(filesystem)->NewInstance(2, argv) );

        fuse->fsobj.Reset( filesystem->NewInstance(2,argv) );
        assert(Nan::New(fuse->fsobj)->IsObject());
        assert(
            Nan::Get( Nan::New(fuse->fsobj), 
                Nan::New("init").ToLocalChecked()).ToLocalChecked()->IsFunction()
        );

        uv_async_init(uv_default_loop(), &uv_async_handle, (uv_async_cb) FileSystem::DispatchOp);
         uv_thread_create(&(fuse->fuse_thread), Fuse::RemoteMount, (void *) fuse);


        scope.Escape(currentInstance);
    }



    void Fuse::Unmount(const Nan::FunctionCallbackInfo<v8::Value>& args) {
        Nan::EscapableHandleScope scope;
        Local<Object> currentInstance = args.This();
        Fuse *fuse = ObjectWrap::Unwrap<Fuse>(currentInstance);
        // fuse_session_exit(fuse->session);
        // printf("unmount called");
        // fuse_session_remove_chan(fuse->channel);
        // printf("unmount called");
        // fuse_remove_signal_handlers(fuse->session);
        // printf("unmount called");
        // fuse_unmount(fuse->mountpoint, fuse->channel);
        // printf("unmount called");
        // fuse_session_remove_chan(fuse->channel);
        // printf("unmount called");
        // fuse_session_destroy(fuse->session);

        fuse_session_exit(fuse->session);

        scope.Escape(currentInstance);

    }
} //namespace NodeFuse

