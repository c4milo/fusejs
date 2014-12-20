#include "filesystem.h"
#include "reply.h"
#include "file_info.h"
#include "bindings.h"
#include "node_buffer.h"

namespace NodeFuse {
    Persistent<FunctionTemplate> FileSystem::constructor_template;

    ck_ring_t *ck_ring;
    ck_ring_buffer_t ck_ring_buffer[_RING_SIZE_];

    static struct fuse_lowlevel_ops fuse_ops = {};

    //Symbols for FUSE operations
    static Persistent<String> init_sym;
    static Persistent<String> destroy_sym;
    static Persistent<String> lookup_sym;
    static Persistent<String> forget_sym;
    static Persistent<String> getattr_sym;
    static Persistent<String> setattr_sym;
    static Persistent<String> readlink_sym;
    static Persistent<String> mknod_sym;
    static Persistent<String> mkdir_sym;
    static Persistent<String> unlink_sym;
    static Persistent<String> rmdir_sym;
    static Persistent<String> symlink_sym;
    static Persistent<String> rename_sym;
    static Persistent<String> link_sym;
    static Persistent<String> open_sym;
    static Persistent<String> read_sym;
    static Persistent<String> write_sym;
    static Persistent<String> flush_sym;
    static Persistent<String> release_sym;
    static Persistent<String> fsync_sym;
    static Persistent<String> opendir_sym;
    static Persistent<String> readdir_sym;
    static Persistent<String> releasedir_sym;
    static Persistent<String> fsyncdir_sym;
    static Persistent<String> statfs_sym;
    static Persistent<String> setxattr_sym;
    static Persistent<String> getxattr_sym;
    static Persistent<String> listxattr_sym;
    static Persistent<String> removexattr_sym;
    static Persistent<String> access_sym;
    static Persistent<String> create_sym;
    static Persistent<String> getlk_sym;
    static Persistent<String> setlk_sym;
    static Persistent<String> bmap_sym;
    static Persistent<String> ioctl_sym;
    static Persistent<String> poll_sym;

    //fuse_conn_info symbols
    //Major version of the fuse protocol
    static Persistent<String> conn_info_proto_major_sym;
    //Minor version of the fuse protocol
    static Persistent<String> conn_info_proto_minor_sym;
    //Is asynchronous read supported
    static Persistent<String> conn_info_async_read_sym;
    //Maximum size of the write buffer
    static Persistent<String> conn_info_max_write_sym;
    //Maximum readahead
    static Persistent<String> conn_info_max_readahead_sym;
    //Capability flags, that the kernel supports
    static Persistent<String> conn_info_capable_sym;
    //Capability flags, that the filesystem wants to enable
    static Persistent<String> conn_info_want_sym;
    
    void FileSystem::DispatchOp(uv_async_t* handle, int status)
    {
        struct fuse_cmd *op = NULL; //(struct fuse_cmd *)malloc(sizeof(struct fuse_cmd));

        while (ck_ring_dequeue_spmc(ck_ring, ck_ring_buffer, (void*) &op) == true){
            switch(op->op){
                case _FUSE_OPS_LOOKUP_:
                    RemoteLookup(op->req, op->ino, op->name);
                    break;
                case _FUSE_OPS_GETATTR_:
                    RemoteGetAttr(op->req, op->ino, op->s.fi);
                    break;
                case _FUSE_OPS_OPEN_:
                    RemoteOpen(op->req, op->ino, op->s.fi);
                    break;
                case _FUSE_OPS_READ_:
                    RemoteRead(op->req, op->ino, op->size, op->off, (op->s).fi);
                    break;
                case _FUSE_OPS_READDIR_:
                    RemoteReadDir(op->req, op->ino, op->size, op->off, (op->s).fi);
                    break;
                case _FUSE_OPS_INIT_:
                    break;
                case _FUSE_OPS_DESTROY_:
                    break;
                case _FUSE_OPS_FORGET_:
                    break;
                case _FUSE_OPS_SETATTR_:
                    RemoteSetAttr(op->req, op->ino, op->attr, op->to_set, op->s.fi);
                    break;
                case _FUSE_OPS_READLINK_:
                    break;
                case _FUSE_OPS_MKNOD_:
                    RemoteMkNod(op->req, op->ino, op->name, op->mode, op-> dev);
                    break;
                case _FUSE_OPS_MKDIR_:
                    RemoteMkDir(op->req, op->ino, op->name,op->mode);
                    break;
                case _FUSE_OPS_UNLINK_:
                    RemoteUnlink(op->req, op->ino, op->name);
                    break;
                case _FUSE_OPS_RMDIR_:
                    RemoteRmDir(op->req, op->ino, op->name);
                    break;
                case _FUSE_OPS_SYMLINK_:
                    break;
                case _FUSE_OPS_RENAME_:
                    break;
                case _FUSE_OPS_LINK_:
                    break;
                case _FUSE_OPS_WRITE_:
                    RemoteWrite(op->req, op->ino, op->name, op->size, op->off, op->s.fi);
                    break;
                case _FUSE_OPS_FLUSH_:
                    break;
                case _FUSE_OPS_RELEASE_:
                    RemoteRelease(op->req, op->ino, op->s.fi);
                    break;
                case _FUSE_OPS_FSYNC_:
                    break;
                case _FUSE_OPS_OPENDIR_:
                    break;
                case _FUSE_OPS_RELEASEDIR_:
                    break;
                case _FUSE_OPS_FSYNCDIR_:
                    break;
                case _FUSE_OPS_STATFS_:
                    RemoteStatFs(op->req, op->ino);
                    break;
                case _FUSE_OPS_SETXATTR_:
                    break;
                case _FUSE_OPS_GETXATTR_:
                    break;
                case _FUSE_OPS_LISTXATTR_:
                    break;
                case _FUSE_OPS_REMOVEXATTR_:
                    break;
                case _FUSE_OPS_ACCESS_:
                    break;
                case _FUSE_OPS_CREATE_:
                    RemoteCreate(op->req, op->ino, op->name, op->mode, op->s.fi);
                    break;
                case _FUSE_OPS_GETLK_:
                    break;
                case _FUSE_OPS_SETLK_:
                    break;
                case _FUSE_OPS_BMAP_:
                    break;
            }
            free(op);
        }
    }


    void FileSystem::Initialize() {

        ck_ring = (ck_ring_t *) malloc(sizeof(ck_ring_t));
        ck_ring_init(ck_ring, _RING_SIZE_);

        fuse_ops.lookup     = FileSystem::Lookup;
        fuse_ops.getattr    = FileSystem::GetAttr;
        fuse_ops.open       = FileSystem::Open;
        fuse_ops.read       = FileSystem::Read;
        fuse_ops.readdir    = FileSystem::ReadDir;
        fuse_ops.write      = FileSystem::Write;
        // fuse_ops.create     = FileSystem::Create;
        fuse_ops.setattr    = FileSystem::SetAttr;
        // fuse_ops.init       = FileSystem::Init;
        // fuse_ops.destroy    = FileSystem::Destroy;
        // fuse_ops.forget     = FileSystem::Forget;
        // fuse_ops.readlink   = FileSystem::ReadLink;
        fuse_ops.mknod      = FileSystem::MkNod;
        fuse_ops.mkdir      = FileSystem::MkDir;
        fuse_ops.unlink     = FileSystem::Unlink;
        fuse_ops.rmdir      = FileSystem::RmDir;
        // fuse_ops.symlink    = FileSystem::SymLink;
        // fuse_ops.rename     = FileSystem::Rename;
        // fuse_ops.link       = FileSystem::Link;
        // fuse_ops.flush      = FileSystem::Flush;
        fuse_ops.release    = FileSystem::Release;
        // fuse_ops.fsync      = FileSystem::FSync;
        // fuse_ops.opendir    = FileSystem::OpenDir;
        // fuse_ops.releasedir = FileSystem::ReleaseDir;
        // fuse_ops.fsyncdir   = FileSystem::FSyncDir;
        fuse_ops.statfs     = FileSystem::StatFs;
        // fuse_ops.setxattr   = FileSystem::SetXAttr;
        // fuse_ops.getxattr   = FileSystem::GetXAttr;
        // fuse_ops.listxattr  = FileSystem::ListXAttr;
        // fuse_ops.removexattr= FileSystem::RemoveXAttr;
        // fuse_ops.access     = FileSystem::Access;
        // fuse_ops.getlk      = FileSystem::GetLock;
        // fuse_ops.setlk      = FileSystem::SetLock;
        // fuse_ops.bmap       = FileSystem::BMap;
        //fuse_ops.ioctl      = FileSystem::IOCtl;
        //fuse_ops.poll       = FileSystem::Poll;

        NanAssignPersistent(init_sym,        NanNew("init"));
        NanAssignPersistent(destroy_sym,     NanNew("destroy"));
        NanAssignPersistent(lookup_sym,      NanNew("lookup"));
        NanAssignPersistent(forget_sym,      NanNew("forget"));
        NanAssignPersistent(getattr_sym,     NanNew("getattr"));
        NanAssignPersistent(setattr_sym,     NanNew("setattr"));
        NanAssignPersistent(readlink_sym,    NanNew("readlink"));
        NanAssignPersistent(mknod_sym,       NanNew("mknod"));
        NanAssignPersistent(mkdir_sym,       NanNew("mkdir"));
        NanAssignPersistent(unlink_sym,      NanNew("unlink"));
        NanAssignPersistent(rmdir_sym,       NanNew("rmdir"));
        NanAssignPersistent(symlink_sym,     NanNew("symlink"));
        NanAssignPersistent(rename_sym,      NanNew("rename"));
        NanAssignPersistent(link_sym,        NanNew("link"));
        NanAssignPersistent(open_sym,        NanNew("open"));
        NanAssignPersistent(read_sym,        NanNew("read"));
        NanAssignPersistent(write_sym,       NanNew("write"));
        NanAssignPersistent(flush_sym,       NanNew("flush"));
        NanAssignPersistent(release_sym,     NanNew("release"));
        NanAssignPersistent(fsync_sym,       NanNew("fsync"));
        NanAssignPersistent(opendir_sym,     NanNew("opendir"));
        NanAssignPersistent(readdir_sym,     NanNew("readdir"));
        NanAssignPersistent(releasedir_sym,  NanNew("releasedir"));
        NanAssignPersistent(fsyncdir_sym,    NanNew("fsyncdir"));
        NanAssignPersistent(statfs_sym,      NanNew("statfs"));
        NanAssignPersistent(setxattr_sym,    NanNew("setxattr"));
        NanAssignPersistent(getxattr_sym,    NanNew("getxattr"));
        NanAssignPersistent(listxattr_sym,   NanNew("listxattr"));
        NanAssignPersistent(removexattr_sym, NanNew("removexattr"));
        NanAssignPersistent(access_sym,      NanNew("access"));
        NanAssignPersistent(create_sym,      NanNew("create"));
        NanAssignPersistent(getlk_sym,       NanNew("getlk"));
        NanAssignPersistent(setlk_sym,       NanNew("setlk"));
        NanAssignPersistent(bmap_sym,        NanNew("bmap"));
        NanAssignPersistent(ioctl_sym,       NanNew("ioctl"));
        NanAssignPersistent(poll_sym,        NanNew("poll"));
        NanAssignPersistent(conn_info_proto_major_sym,     NanNew("proto_major"));
        NanAssignPersistent(conn_info_proto_minor_sym,     NanNew("proto_minor"));
        NanAssignPersistent(conn_info_async_read_sym,      NanNew("async_read"));
        NanAssignPersistent(conn_info_max_write_sym,       NanNew("max_write"));
        NanAssignPersistent(conn_info_max_readahead_sym,   NanNew("max_readahead"));
        NanAssignPersistent(conn_info_capable_sym,         NanNew("capable"));
        NanAssignPersistent(conn_info_want_sym,            NanNew("want"));
    }

    void FileSystem::Init(void* userdata,
                          struct fuse_conn_info* conn) {
        NanScope();
        Fuse* fuse = static_cast<Fuse *>(userdata);
        Local<Object> fsobj = NanNew(fuse->fsobj);
        Local<Value> vinit = fsobj->Get(NanNew(init_sym));
        Local<Function> init = Local<Function>::Cast(vinit);

        //These properties will be read-only for now.
        //TODO set accessors for read/write properties
        Local<Object> info = NanNew<Object>();
        info->Set(NanNew<String>(conn_info_proto_major_sym), NanNew<Integer>(conn->proto_major));
        info->Set(NanNew<String>(conn_info_proto_minor_sym), NanNew<Integer>(conn->proto_minor));
        info->Set(NanNew<String>(conn_info_async_read_sym), NanNew<Integer>(conn->async_read));
        info->Set(NanNew<String>(conn_info_max_write_sym), NanNew<Number>(conn->max_write));
        info->Set(NanNew<String>(conn_info_max_readahead_sym), NanNew<Number>(conn->max_readahead));
        //TODO macro to enable certain properties given the fuse version
        //info->Set(conn_info_capable_sym, NanNew<Integer>(conn->capable));
        //info->Set(conn_info_want_sym, NanNew<Integer>(conn->want));

        Local<Value> argv[1] = {info};

        TryCatch try_catch;

        init->Call(fsobj, 1, argv);

        if (try_catch.HasCaught()) {
            FatalException(try_catch);
        }
    }

    void FileSystem::Destroy(void* userdata) {
        NanScope();
        Fuse* fuse = static_cast<Fuse *>(userdata);
        Local<Object> fsobj = NanNew(fuse->fsobj);
        Local<Value> vdestroy = fsobj->Get(NanNew(destroy_sym));
        Local<Function> destroy = Local<Function>::Cast(vdestroy);

        TryCatch try_catch;

        destroy->Call(fsobj, 0, NULL);

        if (try_catch.HasCaught()) {
            FatalException(try_catch);
        }
    }

    void FileSystem::Lookup(fuse_req_t req,
                            fuse_ino_t parent,
                            const char* name) {
        struct fuse_cmd *op = (struct fuse_cmd *)malloc(sizeof(struct fuse_cmd));
        op->op = _FUSE_OPS_LOOKUP_;
        op->req = req;
        op->ino = parent;
        op->name = name;
        if (ck_ring_enqueue_spmc(ck_ring, ck_ring_buffer, (void *) op) == false) {
            printf("ckring was full while trying to enqueue lookup at inode %d - with child %s\n", (int) parent,name);
            return;
        }
        uv_async_send(&uv_async_handle);

    }

    void FileSystem::RemoteLookup(fuse_req_t req,
                            fuse_ino_t parent,
                            const char* name) {
        NanScope();
        Fuse* fuse = static_cast<Fuse *>(fuse_req_userdata(req));
        Local<Object> fsobj = NanNew(fuse->fsobj);
        Local<Value> vlookup = fsobj->Get(NanNew(lookup_sym));
        Local<Function> lookup = Local<Function>::Cast(vlookup);

        Local<Object> context = RequestContextToObject(fuse_req_ctx(req))->ToObject();
        Local<Number> parentInode = NanNew<Number>(parent);
        Local<String> entryName = NanNew<String>(name);

        Reply* reply = new Reply();
        reply->request = req;
        Local<Object> replyObj = NanNew(reply->constructor_template)->GetFunction()->NewInstance();
        reply->Wrap(replyObj);

        Local<Value> argv[4] = {context, parentInode,
                                entryName, replyObj};
        TryCatch try_catch;

        lookup->Call(fsobj, 4, argv);

        if (try_catch.HasCaught()) {
            FatalException(try_catch);
        }
        //scope.Close(Undefined());

    }

    void FileSystem::Forget(fuse_req_t req,
                            fuse_ino_t ino,
                            unsigned long nlookup) {
        NanScope();
        Fuse* fuse = static_cast<Fuse *>(fuse_req_userdata(req));
        Local<Object> fsobj = NanNew(fuse->fsobj);
        Local<Value> vforget = fsobj->Get(NanNew(forget_sym));
        Local<Function> forget = Local<Function>::Cast(vforget);

        Local<Object> context = RequestContextToObject(fuse_req_ctx(req))->ToObject();
        Local<Number> inode = NanNew<Number>(ino);
        Local<Integer> nlookup_ = NanNew<Integer>(nlookup);

        Local<Value> argv[3] = {context, inode, nlookup_};

        TryCatch try_catch;

        forget->Call(fsobj, 3, argv);

        if (try_catch.HasCaught()) {
            FatalException(try_catch);
        }

        fuse_reply_none(req);
        //scope.Close(Undefined());

    }

    void FileSystem::GetAttr(fuse_req_t req,
                             fuse_ino_t ino,
                             struct fuse_file_info* fi) {
        struct fuse_cmd *op = (struct fuse_cmd *)malloc(sizeof(struct fuse_cmd));
        op->op = _FUSE_OPS_GETATTR_;
        op->req = req;
        op->ino = ino;
        (op->s).fi = fi;

        if (ck_ring_enqueue_spmc(ck_ring, ck_ring_buffer, (void *) op) == false) {
            printf("ckring was full while trying to enqueue getattr at inode %d\n", (int) ino);
            return;
        }

        uv_async_send(&uv_async_handle);

    }
    void FileSystem::RemoteGetAttr(fuse_req_t req,
                             fuse_ino_t ino,
                             struct fuse_file_info* fi) {
        NanScope();
        Fuse* fuse = static_cast<Fuse *>(fuse_req_userdata(req));
        Local<Object> fsobj = NanNew(fuse->fsobj);

        Local<Value> vgetattr = fsobj->Get(NanNew(getattr_sym));
        Local<Function> getattr = Local<Function>::Cast(vgetattr);

        Local<Object> context = RequestContextToObject(fuse_req_ctx(req))->ToObject();
        Local<Number> inode = NanNew<Number>(ino);

        Reply* reply = new Reply();
        reply->request = req;
        Local<Object> replyObj = NanNew(reply->constructor_template)->GetFunction()->NewInstance();
        reply->Wrap(replyObj);

        Local<Value> argv[3] = {context, inode, replyObj};

        TryCatch try_catch;

        getattr->Call(fsobj, 3, argv);

        if (try_catch.HasCaught()) {
            FatalException(try_catch);
        }
        //scope.Close(Undefined());

    }

    void FileSystem::SetAttr(fuse_req_t req,
                             fuse_ino_t ino,
                             struct stat* attr,
                             int to_set,
                             struct fuse_file_info* fi) {

        struct fuse_cmd *op = (struct fuse_cmd *)malloc(sizeof(struct fuse_cmd));
        op->op = _FUSE_OPS_SETATTR_;
        op->req = req;
        op->ino = ino;
        op->attr = attr;
        op->to_set = to_set;
        (op->s).fi = fi;

        if (ck_ring_enqueue_spmc(ck_ring, ck_ring_buffer, (void *) op) == false) {
            printf("ckring was full while trying to enqueue setattr at inode %d\n", (int) ino);
            return;
        }

        uv_async_send(&uv_async_handle);

    }
    void FileSystem::RemoteSetAttr(fuse_req_t req,
                             fuse_ino_t ino,
                             struct stat* attr,
                             int to_set,
                             struct fuse_file_info* fi) {

        NanScope();
        Fuse *fuse = static_cast<Fuse *>(fuse_req_userdata(req));
        Local<Object> fsobj = NanNew(fuse->fsobj);

        Local<Value> vsetattr = fsobj->Get(NanNew(setattr_sym));
        Local<Function> setattr = Local<Function>::Cast(vsetattr);

        Local<Object> context = RequestContextToObject(fuse_req_ctx(req))->ToObject();
        Local<Number> inode = NanNew<Number>(ino);

        Local<Object> attrs = GetAttrsToBeSet(to_set, attr)->ToObject();

        Reply *reply = new Reply();
        reply->request = req;
        Local<Object> replyObj = NanNew(reply->constructor_template)->GetFunction()->NewInstance();
        reply->Wrap(replyObj);

        Local<Value> argv[4] = {context, inode, attrs, replyObj};

        TryCatch try_catch;

        setattr->Call(fsobj, 4, argv);

        if (try_catch.HasCaught()) {
            FatalException(try_catch);
        }
        //scope.Close(Undefined());

    }

    void FileSystem::ReadLink(fuse_req_t req, fuse_ino_t ino) {
        NanScope();
        Fuse* fuse = static_cast<Fuse *>(fuse_req_userdata(req));
        Local<Object> fsobj = NanNew(fuse->fsobj);

        Local<Value> vreadlink = fsobj->Get(NanNew(readlink_sym));
        Local<Function> readlink = Local<Function>::Cast(vreadlink);

        Local<Object> context = RequestContextToObject(fuse_req_ctx(req))->ToObject();
        Local<Number> inode = NanNew<Number>(ino);

        Reply* reply = new Reply();
        reply->request = req;
        Local<Object> replyObj = NanNew( reply->constructor_template)->GetFunction()->NewInstance();
        reply->Wrap(replyObj);

        Local<Value> argv[3] = {context, inode, replyObj};

        TryCatch try_catch;

        readlink->Call(fsobj, 3, argv);

        if (try_catch.HasCaught()) {
            FatalException(try_catch);
        }
    }

    void FileSystem::MkNod(fuse_req_t req,
                           fuse_ino_t parent,
                           const char* name,
                           mode_t mode,
                           dev_t rdev) {
        struct fuse_cmd *op = (struct fuse_cmd *)malloc(sizeof(struct fuse_cmd));
        op->op = _FUSE_OPS_MKNOD_;
        op->req = req;
        op->ino= parent;
        op->name = name;
        op->mode = mode;
        op->dev = rdev;
        if (ck_ring_enqueue_spmc(ck_ring, ck_ring_buffer, (void *) op) == false) {
            printf("ckring was full while trying to enqueue mknod with parent %d and child %s\n", (int) parent, name);
            return;
        }

        uv_async_send(&uv_async_handle);



    }
    void FileSystem::RemoteMkNod(fuse_req_t req,
                           fuse_ino_t parent,
                           const char* name,
                           mode_t mode,
                           dev_t rdev) {

        NanScope();
        Fuse* fuse = static_cast<Fuse *>(fuse_req_userdata(req));
        Local<Object> fsobj = NanNew(fuse->fsobj);

        Local<Value> vmknod = fsobj->Get(NanNew(mknod_sym));
        Local<Function> mknod = Local<Function>::Cast(vmknod);

        Local<Object> context = RequestContextToObject(fuse_req_ctx(req))->ToObject();
        Local<Number> parentInode = NanNew<Number>(parent);

        Local<String> name_ = NanNew<String>(name);
        Local<Integer> mode_ = NanNew<Integer>(mode);
        Local<Integer> rdev_ = NanNew<Integer>(rdev);

        Reply* reply = new Reply();
        reply->request = req;
        Local<Object> replyObj = NanNew(reply->constructor_template)->GetFunction()->NewInstance();
        reply->Wrap(replyObj);

        Local<Value> argv[6] = {context, parentInode,
                                name_, mode_,
                                rdev_, replyObj};

        TryCatch try_catch;

        mknod->Call(fsobj, 6, argv);

        if (try_catch.HasCaught()) {
            FatalException(try_catch);
        }
        //scope.Close(Undefined());

    }

    void FileSystem::MkDir(fuse_req_t req,
                           fuse_ino_t parent,
                           const char* name,
                           mode_t mode) {

        struct fuse_cmd *op = (struct fuse_cmd *)malloc(sizeof(struct fuse_cmd));
        op->op = _FUSE_OPS_MKDIR_;
        op->req = req;
        op->ino = parent;
        op->name = name;
        op->mode = mode;
        if (ck_ring_enqueue_spmc(ck_ring, ck_ring_buffer, (void *) op) == false) {
            printf("ckring was full while trying to enqueue mkdir with parent %d and name %s\n", (int) parent, name);
            return;
        }

        uv_async_send(&uv_async_handle);

    }
    void FileSystem::RemoteMkDir(fuse_req_t req,
                           fuse_ino_t parent,
                           const char* name,
                           mode_t mode) {

        NanScope();
        Fuse* fuse = static_cast<Fuse *>(fuse_req_userdata(req));
        Local<Object> fsobj = NanNew(fuse->fsobj);

        Local<Value> vmkdir = fsobj->Get(NanNew(mkdir_sym));
        Local<Function> mkdir = Local<Function>::Cast(vmkdir);

        Local<Object> context = RequestContextToObject(fuse_req_ctx(req))->ToObject();
        Local<Number> parentInode = NanNew<Number>(parent);

        Local<String> name_ = NanNew<String>(name);
        Local<Integer> mode_ = NanNew<Integer>(mode);

        Reply* reply = new Reply();
        reply->request = req;
        Local<Object> replyObj = NanNew(reply->constructor_template)->GetFunction()->NewInstance();
        reply->Wrap(replyObj);

        Local<Value> argv[5] = {context, parentInode,
                                name_, mode_, replyObj};

        TryCatch try_catch;

        mkdir->Call(fsobj, 5, argv);

        if (try_catch.HasCaught()) {
            FatalException(try_catch);
        }
        //scope.Close(Undefined());

    }

    void FileSystem::Unlink(fuse_req_t req,
                            fuse_ino_t parent,
                            const char* name) {

        struct fuse_cmd *op = (struct fuse_cmd *)malloc(sizeof(struct fuse_cmd));
        op->op = _FUSE_OPS_UNLINK_;
        op->req = req;
        op->ino = parent;
        op->name = name;
        if (ck_ring_enqueue_spmc(ck_ring, ck_ring_buffer, (void *) op) == false) {
            printf("ckring was full while trying to unlink %s from parent %d \n",  name, (int) parent);
            return;
        }

        uv_async_send(&uv_async_handle);

    }
    void FileSystem::RemoteUnlink(fuse_req_t req,
                            fuse_ino_t parent,
                            const char* name) {

        NanScope();
        Fuse* fuse = static_cast<Fuse *>(fuse_req_userdata(req));
        Local<Object> fsobj = NanNew(fuse->fsobj);

        Local<Value> vunlink = fsobj->Get(NanNew(unlink_sym));
        Local<Function> unlink = Local<Function>::Cast(vunlink);

        Local<Object> context = RequestContextToObject(fuse_req_ctx(req))->ToObject();
        Local<Number> parentInode = NanNew<Number>(parent);
        Local<String> name_ = NanNew<String>(name);

        Reply* reply = new Reply();
        reply->request = req;
        Local<Object> replyObj = NanNew(reply->constructor_template)->GetFunction()->NewInstance();
        reply->Wrap(replyObj);

        Local<Value> argv[4] = {context, parentInode, name_, replyObj};

        TryCatch try_catch;

        unlink->Call(fsobj, 4, argv);

        if (try_catch.HasCaught()) {
            FatalException(try_catch);
        }
        //scope.Close(Undefined());

    }

    void FileSystem::RmDir(fuse_req_t req,
                           fuse_ino_t parent,
                           const char* name) {
        struct fuse_cmd *op = (struct fuse_cmd *)malloc(sizeof(struct fuse_cmd));
        op->op = _FUSE_OPS_RMDIR_;
        op->req = req;
        op->ino = parent;
        op->name = name;
        if (ck_ring_enqueue_spmc(ck_ring, ck_ring_buffer, (void *) op) == false) {
            printf("ckring was full while trying to unlink folder %s from parent %d \n",  name, (int) parent);
            return;
        }

        uv_async_send(&uv_async_handle);

    }
    void FileSystem::RemoteRmDir(fuse_req_t req,
                           fuse_ino_t parent,
                           const char* name) {

        NanScope();
        Fuse* fuse = static_cast<Fuse *>(fuse_req_userdata(req));
        Local<Object> fsobj = NanNew(fuse->fsobj);

        Local<Value> vrmdir = fsobj->Get(NanNew(rmdir_sym));
        Local<Function> rmdir = Local<Function>::Cast(vrmdir);

        Local<Object> context = RequestContextToObject(fuse_req_ctx(req))->ToObject();
        Local<Number> parentInode = NanNew<Number>(parent);
        Local<String> name_ = NanNew<String>(name);

        Reply* reply = new Reply();
        reply->request = req;
        Local<Object> replyObj = NanNew(reply->constructor_template)->GetFunction()->NewInstance();
        reply->Wrap(replyObj);

        Local<Value> argv[4] = {context, parentInode, name_, replyObj};

        TryCatch try_catch;

        rmdir->Call(fsobj, 4, argv);

        if (try_catch.HasCaught()) {
            FatalException(try_catch);
        }
        //scope.Close(Undefined());
    }

    void FileSystem::SymLink(fuse_req_t req,
                             const char* link,
                             fuse_ino_t parent,
                             const char* name) {
        NanScope();
        Fuse* fuse = static_cast<Fuse *>(fuse_req_userdata(req));
        Local<Object> fsobj = NanNew(fuse->fsobj);

        Local<Value> vsymlink = fsobj->Get(NanNew(symlink_sym));
        Local<Function> symlink = Local<Function>::Cast(vsymlink);

        Local<Object> context = RequestContextToObject(fuse_req_ctx(req))->ToObject();
        Local<Number> parentInode = NanNew<Number>(parent);
        Local<String> name_ = NanNew<String>(name);
        Local<String> link_ = NanNew<String>(link);

        Reply* reply = new Reply();
        reply->request = req;
        Local<Object> replyObj = NanNew(reply->constructor_template)->GetFunction()->NewInstance();
        reply->Wrap(replyObj);

        Local<Value> argv[5] = {context, parentInode,
                                link_, name_, replyObj};

        TryCatch try_catch;

        symlink->Call(fsobj, 5, argv);

        if (try_catch.HasCaught()) {
            FatalException(try_catch);
        }
    }

    void FileSystem::Rename(fuse_req_t req,
                            fuse_ino_t parent,
                            const char *name,
                            fuse_ino_t newparent,
                            const char *newname) {
        NanScope();
        Fuse* fuse = static_cast<Fuse *>(fuse_req_userdata(req));
        Local<Object> fsobj = NanNew(fuse->fsobj);

        Local<Value> vrename = fsobj->Get(NanNew(rename_sym));
        Local<Function> rename = Local<Function>::Cast(vrename);

        Local<Object> context = RequestContextToObject(fuse_req_ctx(req))->ToObject();
        Local<Number> parentInode = NanNew<Number>(parent);
        Local<String> name_ = NanNew<String>(name);
        Local<Number> newParentInode = NanNew<Number>(newparent);
        Local<String> newName = NanNew<String>(newname);

        Reply* reply = new Reply();
        reply->request = req;
        Local<Object> replyObj = NanNew(reply->constructor_template)->GetFunction()->NewInstance();
        reply->Wrap(replyObj);

        Local<Value> argv[6] = {context, parentInode,
                                name_, newParentInode,
                                newName, replyObj};

        TryCatch try_catch;

        rename->Call(fsobj, 6, argv);

        if (try_catch.HasCaught()) {
            FatalException(try_catch);
        }
    }

    void FileSystem::Link(fuse_req_t req,
                          fuse_ino_t ino,
                          fuse_ino_t newparent,
                          const char* newname) {
        NanScope();
        Fuse* fuse = static_cast<Fuse *>(fuse_req_userdata(req));
        Local<Object> fsobj = NanNew(fuse->fsobj);

        Local<Value> vlink = fsobj->Get(NanNew(link_sym));
        Local<Function> link = Local<Function>::Cast(vlink);

        Local<Object> context = RequestContextToObject(fuse_req_ctx(req))->ToObject();
        Local<Number> inode = NanNew<Number>(ino);
        Local<Number> newParent = NanNew<Number>(newparent);
        Local<String> newName = NanNew<String>(newname);

        Reply* reply = new Reply();
        reply->request = req;
        Local<Object> replyObj = NanNew(reply->constructor_template)->GetFunction()->NewInstance();
        reply->Wrap(replyObj);

        Local<Value> argv[5] = {context, inode,
                                newParent, newName, replyObj};

        TryCatch try_catch;

        link->Call(fsobj, 5, argv);

        if (try_catch.HasCaught()) {
            FatalException(try_catch);
        }
    }

    void FileSystem::Open(fuse_req_t req,
                          fuse_ino_t ino,
                          struct fuse_file_info* fi) {
        struct fuse_cmd *op = (struct fuse_cmd *)malloc(sizeof(struct fuse_cmd));
        op->op = _FUSE_OPS_OPEN_;
        op->req = req;
        op->ino = ino;
        op->s.fi = fi;
        if (ck_ring_enqueue_spmc(ck_ring, ck_ring_buffer, (void *) op) == false) {
            printf("ckring was full while trying to enqueue open at inode %d\n", (int) ino);
            return;
        }
        uv_async_send(&uv_async_handle);

    }
    void FileSystem::RemoteOpen(fuse_req_t req,
                          fuse_ino_t ino,
                          struct fuse_file_info* fi) {

        NanScope();
        Fuse* fuse = static_cast<Fuse *>(fuse_req_userdata(req));
        Local<Object> fsobj = NanNew(fuse->fsobj);

        Local<Value> vopen = fsobj->Get(NanNew(open_sym));
        Local<Function> open = Local<Function>::Cast(vopen);

        Local<Object> context = RequestContextToObject(fuse_req_ctx(req))->ToObject();
        Local<Number> inode = NanNew<Number>(ino);

        FileInfo* info = new FileInfo();
        info->fi = fi;
        Local<Object> infoObj = NanNew(info->constructor_template)->GetFunction()->NewInstance();
        info->Wrap(infoObj);

        Reply* reply = new Reply();
        reply->request = req;
        Local<Object> replyObj = NanNew(reply->constructor_template)->GetFunction()->NewInstance();
        reply->Wrap(replyObj);

        Local<Value> argv[4] = {context, inode,
                                infoObj, replyObj};

        TryCatch try_catch;

        open->Call(fsobj, 4, argv);

        if (try_catch.HasCaught()) {
            FatalException(try_catch);
        }
        //scope.Close(Undefined());

    }

    void FileSystem::Read(fuse_req_t req,
                          fuse_ino_t ino,
                          size_t size_,
                          off_t off,
                          struct fuse_file_info* fi) {
        struct fuse_cmd *op = (struct fuse_cmd *)malloc(sizeof(struct fuse_cmd));
        op->op = _FUSE_OPS_READ_;
        op->req = req;
        op->ino = ino;
        op->off = off;
        op->size = size_;
        op->s.fi = fi;
        if (ck_ring_enqueue_spmc(ck_ring, ck_ring_buffer, (void *) op) == false) {
            printf("ckring was full while trying to enqueue read at inode %d\n", (int) ino);
            return;
        }
        uv_async_send(&uv_async_handle);


    }
    void FileSystem::RemoteRead(fuse_req_t req,
                          fuse_ino_t ino,
                          size_t size_,
                          off_t off,
                          struct fuse_file_info* fi) {

        NanScope();
        Fuse* fuse = static_cast<Fuse *>(fuse_req_userdata(req));
        Local<Object> fsobj = NanNew(fuse->fsobj);

        Local<Value> vread = fsobj->Get(NanNew(read_sym));
        Local<Function> read = Local<Function>::Cast(vread);

        Local<Object> context = RequestContextToObject(fuse_req_ctx(req))->ToObject();
        Local<Number> inode = NanNew<Number>(ino);
        Local<Integer> size = NanNew<Integer>(size_);
        Local<Integer> offset = NanNew<Integer>(off);

        FileInfo* info = new FileInfo();
        info->fi = fi;
        Local<Object> infoObj = NanNew(info->constructor_template)->GetFunction()->NewInstance();
        info->Wrap(infoObj);

        Reply* reply = new Reply();
        reply->request = req;
        Local<Object> replyObj = NanNew(reply->constructor_template)->GetFunction()->NewInstance();
        reply->Wrap(replyObj);

        Local<Value> argv[6] = {context, inode,
                                size, offset,
                                infoObj, replyObj};

        TryCatch try_catch;

        read->Call(fsobj, 6, argv);

        if (try_catch.HasCaught()) {
            FatalException(try_catch);
        }
        //scope.Close(Undefined());

    }

    void FileSystem::Write(fuse_req_t req,
                           fuse_ino_t ino,
                           const char *buf,
                           size_t size,
                           off_t off,
                           struct fuse_file_info* fi) {
        struct fuse_cmd *op = (struct fuse_cmd *)malloc(sizeof(struct fuse_cmd));
        op->op = _FUSE_OPS_WRITE_;
        op->req = req;
        op->ino = ino;
        op->off = off;
        op->size = size;
        op->name = buf;
        op->s.fi = fi;
        if (ck_ring_enqueue_spmc(ck_ring, ck_ring_buffer, (void *) op) == false) {
            printf("ckring was full while trying to enqueue write at inode %d\n", (int) ino);
            return;
        }
        uv_async_send(&uv_async_handle);


    }
    void FileSystem::RemoteWrite(fuse_req_t req,
                           fuse_ino_t ino,
                           const char *buf_,
                           size_t size,
                           off_t off,
                           struct fuse_file_info* fi){
        NanScope();
        Fuse* fuse = static_cast<Fuse *>(fuse_req_userdata(req));
        Local<Object> fsobj = NanNew(fuse->fsobj);

        Local<Value> vwrite = fsobj->Get(NanNew(write_sym));
        Local<Function> write = Local<Function>::Cast(vwrite);

        Local<Object> context = RequestContextToObject(fuse_req_ctx(req))->ToObject();
        Local<Number> inode = NanNew<Number>(ino);
        Local<Integer> offset = NanNew<Integer>(off);

        const char * buf = strdup(buf_);
        Local<Object> buffer = NanBufferUse((char*) buf, size);

        FileInfo* info = new FileInfo();
        info->fi = fi;
        Local<Object> infoObj = NanNew(info->constructor_template)->GetFunction()->NewInstance();
        info->Wrap(infoObj);

        Reply* reply = new Reply();
        reply->request = req;
        Local<Object> replyObj = NanNew(reply->constructor_template)->GetFunction()->NewInstance();
        reply->Wrap(replyObj);

        Local<Value> argv[6] = {context, inode,
                                // NanNew<Object>(buffer->handle_), offset,
                                buffer, offset,
                                infoObj, replyObj};

        TryCatch try_catch;
        write->Call(fsobj, 6, argv);

        if (try_catch.HasCaught()) {
            FatalException(try_catch);
        }
        //scope.Close(Undefined());

    }

    void FileSystem::Flush(fuse_req_t req,
                           fuse_ino_t ino,
                           struct fuse_file_info* fi) {
        NanScope();
        Fuse* fuse = static_cast<Fuse *>(fuse_req_userdata(req));
        Local<Object> fsobj = NanNew(fuse->fsobj);

        Local<Value> vflush = fsobj->Get(NanNew(flush_sym));
        Local<Function> flush = Local<Function>::Cast(vflush);

        Local<Object> context = RequestContextToObject(fuse_req_ctx(req))->ToObject();
        Local<Number> inode = NanNew<Number>(ino);

        FileInfo* info = new FileInfo();
        info->fi = fi;
        Local<Object> infoObj = NanNew(info->constructor_template)->GetFunction()->NewInstance();
        info->Wrap(infoObj);

        Reply* reply = new Reply();
        reply->request = req;
        Local<Object> replyObj = NanNew(reply->constructor_template)->GetFunction()->NewInstance();
        reply->Wrap(replyObj);

        Local<Value> argv[4] = {context, inode,
                                infoObj, replyObj};

        TryCatch try_catch;

        flush->Call(fsobj, 4, argv);

        if (try_catch.HasCaught()) {
            FatalException(try_catch);
        }
    }

    void FileSystem::Release(fuse_req_t req,
                             fuse_ino_t ino,
                             struct fuse_file_info* fi) {

        struct fuse_cmd *op = (struct fuse_cmd *)malloc(sizeof(struct fuse_cmd));
        op->op = _FUSE_OPS_RELEASE_;
        op->req = req;
        op->ino = ino;
        op->s.fi = fi;
        if (ck_ring_enqueue_spmc(ck_ring, ck_ring_buffer, (void *) op) == false) {
            printf("ckring was full while trying to release inode %d\n", (int) ino);
            return;
        }
        uv_async_send(&uv_async_handle);

    }
    void FileSystem::RemoteRelease(fuse_req_t req,
                             fuse_ino_t ino,
                             struct fuse_file_info* fi) {

        NanScope();
        Fuse* fuse = static_cast<Fuse *>(fuse_req_userdata(req));
        Local<Object> fsobj = NanNew(fuse->fsobj);

        Local<Value> vrelease = fsobj->Get(NanNew(release_sym));
        Local<Function> release = Local<Function>::Cast(vrelease);

        Local<Object> context = RequestContextToObject(fuse_req_ctx(req))->ToObject();
        Local<Number> inode = NanNew<Number>(ino);

        FileInfo* info = new FileInfo();
        info->fi = fi;
        Local<Object> infoObj = NanNew(info->constructor_template)->GetFunction()->NewInstance();
        info->Wrap(infoObj);

        Reply* reply = new Reply();
        reply->request = req;
        Local<Object> replyObj = NanNew(reply->constructor_template)->GetFunction()->NewInstance();
        reply->Wrap(replyObj);

        Local<Value> argv[4] = {context, inode,
                                infoObj, replyObj};

        TryCatch try_catch;

        release->Call(fsobj, 4, argv);

        if (try_catch.HasCaught()) {
            FatalException(try_catch);
        }
        //scope.Close(Undefined());
    }

    void FileSystem::FSync(fuse_req_t req,
                           fuse_ino_t ino,
                           int datasync_,
                           struct fuse_file_info* fi) {
        NanScope();
        Fuse* fuse = static_cast<Fuse *>(fuse_req_userdata(req));
        Local<Object> fsobj = NanNew(fuse->fsobj);

        Local<Value> vfsync = fsobj->Get(NanNew(fsync_sym));
        Local<Function> fsync = Local<Function>::Cast(vfsync);

        Local<Object> context = RequestContextToObject(fuse_req_ctx(req))->ToObject();
        Local<Number> inode = NanNew<Number>(ino);
        bool datasync = datasync_ == 0 ? false : true;

        FileInfo* info = new FileInfo();
        info->fi = fi;
        Local<Object> infoObj = NanNew(info->constructor_template)->GetFunction()->NewInstance();
        info->Wrap(infoObj);

        Reply* reply = new Reply();
        reply->request = req;
        Local<Object> replyObj = NanNew(reply->constructor_template)->GetFunction()->NewInstance();
        reply->Wrap(replyObj);

        Local<Value> argv[5] = {context, inode,
                                NanNew<Boolean>(datasync)->ToObject(),
                                infoObj, replyObj};

        TryCatch try_catch;

        fsync->Call(fsobj, 5, argv);

        if (try_catch.HasCaught()) {
            FatalException(try_catch);
        }
    }

    void FileSystem::OpenDir(fuse_req_t req,
                             fuse_ino_t ino,
                             struct fuse_file_info* fi) {
        NanScope();
        Fuse* fuse = static_cast<Fuse *>(fuse_req_userdata(req));
        Local<Object> fsobj = NanNew(fuse->fsobj);

        Local<Value> vopendir = fsobj->Get(NanNew(opendir_sym));
        Local<Function> opendir = Local<Function>::Cast(vopendir);

        Local<Object> context = RequestContextToObject(fuse_req_ctx(req))->ToObject();
        Local<Number> inode = NanNew<Number>(ino);

        FileInfo* info = new FileInfo();
        info->fi = fi;
        Local<Object> infoObj = NanNew(info->constructor_template)->GetFunction()->NewInstance();
        info->Wrap(infoObj);

        Reply* reply = new Reply();
        reply->request = req;
        Local<Object> replyObj = NanNew(reply->constructor_template)->GetFunction()->NewInstance();
        reply->Wrap(replyObj);

        Local<Value> argv[4] = {context, inode,
                                infoObj, replyObj};

        TryCatch try_catch;

        opendir->Call(fsobj, 4, argv);

        if (try_catch.HasCaught()) {
            FatalException(try_catch);
        }
    }

    void FileSystem::ReadDir(fuse_req_t req,
                             fuse_ino_t ino,
                             size_t size_,
                             off_t off,
                             struct fuse_file_info* fi) {

        struct fuse_cmd *op = (struct fuse_cmd *)malloc(sizeof(struct fuse_cmd));
        op->op = _FUSE_OPS_READDIR_;
        op->req = req;
        op->ino = ino;
        op->size = size_;
        op->off = off;
        op->s.fi = fi;
        if (ck_ring_enqueue_spmc(ck_ring, ck_ring_buffer, (void *) op) == false) {
            printf("ckring was full while trying to enqueue readdir at inode %d\n", (uint8_t) ino);
            return;
        }
        uv_async_send(&uv_async_handle);
    }
    void FileSystem::RemoteReadDir(fuse_req_t req,
                             fuse_ino_t ino,
                             size_t size_,
                             off_t off,
                             struct fuse_file_info* fi) {

        NanScope();
        Fuse* fuse = static_cast<Fuse *>(fuse_req_userdata(req));
        Local<Object> fsobj = NanNew(fuse->fsobj);

        Local<Value> vreaddir = fsobj->Get(NanNew(readdir_sym));
        Local<Function> readdir = Local<Function>::Cast(vreaddir);

        Local<Object> context = RequestContextToObject(fuse_req_ctx(req))->ToObject();
        Local<Number> inode = NanNew<Number>(ino);
        Local<Integer> size = NanNew<Integer>(size_);
        Local<Integer> offset = NanNew<Integer>(off);

        FileInfo* info = new FileInfo();
        info->fi = fi;
        Local<Object> infoObj = NanNew(info->constructor_template)->GetFunction()->NewInstance();
        info->Wrap(infoObj);

        Reply* reply = new Reply();
        reply->request = req;
        Local<Object> replyObj = NanNew(reply->constructor_template)->GetFunction()->NewInstance();
        reply->Wrap(replyObj);

        Local<Value> argv[6] = {context, inode,
                                size, offset,
                                infoObj, replyObj};

        TryCatch try_catch;

        readdir->Call(fsobj, 6, argv);

        if (try_catch.HasCaught()) {
            FatalException(try_catch);
        }
        //scope.Close(Undefined());

    }

    void FileSystem::ReleaseDir(fuse_req_t req,
                                fuse_ino_t ino,
                                struct fuse_file_info* fi) {
        NanScope();
        Fuse* fuse = static_cast<Fuse *>(fuse_req_userdata(req));
        Local<Object> fsobj = NanNew(fuse->fsobj);

        Local<Value> vreleasedir = fsobj->Get(NanNew(releasedir_sym));
        Local<Function> releasedir = Local<Function>::Cast(vreleasedir);

        Local<Object> context = RequestContextToObject(fuse_req_ctx(req))->ToObject();
        Local<Number> inode = NanNew<Number>(ino);

        FileInfo* info = new FileInfo();
        info->fi = fi;
        Local<Object> infoObj = NanNew(info->constructor_template)->GetFunction()->NewInstance();
        info->Wrap(infoObj);

        Reply* reply = new Reply();
        reply->request = req;
        Local<Object> replyObj = NanNew(reply->constructor_template)->GetFunction()->NewInstance();
        reply->Wrap(replyObj);

        Local<Value> argv[4] = {context, inode,
                                infoObj, replyObj};

        TryCatch try_catch;

        releasedir->Call(fsobj, 4, argv);

        if (try_catch.HasCaught()) {
            FatalException(try_catch);
        }
    }

    void FileSystem::FSyncDir(fuse_req_t req,
                              fuse_ino_t ino,
                              int datasync_,
                              struct fuse_file_info* fi) {
        NanScope();
        Fuse* fuse = static_cast<Fuse *>(fuse_req_userdata(req));
        Local<Object> fsobj = NanNew(fuse->fsobj);

        Local<Value> vfsyncdir = fsobj->Get(NanNew(fsyncdir_sym));
        Local<Function> fsyncdir = Local<Function>::Cast(vfsyncdir);

        Local<Object> context = RequestContextToObject(fuse_req_ctx(req))->ToObject();
        Local<Number> inode = NanNew<Number>(ino);
        bool datasync = datasync_ == 0 ? false : true;

        FileInfo* info = new FileInfo();
        info->fi = fi;
        Local<Object> infoObj = NanNew(info->constructor_template)->GetFunction()->NewInstance();
        info->Wrap(infoObj);

        Reply* reply = new Reply();
        reply->request = req;
        Local<Object> replyObj = NanNew(reply->constructor_template)->GetFunction()->NewInstance();
        reply->Wrap(replyObj);

        Local<Value> argv[5] = {context, inode,
                                NanNew<Boolean>(datasync)->ToObject(),
                                infoObj, replyObj};

        TryCatch try_catch;

        fsyncdir->Call(fsobj, 5, argv);

        if (try_catch.HasCaught()) {
            FatalException(try_catch);
        }
    }

    void FileSystem::StatFs(fuse_req_t req, fuse_ino_t ino) {
        struct fuse_cmd *op = (struct fuse_cmd *)malloc(sizeof(struct fuse_cmd));
        op->op = _FUSE_OPS_STATFS_;
        op->req = req;
        op->ino = ino;
        if (ck_ring_enqueue_spmc(ck_ring, ck_ring_buffer, (void *) op) == false) {
            printf("ckring was full while trying to statfs inode %d\n", (int) ino);
            return;
        }
        uv_async_send(&uv_async_handle);

    }
    void FileSystem::RemoteStatFs(fuse_req_t req, fuse_ino_t ino) {

        NanScope();
        Fuse* fuse = static_cast<Fuse *>(fuse_req_userdata(req));
        Local<Object> fsobj = NanNew(fuse->fsobj);

        Local<Value> vstatfs = fsobj->Get(NanNew(statfs_sym));
        Local<Function> statfs = Local<Function>::Cast(vstatfs);

        Local<Object> context = RequestContextToObject(fuse_req_ctx(req))->ToObject();
        Local<Number> inode = NanNew<Number>(ino);

        Reply* reply = new Reply();
        reply->request = req;
        Local<Object> replyObj = NanNew(reply->constructor_template)->GetFunction()->NewInstance();
        reply->Wrap(replyObj);

        Local<Value> argv[3] = {context, inode, replyObj};

        TryCatch try_catch;

        statfs->Call(fsobj, 3, argv);

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
        NanScope();
        Fuse *fuse = static_cast<Fuse *>(fuse_req_userdata(req));
        Local<Object> fsobj = NanNew(fuse->fsobj);

        Local<Value> vsetxattr = fsobj->Get(NanNew(setxattr_sym));
        Local<Function> setxattr = Local<Function>::Cast(vsetxattr);

        Local<Object> context = RequestContextToObject(fuse_req_ctx(req))->ToObject();
        Local<Number> inode = NanNew<Number>(ino);
        Local<String> name = NanNew<String>(name_);
        Local<String> value = NanNew<String>(value_);
#ifdef __APPLE__
        Local<Integer> position = NanNew<Integer>(position_);
#endif
        Local<Number> size = NanNew<Number>(size_);

        //TODO change for an object with accessors
        Local<Integer> flags = NanNew<Integer>(flags_);

        Reply *reply = new Reply();
        reply->request = req;
        Local<Object> replyObj = NanNew(reply->constructor_template)->GetFunction()->NewInstance();
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
        setxattr->Call(fsobj, 8, argv);
#else
        setxattr->Call(fsobj, 7, argv);
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
        NanScope();
        Fuse* fuse = static_cast<Fuse *>(fuse_req_userdata(req));
        Local<Object> fsobj = NanNew(fuse->fsobj);

        Local<Value> vgetxattr = fsobj->Get(NanNew(getxattr_sym));
        Local<Function> getxattr = Local<Function>::Cast(vgetxattr);

        Local<Object> context = RequestContextToObject(fuse_req_ctx(req))->ToObject();
        Local<Number> inode = NanNew<Number>(ino);
        Local<String> name = NanNew<String>(name_);
        Local<Number> size = NanNew<Number>(size_);
#ifdef __APPLE__
        Local<Integer> position = NanNew<Integer>(position_);
#endif


        Reply* reply = new Reply();
        reply->request = req;
        Local<Object> replyObj = NanNew(reply->constructor_template)->GetFunction()->NewInstance();
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
        getxattr->Call(fsobj, 6, argv);
#else
        getxattr->Call(fsobj, 5, argv);
#endif
        if (try_catch.HasCaught()) {
            FatalException(try_catch);
        }
    }

    void FileSystem::ListXAttr(fuse_req_t req,
                               fuse_ino_t ino,
                               size_t size_) {
        NanScope();
        Fuse* fuse = static_cast<Fuse *>(fuse_req_userdata(req));
        Local<Object> fsobj = NanNew(fuse->fsobj);

        Local<Value> vlistxattr = fsobj->Get(NanNew(listxattr_sym));
        Local<Function> listxattr = Local<Function>::Cast(vlistxattr);

        Local<Object> context = RequestContextToObject(fuse_req_ctx(req))->ToObject();
        Local<Number> inode = NanNew<Number>(ino);
        Local<Number> size = NanNew<Number>(size_);

        Reply* reply = new Reply();
        reply->request = req;
        Local<Object> replyObj = NanNew(reply->constructor_template)->GetFunction()->NewInstance();
        reply->Wrap(replyObj);

        Local<Value> argv[4] = {context, inode,
                                size, replyObj};
        TryCatch try_catch;

        listxattr->Call(fsobj, 4, argv);

        if (try_catch.HasCaught()) {
            FatalException(try_catch);
        }
    }

    void FileSystem::RemoveXAttr(fuse_req_t req,
                                 fuse_ino_t ino,
                                 const char* name_) {
        NanScope();
        Fuse* fuse = static_cast<Fuse *>(fuse_req_userdata(req));
        Local<Object> fsobj = NanNew(fuse->fsobj);

        Local<Value> vremovexattr = fsobj->Get(NanNew(removexattr_sym));
        Local<Function> removexattr = Local<Function>::Cast(vremovexattr);

        Local<Object> context = RequestContextToObject(fuse_req_ctx(req))->ToObject();
        Local<Number> inode = NanNew<Number>(ino);
        Local<String> name = NanNew<String>(name_);

        Reply* reply = new Reply();
        reply->request = req;
        Local<Object> replyObj = NanNew(reply->constructor_template)->GetFunction()->NewInstance();
        reply->Wrap(replyObj);

        Local<Value> argv[4] = {context, inode,
                                name, replyObj};
        TryCatch try_catch;

        removexattr->Call(fsobj, 4, argv);

        if (try_catch.HasCaught()) {
            FatalException(try_catch);
        }
    }

    void FileSystem::Access(fuse_req_t req,
                            fuse_ino_t ino,
                            int mask_) {
        NanScope();
        Fuse* fuse = static_cast<Fuse *>(fuse_req_userdata(req));
        Local<Object> fsobj = NanNew(fuse->fsobj);

        Local<Value> vaccess = fsobj->Get(NanNew(access_sym));
        Local<Function> access = Local<Function>::Cast(vaccess);

        Local<Object> context = RequestContextToObject(fuse_req_ctx(req))->ToObject();
        Local<Number> inode = NanNew<Number>(ino);
        Local<Integer> mask = NanNew<Integer>(mask_);

        Reply* reply = new Reply();
        reply->request = req;
        Local<Object> replyObj = NanNew(reply->constructor_template)->GetFunction()->NewInstance();
        reply->Wrap(replyObj);

        Local<Value> argv[4] = {context, inode,
                                mask, replyObj};

        TryCatch try_catch;

        access->Call(fsobj, 4, argv);

        if (try_catch.HasCaught()) {
            FatalException(try_catch);
        }
    }

    void FileSystem::Create(fuse_req_t req,
                            fuse_ino_t parent,
                            const char* name,
                            mode_t mode,
                            struct fuse_file_info* fi) {
        struct fuse_cmd *op = (struct fuse_cmd *)malloc(sizeof(struct fuse_cmd));
        op->op = _FUSE_OPS_CREATE_;
        op->req = req;
        op->ino = parent;
        op->name = name;
        op->mode = mode;
        op->s.fi = fi;
        if (ck_ring_enqueue_spmc(ck_ring, ck_ring_buffer, (void *) op) == false) {
            printf("ckring was full while trying to enqueue write at inode %d\n", (int) parent);
            return;
        }
        uv_async_send(&uv_async_handle);

    }
    void FileSystem::RemoteCreate(fuse_req_t req,
                            fuse_ino_t parent,
                            const char* name,
                            mode_t mode,
                            struct fuse_file_info* fi) {

        NanScope();
        Fuse* fuse = static_cast<Fuse *>(fuse_req_userdata(req));
        Local<Object> fsobj = NanNew(fuse->fsobj);

        Local<Value> vcreate = fsobj->Get(NanNew(create_sym));
        Local<Function> create = Local<Function>::Cast(vcreate);

        Local<Object> context = RequestContextToObject(fuse_req_ctx(req))->ToObject();
        Local<Number> parentInode = NanNew<Number>(parent);
        Local<String> name_ = NanNew<String>(name);
        Local<Integer> mode_ = NanNew<Integer>(mode);

        FileInfo* info = new FileInfo();
        info->fi = fi;
        Local<Object> infoObj = NanNew(info->constructor_template)->GetFunction()->NewInstance();
        info->Wrap(infoObj);

        Reply* reply = new Reply();
        reply->request = req;
        Local<Object> replyObj = NanNew(reply->constructor_template)->GetFunction()->NewInstance();
        reply->Wrap(replyObj);

        Local<Value> argv[6] = {context, parentInode,
                                name_, mode_,
                                infoObj, replyObj};

        TryCatch try_catch;

        create->Call(fsobj, 6, argv);

        if (try_catch.HasCaught()) {
            FatalException(try_catch);
        }
        //scope.Close(Undefined());

    }

    void FileSystem::GetLock(fuse_req_t req,
                             fuse_ino_t ino,
                             struct fuse_file_info* fi,
                             struct flock* lock) {
        NanScope();
        Fuse* fuse = static_cast<Fuse *>(fuse_req_userdata(req));
        Local<Object> fsobj = NanNew(fuse->fsobj);

        Local<Value> vgetlk = fsobj->Get(NanNew(getlk_sym));
        Local<Function> getlk = Local<Function>::Cast(vgetlk);

        Local<Object> context = RequestContextToObject(fuse_req_ctx(req))->ToObject();
        Local<Number> inode = NanNew<Number>(ino);

        FileInfo* info = new FileInfo();
        info->fi = fi;
        Local<Object> infoObj = NanNew(info->constructor_template)->GetFunction()->NewInstance();
        info->Wrap(infoObj);

        Local<Object> lockObj = FlockToObject(lock)->ToObject();

        Reply* reply = new Reply();
        reply->request = req;
        Local<Object> replyObj = NanNew(reply->constructor_template)->GetFunction()->NewInstance();
        reply->Wrap(replyObj);

        Local<Value> argv[5] = {context, inode,
                                infoObj, lockObj, replyObj};

        TryCatch try_catch;

        getlk->Call(fsobj, 5, argv);

        if (try_catch.HasCaught()) {
            FatalException(try_catch);
        }
    }

    void FileSystem::SetLock(fuse_req_t req,
                             fuse_ino_t ino,
                             struct fuse_file_info* fi,
                             struct flock* lock,
                             int sleep_) {
        NanScope();
        Fuse* fuse = static_cast<Fuse *>(fuse_req_userdata(req));
        Local<Object> fsobj = NanNew(fuse->fsobj);

        Local<Value> vsetlk = fsobj->Get(NanNew(setlk_sym));
        Local<Function> setlk = Local<Function>::Cast(vsetlk);

        Local<Object> context = RequestContextToObject(fuse_req_ctx(req))->ToObject();
        Local<Number> inode = NanNew<Number>(ino);
        Local<Integer> sleep = NanNew<Integer>(sleep_);

        FileInfo* info = new FileInfo();
        info->fi = fi;
        Local<Object> infoObj = NanNew(info->constructor_template)->GetFunction()->NewInstance();
        info->Wrap(infoObj);

        Local<Object> lockObj = FlockToObject(lock)->ToObject();

        Reply* reply = new Reply();
        reply->request = req;
        Local<Object> replyObj = NanNew(reply->constructor_template)->GetFunction()->NewInstance();
        reply->Wrap(replyObj);

        Local<Value> argv[6] = {context, inode,
                                infoObj, lockObj,
                                sleep, replyObj};

        TryCatch try_catch;

        setlk->Call(fsobj, 6, argv);

        if (try_catch.HasCaught()) {
            FatalException(try_catch);
        }
    }

    void FileSystem::BMap(fuse_req_t req,
                          fuse_ino_t ino,
                          size_t blocksize_,
                          uint64_t idx) {
        NanScope();
        Fuse* fuse = static_cast<Fuse *>(fuse_req_userdata(req));
        Local<Object> fsobj = NanNew(fuse->fsobj);

        Local<Value> vbmap = fsobj->Get(NanNew(bmap_sym));
        Local<Function> bmap = Local<Function>::Cast(vbmap);

        Local<Object> context = RequestContextToObject(fuse_req_ctx(req))->ToObject();
        Local<Number> inode = NanNew<Number>(ino);
        Local<Integer> blocksize = NanNew<Integer>(blocksize_);
        Local<Integer> index = NanNew<Integer>(idx);

        Reply* reply = new Reply();
        reply->request = req;
        Local<Object> replyObj = NanNew(reply->constructor_template)->GetFunction()->NewInstance();
        reply->Wrap(replyObj);

        Local<Value> argv[5] = {context, inode,
                                blocksize, index, replyObj};

        TryCatch try_catch;

        bmap->Call(fsobj, 5, argv);

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


