#include "filesystem.h"
#include "reply.h"
#include "file_info.h"
#include "bindings.h"
#include "node_buffer.h"

namespace NodeFuse {

    static struct fuse_lowlevel_ops fuse_ops = {};

    //Symbols for FUSE operations
    static Persistent<String> init_sym        = NODE_PSYMBOL("init");
    static Persistent<String> destroy_sym     = NODE_PSYMBOL("destroy");
    static Persistent<String> lookup_sym      = NODE_PSYMBOL("lookup");
    static Persistent<String> forget_sym      = NODE_PSYMBOL("forget");
    static Persistent<String> getattr_sym     = NODE_PSYMBOL("getattr");
    static Persistent<String> setattr_sym     = NODE_PSYMBOL("setattr");
    static Persistent<String> readlink_sym    = NODE_PSYMBOL("readlink");
    static Persistent<String> mknod_sym       = NODE_PSYMBOL("mknod");
    static Persistent<String> mkdir_sym       = NODE_PSYMBOL("mkdir");
    static Persistent<String> unlink_sym      = NODE_PSYMBOL("unlink");
    static Persistent<String> rmdir_sym       = NODE_PSYMBOL("rmdir");
    static Persistent<String> symlink_sym     = NODE_PSYMBOL("symlink");
    static Persistent<String> rename_sym      = NODE_PSYMBOL("rename");
    static Persistent<String> link_sym        = NODE_PSYMBOL("link");
    static Persistent<String> open_sym        = NODE_PSYMBOL("open");
    static Persistent<String> read_sym        = NODE_PSYMBOL("read");
    static Persistent<String> write_sym       = NODE_PSYMBOL("write");
    static Persistent<String> flush_sym       = NODE_PSYMBOL("flush");
    static Persistent<String> release_sym     = NODE_PSYMBOL("release");
    static Persistent<String> fsync_sym       = NODE_PSYMBOL("fsync");
    static Persistent<String> opendir_sym     = NODE_PSYMBOL("opendir");
    static Persistent<String> readdir_sym     = NODE_PSYMBOL("readdir");
    static Persistent<String> releasedir_sym  = NODE_PSYMBOL("releasedir");
    static Persistent<String> fsyncdir_sym    = NODE_PSYMBOL("fsyncdir");
    static Persistent<String> statfs_sym      = NODE_PSYMBOL("statfs");
    static Persistent<String> setxattr_sym    = NODE_PSYMBOL("setxattr");
    static Persistent<String> getxattr_sym    = NODE_PSYMBOL("getxattr");
    static Persistent<String> listxattr_sym   = NODE_PSYMBOL("listxattr");
    static Persistent<String> removexattr_sym = NODE_PSYMBOL("removexattr");
    static Persistent<String> access_sym      = NODE_PSYMBOL("access");
    static Persistent<String> create_sym      = NODE_PSYMBOL("create");
    static Persistent<String> getlk_sym       = NODE_PSYMBOL("getlk");
    static Persistent<String> setlk_sym       = NODE_PSYMBOL("setlk");
    static Persistent<String> bmap_sym        = NODE_PSYMBOL("bmap");
    static Persistent<String> ioctl_sym       = NODE_PSYMBOL("ioctl");
    static Persistent<String> poll_sym        = NODE_PSYMBOL("poll");

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

    void FileSystem::Initialize() {
        fuse_ops.init       = FileSystem::Init;
        fuse_ops.destroy    = FileSystem::Destroy;
        fuse_ops.lookup     = FileSystem::Lookup;
        fuse_ops.forget     = FileSystem::Forget;
        fuse_ops.getattr    = FileSystem::GetAttr;
        fuse_ops.setattr    = FileSystem::SetAttr;
        // fuse_ops.readlink   = FileSystem::ReadLink;
        // fuse_ops.mknod      = FileSystem::MkNod;
        fuse_ops.mkdir      = FileSystem::MkDir;
        fuse_ops.unlink     = FileSystem::Unlink;
        fuse_ops.rmdir      = FileSystem::RmDir;
        // fuse_ops.symlink    = FileSystem::SymLink;
        fuse_ops.rename     = FileSystem::Rename;
        fuse_ops.link       = FileSystem::Link;
        fuse_ops.open       = FileSystem::Open;
        fuse_ops.read       = FileSystem::Read;
        fuse_ops.write      = FileSystem::Write;
        fuse_ops.flush      = FileSystem::Flush;
        fuse_ops.release    = FileSystem::Release;
        // fuse_ops.fsync      = FileSystem::FSync;
        fuse_ops.opendir    = FileSystem::OpenDir;
        fuse_ops.readdir    = FileSystem::ReadDir;
        // fuse_ops.releasedir = FileSystem::ReleaseDir;
        // fuse_ops.fsyncdir   = FileSystem::FSyncDir;
        fuse_ops.statfs     = FileSystem::StatFs;
        // fuse_ops.setxattr   = FileSystem::SetXAttr;
        // fuse_ops.getxattr   = FileSystem::GetXAttr;
        // fuse_ops.listxattr  = FileSystem::ListXAttr;
        // fuse_ops.removexattr= FileSystem::RemoveXAttr;
        fuse_ops.access     = FileSystem::Access;
        fuse_ops.create     = FileSystem::Create;
        // fuse_ops.getlk      = FileSystem::GetLock;
        // fuse_ops.setlk      = FileSystem::SetLock;
        // fuse_ops.bmap       = FileSystem::BMap;
        //fuse_ops.ioctl      = FileSystem::IOCtl;
        //fuse_ops.poll       = FileSystem::Poll;
    }

    void FileSystem::Init(void* userdata,
                          struct fuse_conn_info* conn) {
        HandleScope scope;
        Fuse* fuse = static_cast<Fuse *>(userdata);

        Local<Value> vinit = fuse->fsobj->Get(init_sym);
        Local<Function> init = Local<Function>::Cast(vinit);

        //These properties will be read-only for now.
        //TODO set accessors for read/write properties
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

        Local<Value> argv[5] = {context, parentInode,
                                link_, name_, replyObj};

        TryCatch try_catch;

        symlink->Call(fuse->fsobj, 5, argv);

        if (try_catch.HasCaught()) {
            FatalException(try_catch);
        }
    }

    void FileSystem::Rename(fuse_req_t req,
                            fuse_ino_t parent,
                            const char *name,
                            fuse_ino_t newparent,
                            const char *newname) {
        HandleScope scope;
        Fuse* fuse = static_cast<Fuse *>(fuse_req_userdata(req));

        Local<Value> vrename = fuse->fsobj->Get(rename_sym);
        Local<Function> rename = Local<Function>::Cast(vrename);

        Local<Object> context = RequestContextToObject(fuse_req_ctx(req))->ToObject();
        Local<Number> parentInode = Number::New(parent);
        Local<String> name_ = String::New(name);
        Local<Number> newParentInode = Number::New(newparent);
        Local<String> newName = String::New(newname);

        Reply* reply = new Reply();
        reply->request = req;
        Local<Object> replyObj = reply->constructor_template->GetFunction()->NewInstance();
        reply->Wrap(replyObj);

        Local<Value> argv[6] = {context, parentInode,
                                name_, newParentInode,
                                newName, replyObj};

        TryCatch try_catch;

        rename->Call(fuse->fsobj, 6, argv);

        if (try_catch.HasCaught()) {
            FatalException(try_catch);
        }
    }

    void FileSystem::Link(fuse_req_t req,
                          fuse_ino_t ino,
                          fuse_ino_t newparent,
                          const char* newname) {
        HandleScope scope;
        Fuse* fuse = static_cast<Fuse *>(fuse_req_userdata(req));

        Local<Value> vlink = fuse->fsobj->Get(link_sym);
        Local<Function> link = Local<Function>::Cast(vlink);

        Local<Object> context = RequestContextToObject(fuse_req_ctx(req))->ToObject();
        Local<Number> inode = Number::New(ino);
        Local<Number> newParent = Number::New(newparent);
        Local<String> newName = String::New(newname);

        Reply* reply = new Reply();
        reply->request = req;
        Local<Object> replyObj = reply->constructor_template->GetFunction()->NewInstance();
        reply->Wrap(replyObj);

        Local<Value> argv[5] = {context, inode,
                                newParent, newName, replyObj};

        TryCatch try_catch;

        link->Call(fuse->fsobj, 5, argv);

        if (try_catch.HasCaught()) {
            FatalException(try_catch);
        }
    }

    void FileSystem::Open(fuse_req_t req,
                          fuse_ino_t ino,
                          struct fuse_file_info* fi) {
        HandleScope scope;
        Fuse* fuse = static_cast<Fuse *>(fuse_req_userdata(req));

        Local<Value> vopen = fuse->fsobj->Get(open_sym);
        Local<Function> open = Local<Function>::Cast(vopen);

        Local<Object> context = RequestContextToObject(fuse_req_ctx(req))->ToObject();
        Local<Number> inode = Number::New(ino);

        FileInfo* info = new FileInfo();
        info->fi = fi;
        Local<Object> infoObj = info->constructor_template->GetFunction()->NewInstance();
        info->Wrap(infoObj);

        Reply* reply = new Reply();
        reply->request = req;
        Local<Object> replyObj = reply->constructor_template->GetFunction()->NewInstance();
        reply->Wrap(replyObj);

        Local<Value> argv[4] = {context, inode,
                                infoObj, replyObj};

        TryCatch try_catch;

        open->Call(fuse->fsobj, 4, argv);

        if (try_catch.HasCaught()) {
            FatalException(try_catch);
        }
    }

    void FileSystem::Read(fuse_req_t req,
                          fuse_ino_t ino,
                          size_t size_,
                          off_t off,
                          struct fuse_file_info* fi) {
        HandleScope scope;
        Fuse* fuse = static_cast<Fuse *>(fuse_req_userdata(req));

        Local<Value> vread = fuse->fsobj->Get(read_sym);
        Local<Function> read = Local<Function>::Cast(vread);

        Local<Object> context = RequestContextToObject(fuse_req_ctx(req))->ToObject();
        Local<Number> inode = Number::New(ino);
        Local<Integer> size = Integer::New(size_);
        Local<Integer> offset = Integer::New(off);

        FileInfo* info = new FileInfo();
        info->fi = fi;
        Local<Object> infoObj = info->constructor_template->GetFunction()->NewInstance();
        info->Wrap(infoObj);

        Reply* reply = new Reply();
        reply->request = req;
        Local<Object> replyObj = reply->constructor_template->GetFunction()->NewInstance();
        reply->Wrap(replyObj);

        Local<Value> argv[6] = {context, inode,
                                size, offset,
                                infoObj, replyObj};

        TryCatch try_catch;

        read->Call(fuse->fsobj, 6, argv);

        if (try_catch.HasCaught()) {
            FatalException(try_catch);
        }
    }

    void FileSystem::Write(fuse_req_t req,
                           fuse_ino_t ino,
                           const char *buf,
                           size_t size,
                           off_t off,
                           struct fuse_file_info* fi) {
        HandleScope scope;
        Fuse* fuse = static_cast<Fuse *>(fuse_req_userdata(req));

        Local<Value> vwrite = fuse->fsobj->Get(write_sym);
        Local<Function> write = Local<Function>::Cast(vwrite);

        Local<Object> context = RequestContextToObject(fuse_req_ctx(req))->ToObject();
        Local<Number> inode = Number::New(ino);
        Local<Integer> offset = Integer::New(off);

        Buffer* buffer = Buffer::New((char*) buf, size);

        FileInfo* info = new FileInfo();
        info->fi = fi;
        Local<Object> infoObj = info->constructor_template->GetFunction()->NewInstance();
        info->Wrap(infoObj);

        Reply* reply = new Reply();
        reply->request = req;
        Local<Object> replyObj = reply->constructor_template->GetFunction()->NewInstance();
        reply->Wrap(replyObj);

        Local<Value> argv[6] = {context, inode,
                                Local<Object>::New(buffer->handle_), offset,
                                infoObj, replyObj};

        TryCatch try_catch;
        //fprintf(stderr, "AAHHHHHHHHHHHHHHHH %d\n", ino);
        write->Call(fuse->fsobj, 6, argv);

        if (try_catch.HasCaught()) {
            FatalException(try_catch);
        }
    }

    void FileSystem::Flush(fuse_req_t req,
                           fuse_ino_t ino,
                           struct fuse_file_info* fi) {
        HandleScope scope;
        Fuse* fuse = static_cast<Fuse *>(fuse_req_userdata(req));

        Local<Value> vflush = fuse->fsobj->Get(flush_sym);
        Local<Function> flush = Local<Function>::Cast(vflush);

        Local<Object> context = RequestContextToObject(fuse_req_ctx(req))->ToObject();
        Local<Number> inode = Number::New(ino);

        FileInfo* info = new FileInfo();
        info->fi = fi;
        Local<Object> infoObj = info->constructor_template->GetFunction()->NewInstance();
        info->Wrap(infoObj);

        Reply* reply = new Reply();
        reply->request = req;
        Local<Object> replyObj = reply->constructor_template->GetFunction()->NewInstance();
        reply->Wrap(replyObj);

        Local<Value> argv[4] = {context, inode,
                                infoObj, replyObj};

        TryCatch try_catch;

        flush->Call(fuse->fsobj, 4, argv);

        if (try_catch.HasCaught()) {
            FatalException(try_catch);
        }
    }

    void FileSystem::Release(fuse_req_t req,
                             fuse_ino_t ino,
                             struct fuse_file_info* fi) {
        HandleScope scope;
        Fuse* fuse = static_cast<Fuse *>(fuse_req_userdata(req));

        Local<Value> vrelease = fuse->fsobj->Get(release_sym);
        Local<Function> release = Local<Function>::Cast(vrelease);

        Local<Object> context = RequestContextToObject(fuse_req_ctx(req))->ToObject();
        Local<Number> inode = Number::New(ino);

        FileInfo* info = new FileInfo();
        info->fi = fi;
        Local<Object> infoObj = info->constructor_template->GetFunction()->NewInstance();
        info->Wrap(infoObj);

        Reply* reply = new Reply();
        reply->request = req;
        Local<Object> replyObj = reply->constructor_template->GetFunction()->NewInstance();
        reply->Wrap(replyObj);

        Local<Value> argv[4] = {context, inode,
                                infoObj, replyObj};

        TryCatch try_catch;

        release->Call(fuse->fsobj, 4, argv);

        if (try_catch.HasCaught()) {
            FatalException(try_catch);
        }
    }

    void FileSystem::FSync(fuse_req_t req,
                           fuse_ino_t ino,
                           int datasync_,
                           struct fuse_file_info* fi) {
        HandleScope scope;
        Fuse* fuse = static_cast<Fuse *>(fuse_req_userdata(req));

        Local<Value> vfsync = fuse->fsobj->Get(fsync_sym);
        Local<Function> fsync = Local<Function>::Cast(vfsync);

        Local<Object> context = RequestContextToObject(fuse_req_ctx(req))->ToObject();
        Local<Number> inode = Number::New(ino);
        bool datasync = datasync_ == 0 ? false : true;

        FileInfo* info = new FileInfo();
        info->fi = fi;
        Local<Object> infoObj = info->constructor_template->GetFunction()->NewInstance();
        info->Wrap(infoObj);

        Reply* reply = new Reply();
        reply->request = req;
        Local<Object> replyObj = reply->constructor_template->GetFunction()->NewInstance();
        reply->Wrap(replyObj);

        Local<Value> argv[5] = {context, inode,
                                Boolean::New(datasync)->ToObject(),
                                infoObj, replyObj};

        TryCatch try_catch;

        fsync->Call(fuse->fsobj, 5, argv);

        if (try_catch.HasCaught()) {
            FatalException(try_catch);
        }
    }

    void FileSystem::OpenDir(fuse_req_t req,
                             fuse_ino_t ino,
                             struct fuse_file_info* fi) {
        HandleScope scope;
        Fuse* fuse = static_cast<Fuse *>(fuse_req_userdata(req));

        Local<Value> vopendir = fuse->fsobj->Get(opendir_sym);
        Local<Function> opendir = Local<Function>::Cast(vopendir);

        Local<Object> context = RequestContextToObject(fuse_req_ctx(req))->ToObject();
        Local<Number> inode = Number::New(ino);

        FileInfo* info = new FileInfo();
        info->fi = fi;
        Local<Object> infoObj = info->constructor_template->GetFunction()->NewInstance();
        info->Wrap(infoObj);

        Reply* reply = new Reply();
        reply->request = req;
        Local<Object> replyObj = reply->constructor_template->GetFunction()->NewInstance();
        reply->Wrap(replyObj);

        Local<Value> argv[4] = {context, inode,
                                infoObj, replyObj};

        TryCatch try_catch;

        opendir->Call(fuse->fsobj, 4, argv);

        if (try_catch.HasCaught()) {
            FatalException(try_catch);
        }
    }

    void FileSystem::ReadDir(fuse_req_t req,
                             fuse_ino_t ino,
                             size_t size_,
                             off_t off,
                             struct fuse_file_info* fi) {
        HandleScope scope;
        Fuse* fuse = static_cast<Fuse *>(fuse_req_userdata(req));

        Local<Value> vreaddir = fuse->fsobj->Get(readdir_sym);
        Local<Function> readdir = Local<Function>::Cast(vreaddir);

        Local<Object> context = RequestContextToObject(fuse_req_ctx(req))->ToObject();
        Local<Number> inode = Number::New(ino);
        Local<Integer> size = Integer::New(size_);
        Local<Integer> offset = Integer::New(off);

        FileInfo* info = new FileInfo();
        info->fi = fi;
        Local<Object> infoObj = info->constructor_template->GetFunction()->NewInstance();
        info->Wrap(infoObj);

        Reply* reply = new Reply();
        reply->request = req;
        Local<Object> replyObj = reply->constructor_template->GetFunction()->NewInstance();
        reply->Wrap(replyObj);

        Local<Value> argv[6] = {context, inode,
                                size, offset,
                                infoObj, replyObj};

        TryCatch try_catch;

        readdir->Call(fuse->fsobj, 6, argv);

        if (try_catch.HasCaught()) {
            FatalException(try_catch);
        }
    }

    void FileSystem::ReleaseDir(fuse_req_t req,
                                fuse_ino_t ino,
                                struct fuse_file_info* fi) {
        HandleScope scope;
        Fuse* fuse = static_cast<Fuse *>(fuse_req_userdata(req));

        Local<Value> vreleasedir = fuse->fsobj->Get(releasedir_sym);
        Local<Function> releasedir = Local<Function>::Cast(vreleasedir);

        Local<Object> context = RequestContextToObject(fuse_req_ctx(req))->ToObject();
        Local<Number> inode = Number::New(ino);

        FileInfo* info = new FileInfo();
        info->fi = fi;
        Local<Object> infoObj = info->constructor_template->GetFunction()->NewInstance();
        info->Wrap(infoObj);

        Reply* reply = new Reply();
        reply->request = req;
        Local<Object> replyObj = reply->constructor_template->GetFunction()->NewInstance();
        reply->Wrap(replyObj);

        Local<Value> argv[4] = {context, inode,
                                infoObj, replyObj};

        TryCatch try_catch;

        releasedir->Call(fuse->fsobj, 4, argv);

        if (try_catch.HasCaught()) {
            FatalException(try_catch);
        }
    }

    void FileSystem::FSyncDir(fuse_req_t req,
                              fuse_ino_t ino,
                              int datasync_,
                              struct fuse_file_info* fi) {
        HandleScope scope;
        Fuse* fuse = static_cast<Fuse *>(fuse_req_userdata(req));

        Local<Value> vfsyncdir = fuse->fsobj->Get(fsyncdir_sym);
        Local<Function> fsyncdir = Local<Function>::Cast(vfsyncdir);

        Local<Object> context = RequestContextToObject(fuse_req_ctx(req))->ToObject();
        Local<Number> inode = Number::New(ino);
        bool datasync = datasync_ == 0 ? false : true;

        FileInfo* info = new FileInfo();
        info->fi = fi;
        Local<Object> infoObj = info->constructor_template->GetFunction()->NewInstance();
        info->Wrap(infoObj);

        Reply* reply = new Reply();
        reply->request = req;
        Local<Object> replyObj = reply->constructor_template->GetFunction()->NewInstance();
        reply->Wrap(replyObj);

        Local<Value> argv[5] = {context, inode,
                                Boolean::New(datasync)->ToObject(),
                                infoObj, replyObj};

        TryCatch try_catch;

        fsyncdir->Call(fuse->fsobj, 5, argv);

        if (try_catch.HasCaught()) {
            FatalException(try_catch);
        }
    }

    void FileSystem::StatFs(fuse_req_t req, fuse_ino_t ino) {
        HandleScope scope;
        Fuse* fuse = static_cast<Fuse *>(fuse_req_userdata(req));

        Local<Value> vstatfs = fuse->fsobj->Get(statfs_sym);
        Local<Function> statfs = Local<Function>::Cast(vstatfs);

        Local<Object> context = RequestContextToObject(fuse_req_ctx(req))->ToObject();
        Local<Number> inode = Number::New(ino);

        Reply* reply = new Reply();
        reply->request = req;
        Local<Object> replyObj = reply->constructor_template->GetFunction()->NewInstance();
        reply->Wrap(replyObj);

        Local<Value> argv[3] = {context, inode, replyObj};

        TryCatch try_catch;

        statfs->Call(fuse->fsobj, 3, argv);

        if (try_catch.HasCaught()) {
            FatalException(try_catch);
        }
    }

    void FileSystem::SetXAttr(fuse_req_t req,
                              fuse_ino_t ino,
                              const char* name_,
                              const char* value_,
                              size_t size_,
#ifdef __APPLE__
                              int flags_,
                              uint32_t position_) {
#else
                              int flags_) {
#endif
        HandleScope scope;
        Fuse *fuse = static_cast<Fuse *>(fuse_req_userdata(req));

        Local<Value> vsetxattr = fuse->fsobj->Get(setxattr_sym);
        Local<Function> setxattr = Local<Function>::Cast(vsetxattr);

        Local<Object> context = RequestContextToObject(fuse_req_ctx(req))->ToObject();
        Local<Number> inode = Number::New(ino);
        Local<String> name = String::New(name_);
        Local<String> value = String::New(value_);
#ifdef __APPLE__
        Local<Integer> position = Integer::New(position_);
#endif
        Local<Number> size = Number::New(size_);

        //TODO change for an object with accessors
        Local<Integer> flags = Integer::New(flags_);

        Reply *reply = new Reply();
        reply->request = req;
        Local<Object> replyObj = reply->constructor_template->GetFunction()->NewInstance();
        reply->Wrap(replyObj);

#ifdef __APPLE__
        Local<Value> argv[8] = {context, inode,
                                name, value,
                                size, flags,
                                position, replyObj};
#else
        Local<Value> argv[7] = {context, inode,
                                name, value,
                                size, flags, replyObj};

#endif
        TryCatch try_catch;

#ifdef __APPLE__
        setxattr->Call(fuse->fsobj, 8, argv);
#else
        setxattr->Call(fuse->fsobj, 7, argv);
#endif

        if (try_catch.HasCaught()) {
            FatalException(try_catch);
        }
    }

    void FileSystem::GetXAttr(fuse_req_t req,
                              fuse_ino_t ino,
                              const char* name_,
                              size_t size_
#ifdef __APPLE__
                              ,uint32_t position_) {
#else
                              ) {
#endif
        HandleScope scope;
        Fuse* fuse = static_cast<Fuse *>(fuse_req_userdata(req));

        Local<Value> vgetxattr = fuse->fsobj->Get(getxattr_sym);
        Local<Function> getxattr = Local<Function>::Cast(vgetxattr);

        Local<Object> context = RequestContextToObject(fuse_req_ctx(req))->ToObject();
        Local<Number> inode = Number::New(ino);
        Local<String> name = String::New(name_);
        Local<Number> size = Number::New(size_);
#ifdef __APPLE__
        Local<Integer> position = Integer::New(position_);
#endif


        Reply* reply = new Reply();
        reply->request = req;
        Local<Object> replyObj = reply->constructor_template->GetFunction()->NewInstance();
        reply->Wrap(replyObj);

#ifdef __APPLE__
        Local<Value> argv[6] = {context, inode,
                                name, size,
                                position, replyObj};
#else
        Local<Value> argv[5] = {context, inode,
                                name, size, replyObj};
#endif

        TryCatch try_catch;

#ifdef __APPLE__
        getxattr->Call(fuse->fsobj, 6, argv);
#else
        getxattr->Call(fuse->fsobj, 5, argv);
#endif
        if (try_catch.HasCaught()) {
            FatalException(try_catch);
        }
    }

    void FileSystem::ListXAttr(fuse_req_t req,
                               fuse_ino_t ino,
                               size_t size_) {
        HandleScope scope;
        Fuse* fuse = static_cast<Fuse *>(fuse_req_userdata(req));

        Local<Value> vlistxattr = fuse->fsobj->Get(listxattr_sym);
        Local<Function> listxattr = Local<Function>::Cast(vlistxattr);

        Local<Object> context = RequestContextToObject(fuse_req_ctx(req))->ToObject();
        Local<Number> inode = Number::New(ino);
        Local<Number> size = Number::New(size_);

        Reply* reply = new Reply();
        reply->request = req;
        Local<Object> replyObj = reply->constructor_template->GetFunction()->NewInstance();
        reply->Wrap(replyObj);

        Local<Value> argv[4] = {context, inode,
                                size, replyObj};
        TryCatch try_catch;

        listxattr->Call(fuse->fsobj, 4, argv);

        if (try_catch.HasCaught()) {
            FatalException(try_catch);
        }
    }

    void FileSystem::RemoveXAttr(fuse_req_t req,
                                 fuse_ino_t ino,
                                 const char* name_) {
        HandleScope scope;
        Fuse* fuse = static_cast<Fuse *>(fuse_req_userdata(req));

        Local<Value> vremovexattr = fuse->fsobj->Get(removexattr_sym);
        Local<Function> removexattr = Local<Function>::Cast(vremovexattr);

        Local<Object> context = RequestContextToObject(fuse_req_ctx(req))->ToObject();
        Local<Number> inode = Number::New(ino);
        Local<String> name = String::New(name_);

        Reply* reply = new Reply();
        reply->request = req;
        Local<Object> replyObj = reply->constructor_template->GetFunction()->NewInstance();
        reply->Wrap(replyObj);

        Local<Value> argv[4] = {context, inode,
                                name, replyObj};
        TryCatch try_catch;

        removexattr->Call(fuse->fsobj, 4, argv);

        if (try_catch.HasCaught()) {
            FatalException(try_catch);
        }
    }

    void FileSystem::Access(fuse_req_t req,
                            fuse_ino_t ino,
                            int mask_) {
        HandleScope scope;
        Fuse* fuse = static_cast<Fuse *>(fuse_req_userdata(req));

        Local<Value> vaccess = fuse->fsobj->Get(access_sym);
        Local<Function> access = Local<Function>::Cast(vaccess);

        Local<Object> context = RequestContextToObject(fuse_req_ctx(req))->ToObject();
        Local<Number> inode = Number::New(ino);
        Local<Integer> mask = Integer::New(mask_);

        Reply* reply = new Reply();
        reply->request = req;
        Local<Object> replyObj = reply->constructor_template->GetFunction()->NewInstance();
        reply->Wrap(replyObj);

        Local<Value> argv[4] = {context, inode,
                                mask, replyObj};

        TryCatch try_catch;

        access->Call(fuse->fsobj, 4, argv);

        if (try_catch.HasCaught()) {
            FatalException(try_catch);
        }
    }

    void FileSystem::Create(fuse_req_t req,
                            fuse_ino_t parent,
                            const char* name,
                            mode_t mode,
                            struct fuse_file_info* fi) {
        HandleScope scope;
        Fuse* fuse = static_cast<Fuse *>(fuse_req_userdata(req));

        Local<Value> vcreate = fuse->fsobj->Get(create_sym);
        Local<Function> create = Local<Function>::Cast(vcreate);

        Local<Object> context = RequestContextToObject(fuse_req_ctx(req))->ToObject();
        Local<Number> parentInode = Number::New(parent);
        Local<String> name_ = String::New(name);
        Local<Integer> mode_ = Integer::New(mode);

        FileInfo* info = new FileInfo();
        info->fi = fi;
        Local<Object> infoObj = info->constructor_template->GetFunction()->NewInstance();
        info->Wrap(infoObj);

        Reply* reply = new Reply();
        reply->request = req;
        Local<Object> replyObj = reply->constructor_template->GetFunction()->NewInstance();
        reply->Wrap(replyObj);

        Local<Value> argv[6] = {context, parentInode,
                                name_, mode_,
                                infoObj, replyObj};

        TryCatch try_catch;

        create->Call(fuse->fsobj, 6, argv);

        if (try_catch.HasCaught()) {
            FatalException(try_catch);
        }
    }

    void FileSystem::GetLock(fuse_req_t req,
                             fuse_ino_t ino,
                             struct fuse_file_info* fi,
                             struct flock* lock) {
        HandleScope scope;
        Fuse* fuse = static_cast<Fuse *>(fuse_req_userdata(req));

        Local<Value> vgetlk = fuse->fsobj->Get(getlk_sym);
        Local<Function> getlk = Local<Function>::Cast(vgetlk);

        Local<Object> context = RequestContextToObject(fuse_req_ctx(req))->ToObject();
        Local<Number> inode = Number::New(ino);

        FileInfo* info = new FileInfo();
        info->fi = fi;
        Local<Object> infoObj = info->constructor_template->GetFunction()->NewInstance();
        info->Wrap(infoObj);

        Local<Object> lockObj = FlockToObject(lock)->ToObject();

        Reply* reply = new Reply();
        reply->request = req;
        Local<Object> replyObj = reply->constructor_template->GetFunction()->NewInstance();
        reply->Wrap(replyObj);

        Local<Value> argv[5] = {context, inode,
                                infoObj, lockObj, replyObj};

        TryCatch try_catch;

        getlk->Call(fuse->fsobj, 5, argv);

        if (try_catch.HasCaught()) {
            FatalException(try_catch);
        }
    }

    void FileSystem::SetLock(fuse_req_t req,
                             fuse_ino_t ino,
                             struct fuse_file_info* fi,
                             struct flock* lock,
                             int sleep_) {
        HandleScope scope;
        Fuse* fuse = static_cast<Fuse *>(fuse_req_userdata(req));

        Local<Value> vsetlk = fuse->fsobj->Get(setlk_sym);
        Local<Function> setlk = Local<Function>::Cast(vsetlk);

        Local<Object> context = RequestContextToObject(fuse_req_ctx(req))->ToObject();
        Local<Number> inode = Number::New(ino);
        Local<Integer> sleep = Integer::New(sleep_);

        FileInfo* info = new FileInfo();
        info->fi = fi;
        Local<Object> infoObj = info->constructor_template->GetFunction()->NewInstance();
        info->Wrap(infoObj);

        Local<Object> lockObj = FlockToObject(lock)->ToObject();

        Reply* reply = new Reply();
        reply->request = req;
        Local<Object> replyObj = reply->constructor_template->GetFunction()->NewInstance();
        reply->Wrap(replyObj);

        Local<Value> argv[6] = {context, inode,
                                infoObj, lockObj,
                                sleep, replyObj};

        TryCatch try_catch;

        setlk->Call(fuse->fsobj, 6, argv);

        if (try_catch.HasCaught()) {
            FatalException(try_catch);
        }
    }

    void FileSystem::BMap(fuse_req_t req,
                          fuse_ino_t ino,
                          size_t blocksize_,
                          uint64_t idx) {
        HandleScope scope;
        Fuse* fuse = static_cast<Fuse *>(fuse_req_userdata(req));

        Local<Value> vbmap = fuse->fsobj->Get(bmap_sym);
        Local<Function> bmap = Local<Function>::Cast(vbmap);

        Local<Object> context = RequestContextToObject(fuse_req_ctx(req))->ToObject();
        Local<Number> inode = Number::New(ino);
        Local<Integer> blocksize = Integer::New(blocksize_);
        Local<Integer> index = Integer::New(idx);

        Reply* reply = new Reply();
        reply->request = req;
        Local<Object> replyObj = reply->constructor_template->GetFunction()->NewInstance();
        reply->Wrap(replyObj);

        Local<Value> argv[5] = {context, inode,
                                blocksize, index, replyObj};

        TryCatch try_catch;

        bmap->Call(fuse->fsobj, 5, argv);

        if (try_catch.HasCaught()) {
            FatalException(try_catch);
        }
    }

    void FileSystem::IOCtl(fuse_req_t req,
                           fuse_ino_t ino,
                           int cmd,
                           void* arg,
                           struct fuse_file_info* fi,
                           unsigned* flagsp,
                           const void* in_buf,
                           size_t in_bufsz,
                           size_t out_bufszp) {

    }

    void FileSystem::Poll(fuse_req_t req,
                          fuse_ino_t ino,
                          struct fuse_file_info* fi,
                          struct fuse_pollhandle* ph) {


    }

    struct fuse_lowlevel_ops* FileSystem::GetOperations() {
        return &fuse_ops;
    }
}


