#include "filesystem.h"
#include "bindings.h"

namespace NodeFuse {

    static struct fuse_lowlevel_ops fuse_ops = {
        init: FileSystem::Init,
        destroy: FileSystem::Destroy,
        lookup: FileSystem::Lookup,
        forget: FileSystem::Forget,
        getattr: FileSystem::GetAttr
    };

    //Operations symbols
    static Persistent<String> init_sym = NODE_PSYMBOL("init");
    static Persistent<String> destroy_sym = NODE_PSYMBOL("destroy");
    static Persistent<String> lookup_sym = NODE_PSYMBOL("lookup");
    static Persistent<String> forget_sym = NODE_PSYMBOL("forget");
    static Persistent<String> getattr_sym = NODE_PSYMBOL("getattr");

    //fuse_conn_info symbols
    //Major version of the fuse protocol
    static Persistent<String> conn_info_proto_major_sym = NODE_PSYMBOL("proto_major");
    //Minor version of the fuse protocol
    static Persistent<String> conn_info_proto_minor_sym = NODE_PSYMBOL("proto_minor");
    //Is asynchronous read supported
    static Persistent<String> conn_info_async_read_sym = NODE_PSYMBOL("async_read");
    //Maximum size of the write buffer
    static Persistent<String> conn_info_max_write_sym = NODE_PSYMBOL("max_write");
    //Maximum readahead
    static Persistent<String> conn_info_max_readahead_sym = NODE_PSYMBOL("max_readahead");
    //Capability flags, that the kernel supports
    static Persistent<String> conn_info_capable_sym = NODE_PSYMBOL("capable");
    //Capability flags, that the filesystem wants to enable
    static Persistent<String> conn_info_want_sym = NODE_PSYMBOL("want");


    void FileSystem::Init(void *userdata, struct fuse_conn_info *conn) {
        HandleScope scope;
        Fuse *fuse = static_cast<Fuse *>(userdata);

        Local<Value> vinit = fuse->fsobj->Get(init_sym);
        Local<Function> init = Local<Function>::Cast(vinit);

        //These properties will be read-only for now.
        Local<Object> info = Object::New();
        info->Set(conn_info_proto_major_sym, Integer::New(conn->proto_major));
        info->Set(conn_info_proto_minor_sym, Integer::New(conn->proto_minor));
        info->Set(conn_info_async_read_sym, Integer::New(conn->async_read));
        info->Set(conn_info_max_write_sym, Number::New(conn->max_write));
        info->Set(conn_info_max_readahead_sym, Number::New(conn->max_readahead));
        //TODO macro to enable certain properties given the fuse version
        //info->Set(conn_info_capable_sym, Integer::New(conn->capable));
        //info->Set(conn_info_want_sym, Integer::New(conn->want));

        Local<Value> argv[1] = {info};

        TryCatch try_catch;

        init->Call(fuse->fsobj, 1, argv);

        if (try_catch.HasCaught()) {
            FatalException(try_catch);
        }

    }

    void FileSystem::Destroy(void *userdata) {
        HandleScope scope;
        Fuse *fuse = static_cast<Fuse *>(userdata);

        Local<Value> vdestroy = fuse->fsobj->Get(destroy_sym);
        Local<Function> destroy = Local<Function>::Cast(vdestroy);

        TryCatch try_catch;

        destroy->Call(fuse->fsobj, 0, NULL);

        if (try_catch.HasCaught()) {
            FatalException(try_catch);
        }
    }

    void FileSystem::Lookup(fuse_req_t req, fuse_ino_t parent, const char *name) {
        HandleScope scope;
        Fuse *fuse = static_cast<Fuse *>(fuse_req_userdata(req));

        Local<Value> vlookup = fuse->fsobj->Get(lookup_sym);
        Local<Function> lookup = Local<Function>::Cast(vlookup);

        Local<Object> context = RequestContextToObject(fuse_req_ctx(req))->ToObject();
        Local<Number> parentInode = Number::New(parent);
        Local<String> entryName = String::New(name);

        Local<Value> argv[3] = {context, parentInode, entryName};

        TryCatch try_catch;

        lookup->Call(fuse->fsobj, 3, argv);

        if (try_catch.HasCaught()) {
            FatalException(try_catch);
        }
    }

    void FileSystem::Forget(fuse_req_t req, fuse_ino_t ino, unsigned long nlookup) {
        Fuse *fuse = static_cast<Fuse *>(fuse_req_userdata(req));

        Local<Value> vforget = fuse->fsobj->Get(forget_sym);
        Local<Function> forget = Local<Function>::Cast(vforget);

        TryCatch try_catch;

        forget->Call(fuse->fsobj, 0, NULL);

        if (try_catch.HasCaught()) {
            FatalException(try_catch);
        }
    }

    void FileSystem::GetAttr(fuse_req_t req, fuse_ino_t ino, struct fuse_file_info *fi) {

    }

    struct fuse_lowlevel_ops* FileSystem::GetOperations() {
        return &fuse_ops;
    }
}


