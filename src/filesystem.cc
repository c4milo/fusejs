#include "filesystem.h"
#include "reply.h"
#include "bindings.h"

namespace NodeFuse {

    static struct fuse_lowlevel_ops fuse_ops = {
        init    : FileSystem::Init,
        destroy : FileSystem::Destroy,
        lookup  : FileSystem::Lookup,
        forget  : FileSystem::Forget,
        getattr : FileSystem::GetAttr,
        setattr : FileSystem::SetAttr,
        readlink: FileSystem::ReadLink,
        mknod   : FileSystem::MkNod,
        mkdir   : FileSystem::MkDir,
        unlink  : FileSystem::Unlink,
        rmdir   : FileSystem::RmDir,
        symlink : FileSystem::SymLink
    };

    //Operations symbols
    static Persistent<String> init_sym      = NODE_PSYMBOL("init");
    static Persistent<String> destroy_sym   = NODE_PSYMBOL("destroy");
    static Persistent<String> lookup_sym    = NODE_PSYMBOL("lookup");
    static Persistent<String> forget_sym    = NODE_PSYMBOL("forget");
    static Persistent<String> getattr_sym   = NODE_PSYMBOL("getattr");
    static Persistent<String> setattr_sym   = NODE_PSYMBOL("setattr");
    static Persistent<String> readlink_sym  = NODE_PSYMBOL("readlink");
    static Persistent<String> mknod_sym     = NODE_PSYMBOL("mknod");
    static Persistent<String> mkdir_sym     = NODE_PSYMBOL("mkdir");
    static Persistent<String> unlink_sym    = NODE_PSYMBOL("unlink");
    static Persistent<String> rmdir_sym     = NODE_PSYMBOL("rmdir");
    static Persistent<String> symlink_sym   = NODE_PSYMBOL("symlink");

    //fuse_conn_info symbols
    //Major version of the fuse protocol
    static Persistent<String> conn_info_proto_major_sym     = NODE_PSYMBOL("proto_major");
    //Minor version of the fuse protocol
    static Persistent<String> conn_info_proto_minor_sym     = NODE_PSYMBOL("proto_minor");
    //Is asynchronous read supported
    static Persistent<String> conn_info_async_read_sym      = NODE_PSYMBOL("async_read");
    //Maximum size of the write buffer
    static Persistent<String> conn_info_max_write_sym       = NODE_PSYMBOL("max_write");
    //Maximum readahead
    static Persistent<String> conn_info_max_readahead_sym   = NODE_PSYMBOL("max_readahead");
    //Capability flags, that the kernel supports
    static Persistent<String> conn_info_capable_sym         = NODE_PSYMBOL("capable");
    //Capability flags, that the filesystem wants to enable
    static Persistent<String> conn_info_want_sym            = NODE_PSYMBOL("want");


    void FileSystem::Init(void* userdata,
                          struct fuse_conn_info* conn) {
        HandleScope scope;
        Fuse* fuse = static_cast<Fuse *>(userdata);

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

    void FileSystem::Destroy(void* userdata) {
        HandleScope scope;
        Fuse* fuse = static_cast<Fuse *>(userdata);

        Local<Value> vdestroy = fuse->fsobj->Get(destroy_sym);
        Local<Function> destroy = Local<Function>::Cast(vdestroy);

        TryCatch try_catch;

        destroy->Call(fuse->fsobj, 0, NULL);

        if (try_catch.HasCaught()) {
            FatalException(try_catch);
        }
    }

    void FileSystem::Lookup(fuse_req_t req,
                            fuse_ino_t parent,
                            const char* name) {
        HandleScope scope;
        Fuse* fuse = static_cast<Fuse *>(fuse_req_userdata(req));

        Local<Value> vlookup = fuse->fsobj->Get(lookup_sym);
        Local<Function> lookup = Local<Function>::Cast(vlookup);

        Local<Object> context = RequestContextToObject(fuse_req_ctx(req))->ToObject();
        Local<Number> parentInode = Number::New(parent);
        Local<String> entryName = String::New(name);

        Reply* reply = new Reply();
        reply->request = req;
        Local<Object> replyObj = reply->constructor_template->GetFunction()->NewInstance();
        reply->Wrap(replyObj);

        Local<Value> argv[4] = {context, parentInode,
                                entryName, replyObj};
        TryCatch try_catch;

        lookup->Call(fuse->fsobj, 4, argv);

        if (try_catch.HasCaught()) {
            FatalException(try_catch);
        }
    }

    void FileSystem::Forget(fuse_req_t req,
                            fuse_ino_t ino,
                            unsigned long nlookup) {
        HandleScope scope;
        Fuse* fuse = static_cast<Fuse *>(fuse_req_userdata(req));

        Local<Value> vforget = fuse->fsobj->Get(forget_sym);
        Local<Function> forget = Local<Function>::Cast(vforget);

        Local<Object> context = RequestContextToObject(fuse_req_ctx(req))->ToObject();
        Local<Number> inode = Number::New(ino);
        Local<Integer> nlookup_ = Integer::New(nlookup);

        Local<Value> argv[3] = {context, inode, nlookup_};

        TryCatch try_catch;

        forget->Call(fuse->fsobj, 3, argv);

        if (try_catch.HasCaught()) {
            FatalException(try_catch);
        }

        fuse_reply_none(req);
    }

    void FileSystem::GetAttr(fuse_req_t req,
                             fuse_ino_t ino,
                             struct fuse_file_info* fi) {
        HandleScope scope;
        Fuse* fuse = static_cast<Fuse *>(fuse_req_userdata(req));

        Local<Value> vgetattr = fuse->fsobj->Get(getattr_sym);
        Local<Function> getattr = Local<Function>::Cast(vgetattr);

        Local<Object> context = RequestContextToObject(fuse_req_ctx(req))->ToObject();
        Local<Number> inode = Number::New(ino);

        Reply* reply = new Reply();
        reply->request = req;
        Local<Object> replyObj = reply->constructor_template->GetFunction()->NewInstance();
        reply->Wrap(replyObj);

        Local<Value> argv[3] = {context, inode, replyObj};

        TryCatch try_catch;

        getattr->Call(fuse->fsobj, 3, argv);

        if (try_catch.HasCaught()) {
            FatalException(try_catch);
        }
    }

    void FileSystem::SetAttr(fuse_req_t req,
                             fuse_ino_t ino,
                             struct stat* attr,
                             int to_set,
                             struct fuse_file_info* fi) {
        HandleScope scope;
        Fuse *fuse = static_cast<Fuse *>(fuse_req_userdata(req));

        Local<Value> vsetattr = fuse->fsobj->Get(setattr_sym);
        Local<Function> setattr = Local<Function>::Cast(vsetattr);

        Local<Object> context = RequestContextToObject(fuse_req_ctx(req))->ToObject();
        Local<Number> inode = Number::New(ino);

        Local<Object> attrs = GetAttrsToBeSet(to_set, attr)->ToObject();

        Reply *reply = new Reply();
        reply->request = req;
        Local<Object> replyObj = reply->constructor_template->GetFunction()->NewInstance();
        reply->Wrap(replyObj);

        Local<Value> argv[4] = {context, inode, attrs, replyObj};

        TryCatch try_catch;

        setattr->Call(fuse->fsobj, 4, argv);

        if (try_catch.HasCaught()) {
            FatalException(try_catch);
        }
    }

    void FileSystem::ReadLink(fuse_req_t req, fuse_ino_t ino) {
        HandleScope scope;
        Fuse* fuse = static_cast<Fuse *>(fuse_req_userdata(req));

        Local<Value> vreadlink = fuse->fsobj->Get(readlink_sym);
        Local<Function> readlink = Local<Function>::Cast(vreadlink);

        Local<Object> context = RequestContextToObject(fuse_req_ctx(req))->ToObject();
        Local<Number> inode = Number::New(ino);

        Reply* reply = new Reply();
        reply->request = req;
        Local<Object> replyObj = reply->constructor_template->GetFunction()->NewInstance();
        reply->Wrap(replyObj);

        Local<Value> argv[3] = {context, inode, replyObj};

        TryCatch try_catch;

        readlink->Call(fuse->fsobj, 3, argv);

        if (try_catch.HasCaught()) {
            FatalException(try_catch);
        }
    }

    void FileSystem::MkNod(fuse_req_t req,
                           fuse_ino_t parent,
                           const char* name,
                           mode_t mode,
                           dev_t rdev) {
        HandleScope scope;
        Fuse* fuse = static_cast<Fuse *>(fuse_req_userdata(req));

        Local<Value> vmknod = fuse->fsobj->Get(mknod_sym);
        Local<Function> mknod = Local<Function>::Cast(vmknod);

        Local<Object> context = RequestContextToObject(fuse_req_ctx(req))->ToObject();
        Local<Number> parentInode = Number::New(parent);

        Local<String> name_ = String::New(name);
        Local<Integer> mode_ = Integer::New(mode);
        Local<Integer> rdev_ = Integer::New(rdev);

        Reply* reply = new Reply();
        reply->request = req;
        Local<Object> replyObj = reply->constructor_template->GetFunction()->NewInstance();
        reply->Wrap(replyObj);

        Local<Value> argv[6] = {context, parentInode,
                                name_, mode_,
                                rdev_, replyObj};

        TryCatch try_catch;

        mknod->Call(fuse->fsobj, 6, argv);

        if (try_catch.HasCaught()) {
            FatalException(try_catch);
        }
    }

    void FileSystem::MkDir(fuse_req_t req,
                           fuse_ino_t parent,
                           const char* name,
                           mode_t mode) {
        HandleScope scope;
        Fuse* fuse = static_cast<Fuse *>(fuse_req_userdata(req));

        Local<Value> vmkdir = fuse->fsobj->Get(mkdir_sym);
        Local<Function> mkdir = Local<Function>::Cast(vmkdir);

        Local<Object> context = RequestContextToObject(fuse_req_ctx(req))->ToObject();
        Local<Number> parentInode = Number::New(parent);

        Local<String> name_ = String::New(name);
        Local<Integer> mode_ = Integer::New(mode);

        Reply* reply = new Reply();
        reply->request = req;
        Local<Object> replyObj = reply->constructor_template->GetFunction()->NewInstance();
        reply->Wrap(replyObj);

        Local<Value> argv[5] = {context, parentInode,
                                name_, mode_, replyObj};

        TryCatch try_catch;

        mkdir->Call(fuse->fsobj, 5, argv);

        if (try_catch.HasCaught()) {
            FatalException(try_catch);
        }
    }

    void FileSystem::Unlink(fuse_req_t req,
                            fuse_ino_t parent,
                            const char* name) {
        HandleScope scope;
        Fuse* fuse = static_cast<Fuse *>(fuse_req_userdata(req));

        Local<Value> vunlink = fuse->fsobj->Get(unlink_sym);
        Local<Function> unlink = Local<Function>::Cast(vunlink);

        Local<Object> context = RequestContextToObject(fuse_req_ctx(req))->ToObject();
        Local<Number> parentInode = Number::New(parent);
        Local<String> name_ = String::New(name);

        Reply* reply = new Reply();
        reply->request = req;
        Local<Object> replyObj = reply->constructor_template->GetFunction()->NewInstance();
        reply->Wrap(replyObj);

        Local<Value> argv[4] = {context, parentInode, name_, replyObj};

        TryCatch try_catch;

        unlink->Call(fuse->fsobj, 4, argv);

        if (try_catch.HasCaught()) {
            FatalException(try_catch);
        }
    }

    void FileSystem::RmDir(fuse_req_t req,
                           fuse_ino_t parent,
                           const char* name) {
        HandleScope scope;
        Fuse* fuse = static_cast<Fuse *>(fuse_req_userdata(req));

        Local<Value> vrmdir = fuse->fsobj->Get(rmdir_sym);
        Local<Function> rmdir = Local<Function>::Cast(vrmdir);

        Local<Object> context = RequestContextToObject(fuse_req_ctx(req))->ToObject();
        Local<Number> parentInode = Number::New(parent);
        Local<String> name_ = String::New(name);

        Reply* reply = new Reply();
        reply->request = req;
        Local<Object> replyObj = reply->constructor_template->GetFunction()->NewInstance();
        reply->Wrap(replyObj);

        Local<Value> argv[4] = {context, parentInode, name_, replyObj};

        TryCatch try_catch;

        rmdir->Call(fuse->fsobj, 4, argv);

        if (try_catch.HasCaught()) {
            FatalException(try_catch);
        }
    }

    void FileSystem::SymLink(fuse_req_t req,
                             const char* link,
                             fuse_ino_t parent,
                             const char* name) {
        HandleScope scope;
        Fuse* fuse = static_cast<Fuse *>(fuse_req_userdata(req));

        Local<Value> vsymlink = fuse->fsobj->Get(symlink_sym);
        Local<Function> symlink = Local<Function>::Cast(vsymlink);

        Local<Object> context = RequestContextToObject(fuse_req_ctx(req))->ToObject();
        Local<Number> parentInode = Number::New(parent);
        Local<String> name_ = String::New(name);
        Local<String> link_ = String::New(link);

        Reply* reply = new Reply();
        reply->request = req;
        Local<Object> replyObj = reply->constructor_template->GetFunction()->NewInstance();
        reply->Wrap(replyObj);

        Local<Value> argv[5] = {context, parentInode, link_, name_, replyObj};

        TryCatch try_catch;

        symlink->Call(fuse->fsobj, 5, argv);

        if (try_catch.HasCaught()) {
            FatalException(try_catch);
        }
    }


    struct fuse_lowlevel_ops* FileSystem::GetOperations() {
        return &fuse_ops;
    }
}


