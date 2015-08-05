/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "filesystem.h"
#include "reply.h"
#include "file_info.h"
#include "bindings.h"
#include "node_buffer.h"

namespace NodeFuse {
    Persistent<Function> FileSystem::constructor;

    ck_ring_t *ck_ring;
    ck_ring_buffer_t ck_ring_buffer[_RING_SIZE_];

    static struct fuse_lowlevel_ops fuse_ops = {};
    
    void FileSystem::DispatchOp(uv_async_t* handle, int status)
    {
        struct fuse_cmd *op = NULL; //(struct fuse_cmd *)malloc(sizeof(struct fuse_cmd));

        while (ck_ring_dequeue_spmc(ck_ring, ck_ring_buffer, (void*) &op) == true){
            switch(op->op){
                case _FUSE_OPS_LOOKUP_:
                    RemoteLookup(op->req, op->ino, op->name);
                    break;
                case _FUSE_OPS_GETATTR_:
                    RemoteGetAttr(op->req, op->ino, op->fi);
                    break;
                case _FUSE_OPS_OPEN_:
                    RemoteOpen(op->req, op->ino, op->fi);
                    break;
                case _FUSE_OPS_READ_:
                    RemoteRead(op->req, op->ino, op->size, op->off, op->fi);
                    break;
                case _FUSE_OPS_READDIR_:
                    RemoteReadDir(op->req, op->ino, op->size, op->off, op->fi);
                    break;
                case _FUSE_OPS_INIT_:
                    RemoteInit(op->userdata, op->conn);
                    break;
                case _FUSE_OPS_DESTROY_:
                    RemoteDestroy(op->userdata);
                    break;
                case _FUSE_OPS_FORGET_:
                    RemoteForget(op->req, op->ino, op->nlookup);
                    break;
                case _FUSE_OPS_SETATTR_:
                    RemoteSetAttr(op->req, op->ino, op->attr, op->to_set, op->fi);
                    break;
                case _FUSE_OPS_READLINK_:
                    RemoteReadLink(op->req, op->ino);
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
                    RemoteSymLink(op->req, op->name, op->ino, op->newname);
                    break;
                case _FUSE_OPS_RENAME_:
                    RemoteRename(op->req, op->ino, op->name, op->newino, op->newname);
                    break;
                case _FUSE_OPS_LINK_:
                    RemoteLink(op->req, op->ino, op->newino, op->name);
                    break;
                case _FUSE_OPS_WRITE_:
                    RemoteWrite(op->req, op->ino, op->name, op->size, op->off, op->fi);
                    break;
                case _FUSE_OPS_FLUSH_:
                    RemoteFlush(op->req, op->ino, op->fi);
                    break;
                case _FUSE_OPS_RELEASE_:
                    RemoteRelease(op->req, op->ino, op->fi);
                    break;
                case _FUSE_OPS_FSYNC_:
                    RemoteFSync(op->req, op->ino, op->to_set, op->fi);
                    break;
                case _FUSE_OPS_OPENDIR_:
                    RemoteOpenDir(op->req, op->ino, op->fi);
                    break;
                case _FUSE_OPS_RELEASEDIR_:
                    RemoteReleaseDir(op->req, op->ino, op->fi);
                    break;
                case _FUSE_OPS_FSYNCDIR_:
                    RemoteFSyncDir(op->req, op->ino, op->to_set, op->fi);
                    break;
                case _FUSE_OPS_STATFS_:
                    RemoteStatFs(op->req, op->ino);
                    break;
                case _FUSE_OPS_SETXATTR_:
                    RemoteSetXAttr(op->req, op->ino,op->name, op->newname, op->size, op->to_set
                            #ifdef __APPLE__
                                              ,op->position
                            #endif
                        );

                    break;
                case _FUSE_OPS_GETXATTR_:
                    RemoteGetXAttr(op->req, op-> ino, op->name, op->size
                            #ifdef __APPLE__
                                              ,op-> position
                            #endif
                        );

                    break;
                case _FUSE_OPS_LISTXATTR_:
                    RemoteListXAttr(op->req, op->ino, op->size);
                    break;
                case _FUSE_OPS_REMOVEXATTR_:
                    RemoteRemoveXAttr(op->req, op->ino, op->name);
                    break;
                case _FUSE_OPS_ACCESS_:
                    RemoteAccess(op->req, op->ino, op->to_set);
                    break;
                case _FUSE_OPS_CREATE_:
                    RemoteCreate(op->req, op->ino, op->name, op->mode, op->fi);
                    break;
                case _FUSE_OPS_GETLK_:
                    break;
                case _FUSE_OPS_SETLK_:
                    break;
                case _FUSE_OPS_BMAP_:
                    break;
            }
                // free((void*)op->name);
                // free((void*)op->newname);
              // free(op->userdata);
            
            free(op);
        }
    }


    void FileSystem::Initialize(Handle<Object> target) {

        ck_ring = (ck_ring_t *) malloc(sizeof(ck_ring_t));
        ck_ring_init(ck_ring, _RING_SIZE_);

        fuse_ops.lookup     = FileSystem::Lookup;
        fuse_ops.getattr    = FileSystem::GetAttr;
        fuse_ops.open       = FileSystem::Open;
        fuse_ops.read       = FileSystem::Read;
        fuse_ops.readdir    = FileSystem::ReadDir;
        fuse_ops.write      = FileSystem::Write;
        fuse_ops.create     = FileSystem::Create;
        fuse_ops.setattr    = FileSystem::SetAttr;
        fuse_ops.init       = FileSystem::Init;
        fuse_ops.destroy    = FileSystem::Destroy;
        fuse_ops.forget     = FileSystem::Forget;
        fuse_ops.readlink   = FileSystem::ReadLink;
        fuse_ops.mknod      = FileSystem::MkNod;
        fuse_ops.mkdir      = FileSystem::MkDir;
        fuse_ops.unlink     = FileSystem::Unlink;
        fuse_ops.rmdir      = FileSystem::RmDir;
        fuse_ops.symlink    = FileSystem::SymLink;
        fuse_ops.rename     = FileSystem::Rename;
        fuse_ops.link       = FileSystem::Link;
        fuse_ops.flush      = FileSystem::Flush;
        fuse_ops.release    = FileSystem::Release;
        fuse_ops.fsync      = FileSystem::FSync;
        fuse_ops.opendir    = FileSystem::OpenDir;
        fuse_ops.releasedir = FileSystem::ReleaseDir;
        fuse_ops.fsyncdir   = FileSystem::FSyncDir;
        fuse_ops.statfs     = FileSystem::StatFs;
        // fuse_ops.setxattr   = FileSystem::SetXAttr;
        // fuse_ops.getxattr   = FileSystem::GetXAttr;
        // fuse_ops.listxattr  = FileSystem::ListXAttr;
        // fuse_ops.removexattr= FileSystem::RemoveXAttr;
        fuse_ops.access     = FileSystem::Access;
        // fuse_ops.getlk      = FileSystem::GetLock;
        // fuse_ops.setlk      = FileSystem::SetLock;
        // fuse_ops.bmap       = FileSystem::BMap;
        // fuse_ops.ioctl      = FileSystem::IOCtl;
        // fuse_ops.poll       = FileSystem::Poll;

        // init_sym Nan::New("init"));
        // NanAssignPersistent(destroy_sym,     Nan::New("destroy"));
        // NanAssignPersistent(lookup_sym,      Nan::New("lookup"));
        // NanAssignPersistent(forget_sym,      Nan::New("forget"));
        // NanAssignPersistent(getattr_sym,     Nan::New("getattr"));
        // NanAssignPersistent(setattr_sym,     Nan::New("setattr"));
        // NanAssignPersistent(readlink_sym,    Nan::New("readlink"));
        // NanAssignPersistent(mknod_sym,       Nan::New("mknod"));
        // NanAssignPersistent(mkdir_sym,       Nan::New("mkdir"));
        // NanAssignPersistent(unlink_sym,      Nan::New("unlink"));
        // NanAssignPersistent(rmdir_sym,       Nan::New("rmdir"));
        // NanAssignPersistent(symlink_sym,     Nan::New("symlink"));
        // NanAssignPersistent(rename_sym,      Nan::New("rename"));
        // NanAssignPersistent(link_sym,        Nan::New("link"));
        // NanAssignPersistent(open_sym,        Nan::New("open"));
        // NanAssignPersistent(read_sym,        Nan::New("read"));
        // NanAssignPersistent(write_sym,       Nan::New("write"));
        // NanAssignPersistent(flush_sym,       Nan::New("flush"));
        // NanAssignPersistent(release_sym,     Nan::New("release"));
        // NanAssignPersistent(fsync_sym,       Nan::New("fsync"));
        // NanAssignPersistent(opendir_sym,     Nan::New("opendir"));
        // NanAssignPersistent(readdir_sym,     Nan::New("readdir"));
        // NanAssignPersistent(releasedir_sym,  Nan::New("releasedir"));
        // NanAssignPersistent(fsyncdir_sym,    Nan::New("fsyncdir"));
        // NanAssignPersistent(statfs_sym,      Nan::New("statfs"));
        // NanAssignPersistent(setxattr_sym,    Nan::New("setxattr"));
        // NanAssignPersistent(getxattr_sym,    Nan::New("getxattr"));
        // NanAssignPersistent(listxattr_sym,   Nan::New("listxattr"));
        // NanAssignPersistent(removexattr_sym, Nan::New("removexattr"));
        // NanAssignPersistent(access_sym,      Nan::New("access"));
        // NanAssignPersistent(create_sym,      Nan::New("create"));
        // NanAssignPersistent(getlk_sym,       Nan::New("getlk"));
        // NanAssignPersistent(setlk_sym,       Nan::New("setlk"));
        // NanAssignPersistent(bmap_sym,        Nan::New("bmap"));
        // NanAssignPersistent(ioctl_sym,       Nan::New("ioctl"));
        // NanAssignPersistent(poll_sym,        Nan::New("poll"));
        // NanAssignPersistent(conn_info_proto_major_sym,     Nan::New("proto_major"));
        // NanAssignPersistent(conn_info_proto_minor_sym,     Nan::New("proto_minor"));
        // NanAssignPersistent(conn_info_async_read_sym,      Nan::New("async_read"));
        // NanAssignPersistent(conn_info_max_write_sym,       Nan::New("max_write"));
        // NanAssignPersistent(conn_info_max_readahead_sym,   Nan::New("max_readahead"));
        // NanAssignPersistent(conn_info_capable_sym,         Nan::New("capable"));
        // NanAssignPersistent(conn_info_want_sym,            Nan::New("want"));
    }

    void FileSystem::Init(void* userdata,
                          struct fuse_conn_info* conn) {
        struct fuse_cmd *op = (struct fuse_cmd *)malloc(sizeof(struct fuse_cmd));
        op->op = _FUSE_OPS_INIT_;
        op->userdata =  userdata;
        memcpy( &(op->conn), conn, sizeof(struct fuse_conn_info));
        if (ck_ring_enqueue_spmc(ck_ring, ck_ring_buffer, (void *) op) == false) {
            printf("ckring was full while trying to enqueue init");
            return;
        }
        uv_async_send(&uv_async_handle);

    }
    void FileSystem::RemoteInit(void* userdata,
                          struct fuse_conn_info conn) {
        Nan::HandleScope scope;;
        Fuse* fuse = static_cast<Fuse *>(userdata);
        Local<Object> fsobj = Nan::New(fuse->fsobj);
        Local<Value> vinit = fsobj->Get(Nan::New<String>("init").ToLocalChecked());
        Local<Function> init = Local<Function>::Cast(vinit);

        //These properties will be read-only for now.
        //TODO set accessors for read/write properties
        Local<Object> info = Nan::New<Object>();
        info->Set(Nan::New<String>("conn_info_proto_major").ToLocalChecked(), Nan::New<Integer>(conn.proto_major));
        info->Set(Nan::New<String>("conn_info_proto_minor").ToLocalChecked(), Nan::New<Integer>(conn.proto_minor));
        info->Set(Nan::New<String>("conn_info_async_read").ToLocalChecked(), Nan::New<Integer>(conn.async_read));
        info->Set(Nan::New<String>("conn_info_max_write").ToLocalChecked(), Nan::New<Number>(conn.max_write));
        info->Set(Nan::New<String>("conn_info_max_readahead").ToLocalChecked(), Nan::New<Number>(conn.max_readahead));
        //TODO macro to enable certain properties given the fuse version
        //info->Set(conn_info_capable_sym, Nan::New<Integer>(conn.capable));
        //info->Set(conn_info_want_sym, Nan::New<Integer>(conn.want));

        Local<Value> argv[1] = {info};

        Nan::TryCatch try_catch;

        init->Call(fsobj, 1, argv);

        if (try_catch.HasCaught()) {
            Nan::FatalException(try_catch);
        }
    }
    void FileSystem::Destroy(void* userdata) {
        struct fuse_cmd *op = (struct fuse_cmd *)malloc(sizeof(struct fuse_cmd));

        op->op = _FUSE_OPS_DESTROY_;
        op->userdata =  userdata;        
        if (ck_ring_enqueue_spmc(ck_ring, ck_ring_buffer, (void *) op) == false) {
            printf("ckring was full while trying to enqueue init");
            return;
        }
        uv_async_send(&uv_async_handle);
    }

    void FileSystem::RemoteDestroy(void* userdata) {
        Nan::HandleScope scope;;
        Fuse* fuse = static_cast<Fuse *>(userdata);
        Local<Object> fsobj = Nan::New(fuse->fsobj);
        Local<Value> vdestroy = fsobj->Get(Nan::New<String>("destroy").ToLocalChecked());
        Local<Function> destroy = Local<Function>::Cast(vdestroy);

        Nan::TryCatch try_catch;

        destroy->Call(fsobj, 0, NULL);

        if (try_catch.HasCaught()) {
            Nan::FatalException(try_catch);
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
        Nan::HandleScope scope;;
        Fuse* fuse = static_cast<Fuse *>(fuse_req_userdata(req));
        Local<Object> fsobj = Nan::New(fuse->fsobj);
        Local<Value> vlookup = fsobj->Get(Nan::New<String>("lookup").ToLocalChecked());
        Local<Function> lookup = Local<Function>::Cast(vlookup);

        Local<Object> context = RequestContextToObject(fuse_req_ctx(req))->ToObject();
        Local<Number> parentInode = Nan::New<Number>(parent);
        Local<String> entryName = Nan::New<String>(name).ToLocalChecked();

        Reply* reply = new Reply();
        reply->request = req;
        Local<Object> replyObj = Nan::NewInstance( Nan::New<Function>(reply->constructor)).ToLocalChecked();//->GetFunction()->NewInstance();
        reply->Wrap(replyObj);

        Local<Value> argv[4] = {context, parentInode,
                                entryName, replyObj};
        Nan::TryCatch try_catch;

        lookup->Call(fsobj, 4, argv);

        if (try_catch.HasCaught()) {
            Nan::FatalException(try_catch);
        }
        //scope.Close(Undefined());

    }
    void FileSystem::Forget(fuse_req_t req,
                            fuse_ino_t ino,
                            unsigned long nlookup) {
        struct fuse_cmd *op = (struct fuse_cmd *)malloc(sizeof(struct fuse_cmd));
        op->op = _FUSE_OPS_FORGET_;
        op->req = req;
        op->ino = ino;
        op->nlookup = nlookup;
        if (ck_ring_enqueue_spmc(ck_ring, ck_ring_buffer, (void *) op) == false) {
            printf("ckring was full while trying to enqueue forget at inode %d\n", (int)ino);
            return;
        }
        uv_async_send(&uv_async_handle);

    }

    void FileSystem::RemoteForget(fuse_req_t req,
                            fuse_ino_t ino,
                            unsigned long nlookup) {
        Nan::HandleScope scope;;
        Fuse* fuse = static_cast<Fuse *>(fuse_req_userdata(req));
        Local<Object> fsobj = Nan::New(fuse->fsobj);
        Local<Value> vforget = fsobj->Get(Nan::New<String>("forget").ToLocalChecked());
        Local<Function> forget = Local<Function>::Cast(vforget);

        Local<Object> context = RequestContextToObject(fuse_req_ctx(req))->ToObject();
        Local<Number> inode = Nan::New<Number>(ino);
        Local<Integer> nlookup_ = Nan::New<Integer>( (int) nlookup);

        Local<Value> argv[3] = {context, inode, nlookup_};

        Nan::TryCatch try_catch;

        forget->Call(fsobj, 3, argv);

        if (try_catch.HasCaught()) {
            Nan::FatalException(try_catch);
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
        if(fi != NULL){
            memcpy( (void *) &(op->fi),  fi, sizeof(struct fuse_file_info));
        }
        if (ck_ring_enqueue_spmc(ck_ring, ck_ring_buffer, (void *) op) == false) {
            printf("ckring was full while trying to enqueue getattr at inode %d\n", (int) ino);
            return;
        }

        uv_async_send(&uv_async_handle);

    }
    void FileSystem::RemoteGetAttr(fuse_req_t req,
                             fuse_ino_t ino,
                             struct fuse_file_info fi) {
        Nan::HandleScope scope;;
        Fuse* fuse = static_cast<Fuse *>(fuse_req_userdata(req));
        Local<Object> fsobj = Nan::New(fuse->fsobj);

        Local<Value> vgetattr = fsobj->Get(Nan::New<String>("getattr").ToLocalChecked());
        Local<Function> getattr = Local<Function>::Cast(vgetattr);

        Local<Object> context = RequestContextToObject(fuse_req_ctx(req))->ToObject();
        Local<Number> inode = Nan::New<Number>(ino);

        Reply* reply = new Reply();
        reply->request = req;
        Local<Object> replyObj = Nan::NewInstance( Nan::New<Function>(reply->constructor)).ToLocalChecked();//->GetFunction()->NewInstance();
        reply->Wrap(replyObj);

        Local<Value> argv[3] = {context, inode, replyObj};

        Nan::TryCatch try_catch;

        getattr->Call(fsobj, 3, argv);

        if (try_catch.HasCaught()) {
            Nan::FatalException(try_catch);
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
        memcpy( (void*) &(op->attr), attr, sizeof(struct stat) );
        op->to_set = to_set;
        if(fi != NULL){
            memcpy( (void*) &(op->fi), fi, sizeof(struct fuse_file_info));
        }
        if (ck_ring_enqueue_spmc(ck_ring, ck_ring_buffer, (void *) op) == false) {
            printf("ckring was full while trying to enqueue setattr at inode %d\n", (int) ino);
            return;
        }

        uv_async_send(&uv_async_handle);

    }
    void FileSystem::RemoteSetAttr(fuse_req_t req,
                             fuse_ino_t ino,
                             struct stat attr_,
                             int to_set,
                             struct fuse_file_info fi) {

        Nan::HandleScope scope;;
        Fuse *fuse = static_cast<Fuse *>(fuse_req_userdata(req));
        Local<Object> fsobj = Nan::New(fuse->fsobj);

        Local<Value> vsetattr = fsobj->Get(Nan::New<String>("setattr").ToLocalChecked());
        Local<Function> setattr = Local<Function>::Cast(vsetattr);

        Local<Object> context = RequestContextToObject(fuse_req_ctx(req))->ToObject();
        Local<Number> inode = Nan::New<Number>(ino);

        struct stat *attr = (struct stat*) malloc(sizeof(struct stat));
        memcpy( (void*) attr, (const void *) &attr_, sizeof(struct stat));         
        Local<Object> attrs = GetAttrsToBeSet(to_set, attr)->ToObject();
        // free(attr);

        Reply *reply = new Reply();
        reply->request = req;
        Local<Object> replyObj = Nan::NewInstance( Nan::New<Function>(reply->constructor)).ToLocalChecked();//->GetFunction()->NewInstance();
        reply->Wrap(replyObj);

        Local<Value> argv[4] = {context, inode, attrs, replyObj};

        Nan::TryCatch try_catch;

        setattr->Call(fsobj, 4, argv);

        if (try_catch.HasCaught()) {
            Nan::FatalException(try_catch);
        }
        //scope.Close(Undefined());

    }

    void FileSystem::ReadLink(fuse_req_t req, fuse_ino_t ino) {
        struct fuse_cmd *op = (struct fuse_cmd *)malloc(sizeof(struct fuse_cmd));
        op->op = _FUSE_OPS_READLINK_;
        op->req = req;
        op->ino = ino;
        if (ck_ring_enqueue_spmc(ck_ring, ck_ring_buffer, (void *) op) == false) {
            printf("ckring was full while trying to enqueue setattr at inode %d\n", (int) ino);
            return;
        }

        uv_async_send(&uv_async_handle);
    }
    void FileSystem::RemoteReadLink(fuse_req_t req, fuse_ino_t ino) {
        Nan::HandleScope scope;;
        Fuse* fuse = static_cast<Fuse *>(fuse_req_userdata(req));
        Local<Object> fsobj = Nan::New(fuse->fsobj);

        Local<Value> vreadlink = fsobj->Get(Nan::New<String>("readlink").ToLocalChecked());
        Local<Function> readlink = Local<Function>::Cast(vreadlink);

        Local<Object> context = RequestContextToObject(fuse_req_ctx(req))->ToObject();
        Local<Number> inode = Nan::New<Number>(ino);

        Reply* reply = new Reply();
        reply->request = req;
        Local<Object> replyObj = Nan::NewInstance(Nan::New<Function>(reply->constructor)).ToLocalChecked();//->GetFunction()->NewInstance();
        reply->Wrap(replyObj);

        Local<Value> argv[3] = {context, inode, replyObj};

        Nan::TryCatch try_catch;

        readlink->Call(fsobj, 3, argv);

        if (try_catch.HasCaught()) {
            Nan::FatalException(try_catch);
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

        Nan::HandleScope scope;;
        Fuse* fuse = static_cast<Fuse *>(fuse_req_userdata(req));
        Local<Object> fsobj = Nan::New(fuse->fsobj);

        Local<Value> vmknod = fsobj->Get(Nan::New<String>("mknod").ToLocalChecked());
        Local<Function> mknod = Local<Function>::Cast(vmknod);

        Local<Object> context = RequestContextToObject(fuse_req_ctx(req))->ToObject();
        Local<Number> parentInode = Nan::New<Number>(parent);

        Local<String> name_ = Nan::New<String>(name).ToLocalChecked();
        Local<Integer> mode_ = Nan::New<Integer>(mode);
        Local<Integer> rdev_ = Nan::New<Integer>((uint32_t)rdev);

        Reply* reply = new Reply();
        reply->request = req;
        Local<Object> replyObj = Nan::NewInstance( Nan::New<Function>(reply->constructor)).ToLocalChecked();//->GetFunction()->NewInstance();
        reply->Wrap(replyObj);

        Local<Value> argv[6] = {context, parentInode,
                                name_, mode_,
                                rdev_, replyObj};

        Nan::TryCatch try_catch;

        mknod->Call(fsobj, 6, argv);

        if (try_catch.HasCaught()) {
            Nan::FatalException(try_catch);
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

        Nan::HandleScope scope;;
        Fuse* fuse = static_cast<Fuse *>(fuse_req_userdata(req));
        Local<Object> fsobj = Nan::New(fuse->fsobj);

        Local<Value> vmkdir = fsobj->Get(Nan::New<String>("mkdir").ToLocalChecked());
        Local<Function> mkdir = Local<Function>::Cast(vmkdir);

        Local<Object> context = RequestContextToObject(fuse_req_ctx(req))->ToObject();
        Local<Number> parentInode = Nan::New<Number>(parent);

        Local<String> name_ = Nan::New<String>(name).ToLocalChecked();
        Local<Integer> mode_ = Nan::New<Integer>(mode);

        Reply* reply = new Reply();
        reply->request = req;
        Local<Object> replyObj = Nan::NewInstance( Nan::New<Function>(reply->constructor)).ToLocalChecked();//->GetFunction()->NewInstance();
        reply->Wrap(replyObj);

        Local<Value> argv[5] = {context, parentInode,
                                name_, mode_, replyObj};

        Nan::TryCatch try_catch;

        mkdir->Call(fsobj, 5, argv);

        if (try_catch.HasCaught()) {
            Nan::FatalException(try_catch);
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

        Nan::HandleScope scope;;
        Fuse* fuse = static_cast<Fuse *>(fuse_req_userdata(req));
        Local<Object> fsobj = Nan::New(fuse->fsobj);

        Local<Value> vunlink = fsobj->Get(Nan::New<String>("unlink").ToLocalChecked());
        Local<Function> unlink = Local<Function>::Cast(vunlink);

        Local<Object> context = RequestContextToObject(fuse_req_ctx(req))->ToObject();
        Local<Number> parentInode = Nan::New<Number>(parent);
        Local<String> name_ = Nan::New<String>(name).ToLocalChecked();

        Reply* reply = new Reply();
        reply->request = req;
        Local<Object> replyObj = Nan::NewInstance( Nan::New<Function>(reply->constructor)).ToLocalChecked();//->GetFunction()->NewInstance();
        reply->Wrap(replyObj);

        Local<Value> argv[4] = {context, parentInode, name_, replyObj};

        Nan::TryCatch try_catch;

        unlink->Call(fsobj, 4, argv);

        if (try_catch.HasCaught()) {
            Nan::FatalException(try_catch);
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

        Nan::HandleScope scope;;
        Fuse* fuse = static_cast<Fuse *>(fuse_req_userdata(req));
        Local<Object> fsobj = Nan::New(fuse->fsobj);

        Local<Value> vrmdir = fsobj->Get(Nan::New<String>("rmdir").ToLocalChecked());
        Local<Function> rmdir = Local<Function>::Cast(vrmdir);

        Local<Object> context = RequestContextToObject(fuse_req_ctx(req))->ToObject();
        Local<Number> parentInode = Nan::New<Number>(parent);
        Local<String> name_ = Nan::New<String>(name).ToLocalChecked();

        Reply* reply = new Reply();
        reply->request = req;
        Local<Object> replyObj = Nan::NewInstance( Nan::New<Function>(reply->constructor)).ToLocalChecked();//->GetFunction()->NewInstance();
        reply->Wrap(replyObj);

        Local<Value> argv[4] = {context, parentInode, name_, replyObj};

        Nan::TryCatch try_catch;

        rmdir->Call(fsobj, 4, argv);

        if (try_catch.HasCaught()) {
            Nan::FatalException(try_catch);
        }
        //scope.Close(Undefined());
    }

    void FileSystem::SymLink(fuse_req_t req,
                             const char* link,
                             fuse_ino_t parent,
                             const char* name) {
        struct fuse_cmd *op = (struct fuse_cmd *)malloc(sizeof(struct fuse_cmd));
        op->op = _FUSE_OPS_SYMLINK_;
        op->name = link;
        op->ino = parent;
        op->newname = name;
        if (ck_ring_enqueue_spmc(ck_ring, ck_ring_buffer, (void *) op) == false) {
            printf("ckring was full while trying to symlink from %s to %s with parent inode %d \n",  name, link, (int) parent);
            return;
        }

        uv_async_send(&uv_async_handle);

    }

    void FileSystem::RemoteSymLink(fuse_req_t req,
                             const char* link,
                             fuse_ino_t parent,
                             const char* name) {
        Nan::HandleScope scope;;
        Fuse* fuse = static_cast<Fuse *>(fuse_req_userdata(req));
        Local<Object> fsobj = Nan::New(fuse->fsobj);

        Local<Value> vsymlink = fsobj->Get(Nan::New<String>("symlink").ToLocalChecked());
        Local<Function> symlink = Local<Function>::Cast(vsymlink);

        Local<Object> context = RequestContextToObject(fuse_req_ctx(req))->ToObject();
        Local<Number> parentInode = Nan::New<Number>(parent);
        Local<String> name_ = Nan::New<String>(name).ToLocalChecked();
        Local<String> link_ = Nan::New<String>(link).ToLocalChecked();

        Reply* reply = new Reply();
        reply->request = req;
        Local<Object> replyObj = Nan::NewInstance( Nan::New<Function>(reply->constructor)).ToLocalChecked();//->GetFunction()->NewInstance();
        reply->Wrap(replyObj);

        Local<Value> argv[5] = {context, parentInode,
                                link_, name_, replyObj};

        Nan::TryCatch try_catch;

        symlink->Call(fsobj, 5, argv);

        if (try_catch.HasCaught()) {
            Nan::FatalException(try_catch);
        }
    }
    void FileSystem::Rename(fuse_req_t req,
                                fuse_ino_t parent,
                                const char *name,
                                fuse_ino_t newparent,
                                const char *newname){
        struct fuse_cmd *op = (struct fuse_cmd *)malloc(sizeof(struct fuse_cmd));
        op->op = _FUSE_OPS_RENAME_;
        op->req = req;
        op->ino = parent;
        op->newino = newparent;
        op->name = name;
        op->newname = newname;
        if (ck_ring_enqueue_spmc(ck_ring, ck_ring_buffer, (void *) op) == false) {
            printf("ckring was full while trying to rename file %s from parent %d \n",  name, (int) parent);
            return;
        }

        uv_async_send(&uv_async_handle);

    }

    void FileSystem::RemoteRename(fuse_req_t req,
                            fuse_ino_t parent,
                            const char *name,
                            fuse_ino_t newparent,
                            const char *newname) {
        Nan::HandleScope scope;
        Fuse* fuse = static_cast<Fuse *>(fuse_req_userdata(req));
        Local<Object> fsobj = Nan::New(fuse->fsobj);

        Local<Value> vrename = fsobj->Get(Nan::New<String>("rename").ToLocalChecked());
        Local<Function> rename = Local<Function>::Cast(vrename);

        Local<Object> context = RequestContextToObject(fuse_req_ctx(req))->ToObject();
        Local<Number> parentInode = Nan::New<Number>(parent);
        Local<String> name_ = Nan::New<String>(name).ToLocalChecked();
        Local<Number> newParentInode = Nan::New<Number>(newparent);
        Local<String> newName = Nan::New<String>(newname).ToLocalChecked();

        Reply* reply = new Reply();
        reply->request = req;
        Local<Object> replyObj = Nan::NewInstance( Nan::New<Function>(reply->constructor)).ToLocalChecked();//->GetFunction()->NewInstance();
        reply->Wrap(replyObj);

        Local<Value> argv[6] = {context, parentInode,
                                name_, newParentInode,
                                newName, replyObj};

        Nan::TryCatch try_catch;

        rename->Call(fsobj, 6, argv);

        if (try_catch.HasCaught()) {
            Nan::FatalException(try_catch);
        }
    }
    void FileSystem::Link(fuse_req_t req,
                          fuse_ino_t ino,
                          fuse_ino_t newparent,
                          const char* newname) 
    {
        struct fuse_cmd *op = (struct fuse_cmd *)malloc(sizeof(struct fuse_cmd));
        op->op = _FUSE_OPS_LINK_;
        op->req = req;
        op->ino = ino;
        op->newino = newparent;
        op->name = newname;
        if (ck_ring_enqueue_spmc(ck_ring, ck_ring_buffer, (void *) op) == false) {
            printf("ckring was full while trying to link inode %d to new parent parent %d with newname %s\n",  (int) ino, (int) newparent, newname);
            return;
        }

        uv_async_send(&uv_async_handle);
    }

    void FileSystem::RemoteLink(fuse_req_t req,
                          fuse_ino_t ino,
                          fuse_ino_t newparent,
                          const char* newname) {
        Nan::HandleScope scope;;
        Fuse* fuse = static_cast<Fuse *>(fuse_req_userdata(req));
        Local<Object> fsobj = Nan::New(fuse->fsobj);

        Local<Value> vlink = fsobj->Get(Nan::New<String>("link").ToLocalChecked());
        Local<Function> link = Local<Function>::Cast(vlink);

        Local<Object> context = RequestContextToObject(fuse_req_ctx(req))->ToObject();
        Local<Number> inode = Nan::New<Number>(ino);
        Local<Number> newParent = Nan::New<Number>(newparent);
        Local<String> newName = Nan::New<String>(newname).ToLocalChecked();

        Reply* reply = new Reply();
        reply->request = req;
        Local<Object> replyObj = Nan::NewInstance( Nan::New<Function>(reply->constructor)).ToLocalChecked();//->GetFunction()->NewInstance();
        reply->Wrap(replyObj);

        Local<Value> argv[5] = {context, inode,
                                newParent, newName, replyObj};

        Nan::TryCatch try_catch;

        link->Call(fsobj, 5, argv);

        if (try_catch.HasCaught()) {
            Nan::FatalException(try_catch);
        }
    }

    void FileSystem::Open(fuse_req_t req,
                          fuse_ino_t ino,
                          struct fuse_file_info* fi) {
        struct fuse_cmd *op = (struct fuse_cmd *)malloc(sizeof(struct fuse_cmd));
        op->op = _FUSE_OPS_OPEN_;
        op->req = req;
        op->ino = ino;
        if(fi != NULL){
            memcpy( (void*) &(op->fi), fi, sizeof(struct fuse_file_info));
        }
        if (ck_ring_enqueue_spmc(ck_ring, ck_ring_buffer, (void *) op) == false) {
            printf("ckring was full while trying to enqueue open at inode %d\n", (int) ino);
            return;
        }
        uv_async_send(&uv_async_handle);

    }
    void FileSystem::RemoteOpen(fuse_req_t req,
                          fuse_ino_t ino,
                          struct fuse_file_info fi) {

        Nan::HandleScope scope;;
        Fuse* fuse = static_cast<Fuse *>(fuse_req_userdata(req));
        Local<Object> fsobj = Nan::New(fuse->fsobj);

        Local<Value> vopen = fsobj->Get(Nan::New<String>("open").ToLocalChecked());
        Local<Function> open = Local<Function>::Cast(vopen);

        Local<Object> context = RequestContextToObject(fuse_req_ctx(req))->ToObject();
        Local<Number> inode = Nan::New<Number>(ino);

        FileInfo* info = new FileInfo();
        info->fi = (struct fuse_file_info* ) malloc(sizeof(struct fuse_file_info));
        memcpy( info->fi, &fi, sizeof(struct fuse_file_info));
        Local<Object> infoObj = Nan::NewInstance(Nan::New<Function>(info->constructor)).ToLocalChecked();//->GetFunction()->NewInstance();
        info->Wrap(infoObj);

        Reply* reply = new Reply();
        reply->request = req;
        Local<Object> replyObj = Nan::NewInstance( Nan::New<Function>(reply->constructor)).ToLocalChecked();//->GetFunction()->NewInstance();
        reply->Wrap(replyObj);

        Local<Value> argv[4] = {context, inode,
                                infoObj, replyObj};

        Nan::TryCatch try_catch;

        open->Call(fsobj, 4, argv);

        if (try_catch.HasCaught()) {
            Nan::FatalException(try_catch);
        }
        //scope.Close(Undefined());

    }

    void FileSystem::Read(fuse_req_t req,
                          fuse_ino_t ino,
                          size_t size,
                          off_t off,
                          struct fuse_file_info* fi) {
        struct fuse_cmd *op = (struct fuse_cmd *)malloc(sizeof(struct fuse_cmd));
        op->op = _FUSE_OPS_READ_;
        op->req = req;
        op->ino = ino;
        op->off = off;
        op->size = size;
        if(fi != NULL){
            memcpy( (void*) &(op->fi), fi, sizeof(struct fuse_file_info));
        }
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
                          struct fuse_file_info fi) {

        Nan::HandleScope scope;;
        Fuse* fuse = static_cast<Fuse *>(fuse_req_userdata(req));
        Local<Object> fsobj = Nan::New(fuse->fsobj);

        Local<Value> vread = fsobj->Get(Nan::New<String>("read").ToLocalChecked());
        Local<Function> read = Local<Function>::Cast(vread);

        Local<Object> context = RequestContextToObject(fuse_req_ctx(req))->ToObject();
        Local<Number> inode = Nan::New<Number>(ino);
        Local<Number> size = Nan::New<Number>(size_);
        Local<Number> offset = Nan::New<Number>(off);

        FileInfo* info = new FileInfo();
        info->fi = (struct fuse_file_info*) malloc(sizeof(struct fuse_file_info) );
        memcpy( (void*) info->fi , &fi, sizeof(struct fuse_file_info));
        Local<Object> infoObj = Nan::NewInstance(Nan::New<Function>(info->constructor)).ToLocalChecked();//->GetFunction()->NewInstance();
        info->Wrap(infoObj);

        Reply* reply = new Reply();
        reply->request = req;
        Local<Object> replyObj = Nan::NewInstance( Nan::New<Function>(reply->constructor)).ToLocalChecked();//->GetFunction()->NewInstance();
        reply->Wrap(replyObj);

        Local<Value> argv[6] = {context, inode,
                                size, offset,
                                infoObj, replyObj};

        Nan::TryCatch try_catch;

        read->Call(fsobj, 6, argv);

        if (try_catch.HasCaught()) {
            Nan::FatalException(try_catch);
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
        op->name = (char *)malloc(size);
        memcpy((void *)op->name, buf, size);

        if(fi != NULL){
            memcpy( (void*) &(op->fi), fi, sizeof(struct fuse_file_info));
        }
        if (ck_ring_enqueue_spmc(ck_ring, ck_ring_buffer, (void *) op) == false) {
            printf("ckring was full while trying to enqueue write at inode %d\n", (int) ino);
            return;
        }
        uv_async_send(&uv_async_handle);


    }
    void FileSystem::RemoteWrite(fuse_req_t req,
                           fuse_ino_t ino,
                           const char *buf,
                           size_t size,
                           off_t off,
                           struct fuse_file_info fi){
        Nan::HandleScope scope;;
        Fuse* fuse = static_cast<Fuse *>(fuse_req_userdata(req));
        Local<Object> fsobj = Nan::New(fuse->fsobj);

        Local<Value> vwrite = fsobj->Get(Nan::New<String>("write").ToLocalChecked());
        Local<Function> write = Local<Function>::Cast(vwrite);

        Local<Object> context = RequestContextToObject(fuse_req_ctx(req))->ToObject();
        Local<Number> inode = Nan::New<Number>(ino);
        Local<Number> offset = Nan::New<Number>(off);

        Local<Object> buffer = Nan::NewBuffer((char*) buf, size).ToLocalChecked();

        FileInfo* info = new FileInfo();
        
        info->fi = (struct fuse_file_info*) malloc(sizeof(struct fuse_file_info) );
        memcpy( (void*) info->fi , &fi, sizeof(struct fuse_file_info));
        Local<Object> infoObj = Nan::NewInstance(Nan::New<Function>(info->constructor)).ToLocalChecked();//->GetFunction()->NewInstance();
        info->Wrap(infoObj);

        Reply* reply = new Reply();
        reply->request = req;
        Local<Object> replyObj = Nan::NewInstance( Nan::New<Function>(reply->constructor)).ToLocalChecked();//->GetFunction()->NewInstance();
        reply->Wrap(replyObj);

        Local<Value> argv[6] = {context, inode,
                                // Nan::New<Object>(buffer->handle_), offset,
                                buffer, offset,
                                infoObj, replyObj};

        Nan::TryCatch try_catch;
        write->Call(fsobj, 6, argv);

        if (try_catch.HasCaught()) {
            Nan::FatalException(try_catch);
        }
        //scope.Close(Undefined());

    }

    void FileSystem::Flush(fuse_req_t req,
                           fuse_ino_t ino,
                           struct fuse_file_info* fi) {
        struct fuse_cmd *op = (struct fuse_cmd *)malloc(sizeof(struct fuse_cmd));
        op->op = _FUSE_OPS_FLUSH_;
        op->req = req;
        op->ino = ino;
        if(fi != NULL){
            memcpy( (void*) &(op->fi), fi, sizeof(struct fuse_file_info));
        }
        if (ck_ring_enqueue_spmc(ck_ring, ck_ring_buffer, (void *) op) == false) {
            printf("ckring was full while trying to enqueue write at inode %d\n", (int) ino);
            return;
        }
        uv_async_send(&uv_async_handle);

    }
    void FileSystem::RemoteFlush(fuse_req_t req,
                           fuse_ino_t ino,
                           struct fuse_file_info fi) {

        Nan::HandleScope scope;;
        Fuse* fuse = static_cast<Fuse *>(fuse_req_userdata(req));
        Local<Object> fsobj = Nan::New(fuse->fsobj);

        Local<Value> vflush = fsobj->Get(Nan::New<String>("flush").ToLocalChecked());
        Local<Function> flush = Local<Function>::Cast(vflush);

        Local<Object> context = RequestContextToObject(fuse_req_ctx(req))->ToObject();
        Local<Number> inode = Nan::New<Number>(ino);

        FileInfo* info = new FileInfo();
        info->fi = (struct fuse_file_info*) malloc(sizeof(struct fuse_file_info) );
        memcpy( (void*) info->fi , &fi, sizeof(struct fuse_file_info));
        
        Local<Object> infoObj = Nan::NewInstance(Nan::New<Function>(info->constructor)).ToLocalChecked();//->GetFunction()->NewInstance();
        info->Wrap(infoObj);

        Reply* reply = new Reply();
        reply->request = req;
        Local<Object> replyObj = Nan::NewInstance( Nan::New<Function>(reply->constructor)).ToLocalChecked();//->GetFunction()->NewInstance();
        reply->Wrap(replyObj);

        Local<Value> argv[4] = {context, inode,
                                infoObj, replyObj};

        Nan::TryCatch try_catch;

        flush->Call(fsobj, 4, argv);

        if (try_catch.HasCaught()) {
            Nan::FatalException(try_catch);
        }
    }

    void FileSystem::Release(fuse_req_t req,
                             fuse_ino_t ino,
                             struct fuse_file_info* fi) {

        struct fuse_cmd *op = (struct fuse_cmd *)malloc(sizeof(struct fuse_cmd));
        op->op = _FUSE_OPS_RELEASE_;
        op->req = req;
        op->ino = ino;
        if(fi != NULL){
            memcpy( (void*) &(op->fi), fi, sizeof(struct fuse_file_info));
        }
        if (ck_ring_enqueue_spmc(ck_ring, ck_ring_buffer, (void *) op) == false) {
            printf("ckring was full while trying to release inode %d\n", (int) ino);
            return;
        }
        uv_async_send(&uv_async_handle);

    }
    void FileSystem::RemoteRelease(fuse_req_t req,
                             fuse_ino_t ino,
                             struct fuse_file_info fi) {

        Nan::HandleScope scope;;
        Fuse* fuse = static_cast<Fuse *>(fuse_req_userdata(req));
        Local<Object> fsobj = Nan::New(fuse->fsobj);

        Local<Value> vrelease = fsobj->Get(Nan::New<String>("release").ToLocalChecked());
        Local<Function> release = Local<Function>::Cast(vrelease);

        Local<Object> context = RequestContextToObject(fuse_req_ctx(req))->ToObject();
        Local<Number> inode = Nan::New<Number>(ino);

        FileInfo* info = new FileInfo();
        info->fi = (struct fuse_file_info*) malloc(sizeof(struct fuse_file_info) );
        memcpy( (void*) info->fi , &fi, sizeof(struct fuse_file_info));
        Local<Object> infoObj = Nan::NewInstance(Nan::New<Function>(info->constructor)).ToLocalChecked();//->GetFunction()->NewInstance();
        info->Wrap(infoObj);

        Reply* reply = new Reply();
        reply->request = req;
        Local<Object> replyObj = Nan::NewInstance( Nan::New<Function>(reply->constructor)).ToLocalChecked();//->GetFunction()->NewInstance();
        reply->Wrap(replyObj);

        Local<Value> argv[4] = {context, inode,
                                infoObj, replyObj};

        Nan::TryCatch try_catch;

        release->Call(fsobj, 4, argv);

        if (try_catch.HasCaught()) {
            Nan::FatalException(try_catch);
        }
        //scope.Close(Undefined());
    }
    void FileSystem::FSync(fuse_req_t req,
                           fuse_ino_t ino,
                           int datasync_,
                           struct fuse_file_info* fi) {
        struct fuse_cmd *op = (struct fuse_cmd *)malloc(sizeof(struct fuse_cmd));
        op->op = _FUSE_OPS_FSYNC_;
        op->req = req;
        op->ino = ino;
        op->to_set = datasync_;
        if(fi != NULL){
            memcpy( (void*) &(op->fi), fi, sizeof(struct fuse_file_info));
        }

        if (ck_ring_enqueue_spmc(ck_ring, ck_ring_buffer, (void *) op) == false) {
            printf("ckring was full while trying to fsync file %d\n",  (int) ino);
            return;
        }

        uv_async_send(&uv_async_handle);

    }

    void FileSystem::RemoteFSync(fuse_req_t req,
                           fuse_ino_t ino,
                           int datasync_,
                           struct fuse_file_info fi) {
        Nan::HandleScope scope;;
        Fuse* fuse = static_cast<Fuse *>(fuse_req_userdata(req));
        Local<Object> fsobj = Nan::New(fuse->fsobj);

        Local<Value> vfsync = fsobj->Get(Nan::New<String>("fsync").ToLocalChecked());
        Local<Function> fsync = Local<Function>::Cast(vfsync);

        Local<Object> context = RequestContextToObject(fuse_req_ctx(req))->ToObject();
        Local<Number> inode = Nan::New<Number>(ino);
        bool datasync = datasync_ == 0 ? false : true;

        FileInfo* info = new FileInfo();
        info->fi = (struct fuse_file_info*) malloc(sizeof(struct fuse_file_info) );
        memcpy( (void*) info->fi , &fi, sizeof(struct fuse_file_info));

        Local<Object> infoObj = Nan::NewInstance(Nan::New<Function>(info->constructor)).ToLocalChecked();//->GetFunction()->NewInstance();
        info->Wrap(infoObj);

        Reply* reply = new Reply();
        reply->request = req;
        Local<Object> replyObj = Nan::NewInstance( Nan::New<Function>(reply->constructor)).ToLocalChecked();//->GetFunction()->NewInstance();
        reply->Wrap(replyObj);

        Local<Value> argv[5] = {context, inode,
                                Nan::New<Boolean>(datasync)->ToObject(),
                                infoObj, replyObj};

        Nan::TryCatch try_catch;

        fsync->Call(fsobj, 5, argv);

        if (try_catch.HasCaught()) {
            Nan::FatalException(try_catch);
        }
    }
    void FileSystem::OpenDir(fuse_req_t req,
                             fuse_ino_t ino,
                             struct fuse_file_info* fi) {
        struct fuse_cmd *op = (struct fuse_cmd *)malloc(sizeof(struct fuse_cmd));
        op->op = _FUSE_OPS_OPENDIR_;
        op->req = req;
        op->ino = ino;
        if(fi != NULL){
            memcpy( (void*) &(op->fi), fi, sizeof(struct fuse_file_info));
        }

        if (ck_ring_enqueue_spmc(ck_ring, ck_ring_buffer, (void *) op) == false) {
            printf("ckring was full while trying to opendir with inode %d \n",  (int)ino);
            return;
        }

        uv_async_send(&uv_async_handle);
    }

    void FileSystem::RemoteOpenDir(fuse_req_t req,
                             fuse_ino_t ino,
                             struct fuse_file_info fi) {
        Nan::HandleScope scope;;
        Fuse* fuse = static_cast<Fuse *>(fuse_req_userdata(req));
        Local<Object> fsobj = Nan::New(fuse->fsobj);

        Local<Value> vopendir = fsobj->Get(Nan::New<String>("opendir").ToLocalChecked());
        Local<Function> opendir = Local<Function>::Cast(vopendir);

        Local<Object> context = RequestContextToObject(fuse_req_ctx(req))->ToObject();
        Local<Number> inode = Nan::New<Number>(ino);

        FileInfo* info = new FileInfo();
        info->fi = (struct fuse_file_info*) malloc(sizeof(struct fuse_file_info) );
        memcpy( (void*) info->fi , &fi, sizeof(struct fuse_file_info));
        Local<Object> infoObj = Nan::NewInstance(Nan::New<Function>(info->constructor)).ToLocalChecked();//->GetFunction()->NewInstance();
        info->Wrap(infoObj);

        Reply* reply = new Reply();
        reply->request = req;
        Local<Object> replyObj = Nan::NewInstance( Nan::New<Function>(reply->constructor)).ToLocalChecked();//->GetFunction()->NewInstance();
        reply->Wrap(replyObj);

        Local<Value> argv[4] = {context, inode,
                                infoObj, replyObj};

        Nan::TryCatch try_catch;

        opendir->Call(fsobj, 4, argv);

        if (try_catch.HasCaught()) {
            Nan::FatalException(try_catch);
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
        if(fi != NULL){
            memcpy( (void*) &(op->fi), fi, sizeof(struct fuse_file_info));
        }
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
                             struct fuse_file_info fi) {

        Nan::HandleScope scope;;
        Fuse* fuse = static_cast<Fuse *>(fuse_req_userdata(req));
        Local<Object> fsobj = Nan::New(fuse->fsobj);

        Local<Value> vreaddir = fsobj->Get(Nan::New<String>("readdir").ToLocalChecked());
        Local<Function> readdir = Local<Function>::Cast(vreaddir);

        Local<Object> context = RequestContextToObject(fuse_req_ctx(req))->ToObject();
        Local<Number> inode = Nan::New<Number>(ino);
        Local<Integer> size = Nan::New<Integer>((int)size_);
        Local<Integer> offset = Nan::New<Integer>((int) off);

        FileInfo* info = new FileInfo();
        info->fi = (struct fuse_file_info*) malloc(sizeof(struct fuse_file_info) );
        memcpy( (void*) info->fi , &fi, sizeof(struct fuse_file_info));
        Local<Object> infoObj = Nan::NewInstance(Nan::New<Function>(info->constructor)).ToLocalChecked();//->GetFunction()->NewInstance();
        info->Wrap(infoObj);

        Reply* reply = new Reply();
        reply->request = req;
        Local<Object> replyObj = Nan::NewInstance( Nan::New<Function>(reply->constructor)).ToLocalChecked();//->GetFunction()->NewInstance();
        reply->Wrap(replyObj);

        Local<Value> argv[6] = {context, inode,
                                size, offset,
                                infoObj, replyObj};

        Nan::TryCatch try_catch;

        readdir->Call(fsobj, 6, argv);

        if (try_catch.HasCaught()) {
            Nan::FatalException(try_catch);
        }
        //scope.Close(Undefined());

    }

    void FileSystem::ReleaseDir(fuse_req_t req,
                                fuse_ino_t ino,
                                struct fuse_file_info* fi) {
        struct fuse_cmd *op = (struct fuse_cmd *)malloc(sizeof(struct fuse_cmd));
        op->op = _FUSE_OPS_RELEASEDIR_;
        op->req = req;
        op->ino = ino;
        if(fi != NULL){
            memcpy( (void*) &(op->fi), fi, sizeof(struct fuse_file_info));
        }

        if (ck_ring_enqueue_spmc(ck_ring, ck_ring_buffer, (void *) op) == false) {
            printf("ckring was full while trying to releasedir with inode %d \n",  (int)ino);
            return;
        }

        uv_async_send(&uv_async_handle);

    }
    void FileSystem::RemoteReleaseDir(fuse_req_t req,
                                fuse_ino_t ino,
                                struct fuse_file_info fi) {

        Nan::HandleScope scope;;
        Fuse* fuse = static_cast<Fuse *>(fuse_req_userdata(req));
        Local<Object> fsobj = Nan::New(fuse->fsobj);

        Local<Value> vreleasedir = fsobj->Get(Nan::New<String>("releasedir").ToLocalChecked());
        Local<Function> releasedir = Local<Function>::Cast(vreleasedir);

        Local<Object> context = RequestContextToObject(fuse_req_ctx(req))->ToObject();
        Local<Number> inode = Nan::New<Number>(ino);

        FileInfo* info = new FileInfo();
        info->fi = (struct fuse_file_info*) malloc(sizeof(struct fuse_file_info) );
        memcpy( (void*) info->fi , &fi, sizeof(struct fuse_file_info));
        Local<Object> infoObj = Nan::NewInstance(Nan::New<Function>(info->constructor)).ToLocalChecked();//;
        info->Wrap(infoObj);

        Reply* reply = new Reply();
        reply->request = req;
        Local<Object> replyObj = Nan::NewInstance( Nan::New<Function>(reply->constructor)).ToLocalChecked();//->GetFunction()->NewInstance();
        reply->Wrap(replyObj);

        Local<Value> argv[4] = {context, inode,
                                infoObj, replyObj};

        Nan::TryCatch try_catch;

        releasedir->Call(fsobj, 4, argv);

        if (try_catch.HasCaught()) {
            Nan::FatalException(try_catch);
        }
    }

    void FileSystem::FSyncDir(fuse_req_t req,
                              fuse_ino_t ino,
                              int datasync_,
                              struct fuse_file_info* fi) {
        struct fuse_cmd *op = (struct fuse_cmd *)malloc(sizeof(struct fuse_cmd));
        op->op = _FUSE_OPS_FSYNCDIR_;
        op->req = req;
        op->ino = ino;
        op->to_set = datasync_;
        if(fi != NULL){
            memcpy( (void*) &(op->fi), fi, sizeof(struct fuse_file_info));
        }

        if (ck_ring_enqueue_spmc(ck_ring, ck_ring_buffer, (void *) op) == false) {
            printf("ckring was full while trying to fsync dir %d\n",  (int) ino);
            return;
        }

        uv_async_send(&uv_async_handle);
    }
    void FileSystem::RemoteFSyncDir(fuse_req_t req,
                              fuse_ino_t ino,
                              int datasync_,
                              struct fuse_file_info fi) {

        Nan::HandleScope scope;;
        Fuse* fuse = static_cast<Fuse *>(fuse_req_userdata(req));
        Local<Object> fsobj = Nan::New(fuse->fsobj);

        Local<Value> vfsyncdir = fsobj->Get(Nan::New<String>("fsyncdir").ToLocalChecked());
        Local<Function> fsyncdir = Local<Function>::Cast(vfsyncdir);

        Local<Object> context = RequestContextToObject(fuse_req_ctx(req))->ToObject();
        Local<Number> inode = Nan::New<Number>(ino);
        bool datasync = datasync_ == 0 ? false : true;

        FileInfo* info = new FileInfo();
        info->fi = (struct fuse_file_info*) malloc(sizeof(struct fuse_file_info) );
        memcpy( (void*) info->fi , &fi, sizeof(struct fuse_file_info));
        Local<Object> infoObj = Nan::NewInstance(Nan::New<Function>(info->constructor)).ToLocalChecked();//->GetFunction()->NewInstance();
        info->Wrap(infoObj);

        Reply* reply = new Reply();
        reply->request = req;
        Local<Object> replyObj = Nan::NewInstance( Nan::New<Function>(reply->constructor)).ToLocalChecked();//->GetFunction()->NewInstance();
        reply->Wrap(replyObj);

        Local<Value> argv[5] = {context, inode,
                                Nan::New<Boolean>(datasync)->ToObject(),
                                infoObj, replyObj};

        Nan::TryCatch try_catch;

        fsyncdir->Call(fsobj, 5, argv);

        if (try_catch.HasCaught()) {
            Nan::FatalException(try_catch);
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

        Nan::HandleScope scope;;
        Fuse* fuse = static_cast<Fuse *>(fuse_req_userdata(req));
        Local<Object> fsobj = Nan::New(fuse->fsobj);

        Local<Value> vstatfs = fsobj->Get(Nan::New<String>("statfs").ToLocalChecked());
        Local<Function> statfs = Local<Function>::Cast(vstatfs);

        Local<Object> context = RequestContextToObject(fuse_req_ctx(req))->ToObject();
        Local<Number> inode = Nan::New<Number>(ino);

        Reply* reply = new Reply();
        reply->request = req;
        Local<Object> replyObj = Nan::NewInstance( Nan::New<Function>(reply->constructor)).ToLocalChecked();//->GetFunction()->NewInstance();
        reply->Wrap(replyObj);

        Local<Value> argv[3] = {context, inode, replyObj};

        Nan::TryCatch try_catch;

        statfs->Call(fsobj, 3, argv);

        if (try_catch.HasCaught()) {
            Nan::FatalException(try_catch);
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
        struct fuse_cmd *op = (struct fuse_cmd *)malloc(sizeof(struct fuse_cmd));
        op->op = _FUSE_OPS_SETXATTR_;
        op->req = req;
        op->name = name_;
        op->newname = value_;
        op->size = size_;
        op->to_set = flags_;
        #ifdef __APPLE__
        op->position = position_;
        #endif

        if (ck_ring_enqueue_spmc(ck_ring, ck_ring_buffer, (void *) op) == false) {
            printf("ckring was full while trying to setxattr file %s from parent %d \n",  name_, (int) ino);
            return;
        }

        uv_async_send(&uv_async_handle);

    }
    void FileSystem::RemoteSetXAttr(fuse_req_t req,
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
        Nan::HandleScope scope;;
        Fuse *fuse = static_cast<Fuse *>(fuse_req_userdata(req));
        Local<Object> fsobj = Nan::New(fuse->fsobj);

        Local<Value> vsetxattr = fsobj->Get(Nan::New<String>("setxattr").ToLocalChecked());
        Local<Function> setxattr = Local<Function>::Cast(vsetxattr);

        Local<Object> context = RequestContextToObject(fuse_req_ctx(req))->ToObject();
        Local<Number> inode = Nan::New<Number>(ino);
        Local<String> name = Nan::New<String>(name_).ToLocalChecked();
        Local<String> value = Nan::New<String>(value_).ToLocalChecked();
#ifdef __APPLE__
        Local<Integer> position = Nan::New<Integer>(position_);
#endif
        Local<Number> size = Nan::New<Number>(size_);

        //TODO change for an object with accessors
        Local<Integer> flags = Nan::New<Integer>(flags_);

        Reply *reply = new Reply();
        reply->request = req;
        Local<Object> replyObj = Nan::NewInstance( Nan::New<Function>(reply->constructor)).ToLocalChecked();//->GetFunction()->NewInstance();
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
        Nan::TryCatch try_catch;

#ifdef __APPLE__
        setxattr->Call(fsobj, 8, argv);
#else
        setxattr->Call(fsobj, 7, argv);
#endif

        if (try_catch.HasCaught()) {
            Nan::FatalException(try_catch);
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
    struct fuse_cmd *op = (struct fuse_cmd *)malloc(sizeof(struct fuse_cmd));
    op->op = _FUSE_OPS_GETXATTR_;
    op->req = req;
    op->ino = ino;
    op->name = name_;
    op->size = size_;
    #ifdef __APPLE__
    op->position = position_;
    #endif

    if (ck_ring_enqueue_spmc(ck_ring, ck_ring_buffer, (void *) op) == false) {
        printf("ckring was full while trying to getxattr for inode %d\n",  (int) ino);
        return;
    }

    uv_async_send(&uv_async_handle);

    }
    void FileSystem::RemoteGetXAttr(fuse_req_t req,
                              fuse_ino_t ino,
                              const char* name_,
                              size_t size_
            #ifdef __APPLE__
                              ,uint32_t position_) {
            #else
                              ) {
            #endif

        Nan::HandleScope scope;;
        Fuse* fuse = static_cast<Fuse *>(fuse_req_userdata(req));
        Local<Object> fsobj = Nan::New(fuse->fsobj);

        Local<Value> vgetxattr = fsobj->Get(Nan::New<String>("getxattr").ToLocalChecked());
        Local<Function> getxattr = Local<Function>::Cast(vgetxattr);

        Local<Object> context = RequestContextToObject(fuse_req_ctx(req))->ToObject();
        Local<Number> inode = Nan::New<Number>(ino);
        Local<String> name = Nan::New<String>(name_).ToLocalChecked();
        Local<Number> size = Nan::New<Number>(size_);
#ifdef __APPLE__
        Local<Integer> position = Nan::New<Integer>(position_);
#endif


        Reply* reply = new Reply();
        reply->request = req;
        Local<Object> replyObj = Nan::NewInstance( Nan::New<Function>(reply->constructor)).ToLocalChecked();//->GetFunction()->NewInstance();
        reply->Wrap(replyObj);

#ifdef __APPLE__
        Local<Value> argv[6] = {context, inode,
                                name, size,
                                position, replyObj};
#else
        Local<Value> argv[5] = {context, inode,
                                name, size, replyObj};
#endif

        Nan::TryCatch try_catch;

#ifdef __APPLE__
        getxattr->Call(fsobj, 6, argv);
#else
        getxattr->Call(fsobj, 5, argv);
#endif
        if (try_catch.HasCaught()) {
            Nan::FatalException(try_catch);
        }
    }

    void FileSystem::ListXAttr(fuse_req_t req,
                               fuse_ino_t ino,
                               size_t size_) {

        struct fuse_cmd *op = (struct fuse_cmd *)malloc(sizeof(struct fuse_cmd));
        op->op = _FUSE_OPS_LISTXATTR_;
        op->req = req;
        op->ino = ino;
        op->size = size_;
        if (ck_ring_enqueue_spmc(ck_ring, ck_ring_buffer, (void *) op) == false) {
            printf("ckring was full while trying to enqueue write at inode %d\n", (int) ino);
            return;
        }
        uv_async_send(&uv_async_handle);

    }

    void FileSystem::RemoteListXAttr(fuse_req_t req,
                               fuse_ino_t ino,
                               size_t size_) {
        Nan::HandleScope scope;;
        Fuse* fuse = static_cast<Fuse *>(fuse_req_userdata(req));
        Local<Object> fsobj = Nan::New(fuse->fsobj);

        Local<Value> vlistxattr = fsobj->Get(Nan::New<String>("listxattr").ToLocalChecked());
        Local<Function> listxattr = Local<Function>::Cast(vlistxattr);

        Local<Object> context = RequestContextToObject(fuse_req_ctx(req))->ToObject();
        Local<Number> inode = Nan::New<Number>(ino);
        Local<Number> size = Nan::New<Number>(size_);

        Reply* reply = new Reply();
        reply->request = req;
        Local<Object> replyObj = Nan::NewInstance( Nan::New<Function>(reply->constructor)).ToLocalChecked();//->GetFunction()->NewInstance();
        reply->Wrap(replyObj);

        Local<Value> argv[4] = {context, inode,
                                size, replyObj};
        Nan::TryCatch try_catch;

        listxattr->Call(fsobj, 4, argv);

        if (try_catch.HasCaught()) {
            Nan::FatalException(try_catch);
        }
    }
    void FileSystem::RemoveXAttr(fuse_req_t req,
                                 fuse_ino_t ino,
                                 const char* name_) {
        struct fuse_cmd *op = (struct fuse_cmd *)malloc(sizeof(struct fuse_cmd));
        op->op = _FUSE_OPS_REMOVEXATTR_;
        op->req = req;
        op->ino = ino;
        op->name = name_;
        if (ck_ring_enqueue_spmc(ck_ring, ck_ring_buffer, (void *) op) == false) {
            printf("ckring was full while trying to enqueue write at inode %d\n", (int) ino);
            return;
        }
        uv_async_send(&uv_async_handle);
    }

    void FileSystem::RemoteRemoveXAttr(fuse_req_t req,
                                 fuse_ino_t ino,
                                 const char* name_) {
        Nan::HandleScope scope;;
        Fuse* fuse = static_cast<Fuse *>(fuse_req_userdata(req));
        Local<Object> fsobj = Nan::New(fuse->fsobj);

        Local<Value> vremovexattr = fsobj->Get(Nan::New<String>("removexattr").ToLocalChecked());
        Local<Function> removexattr = Local<Function>::Cast(vremovexattr);

        Local<Object> context = RequestContextToObject(fuse_req_ctx(req))->ToObject();
        Local<Number> inode = Nan::New<Number>(ino);
        Local<String> name = Nan::New<String>(name_).ToLocalChecked();

        Reply* reply = new Reply();
        reply->request = req;
        Local<Object> replyObj = Nan::NewInstance( Nan::New<Function>(reply->constructor)).ToLocalChecked();//->GetFunction()->NewInstance();
        reply->Wrap(replyObj);

        Local<Value> argv[4] = {context, inode,
                                name, replyObj};
        Nan::TryCatch try_catch;

        removexattr->Call(fsobj, 4, argv);

        if (try_catch.HasCaught()) {
            Nan::FatalException(try_catch);
        }
    }
    void FileSystem::Access(fuse_req_t req,
                            fuse_ino_t ino,
                            int mask_) {
        struct fuse_cmd *op = (struct fuse_cmd *)malloc(sizeof(struct fuse_cmd));
        op->op = _FUSE_OPS_ACCESS_;
        op->req = req;
        op->ino = ino;
        op->to_set = mask_;

        if (ck_ring_enqueue_spmc(ck_ring, ck_ring_buffer, (void *) op) == false) {
            printf("ckring was full while trying to access %d\n", (int) ino);
            return;
        }

        uv_async_send(&uv_async_handle);

    }

    void FileSystem::RemoteAccess(fuse_req_t req,
                            fuse_ino_t ino,
                            int mask_) {
        Nan::HandleScope scope;;
        Fuse* fuse = static_cast<Fuse *>(fuse_req_userdata(req));
        Local<Object> fsobj = Nan::New(fuse->fsobj);

        Local<Value> vaccess = fsobj->Get(Nan::New<String>("access").ToLocalChecked());
        Local<Function> access = Local<Function>::Cast(vaccess);

        Local<Object> context = RequestContextToObject(fuse_req_ctx(req))->ToObject();
        Local<Number> inode = Nan::New<Number>(ino);
        Local<Integer> mask = Nan::New<Integer>(mask_);

        Reply* reply = new Reply();
        reply->request = req;
        Local<Object> replyObj = Nan::NewInstance( Nan::New<Function>(reply->constructor)).ToLocalChecked();//->GetFunction()->NewInstance();
        reply->Wrap(replyObj);

        Local<Value> argv[4] = {context, inode,
                                mask, replyObj};

        Nan::TryCatch try_catch;

        access->Call(fsobj, 4, argv);

        if (try_catch.HasCaught()) {
            Nan::FatalException(try_catch);
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
        if(fi != NULL){
            memcpy( (void*) &(op->fi), fi, sizeof(struct fuse_file_info));
        }
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
                            struct fuse_file_info fi) {

        Nan::HandleScope scope;;
        Fuse* fuse = static_cast<Fuse *>(fuse_req_userdata(req));
        Local<Object> fsobj = Nan::New(fuse->fsobj);

        Local<Value> vcreate = fsobj->Get(Nan::New<String>("create").ToLocalChecked());
        Local<Function> create = Local<Function>::Cast(vcreate);

        Local<Object> context = RequestContextToObject(fuse_req_ctx(req))->ToObject();
        Local<Number> parentInode = Nan::New<Number>(parent);
        Local<String> name_ = Nan::New<String>(name).ToLocalChecked();
        Local<Integer> mode_ = Nan::New<Integer>(mode);

        FileInfo* info = new FileInfo();
        info->fi = (struct fuse_file_info*) malloc(sizeof(struct fuse_file_info) );
        memcpy( (void*) info->fi , &fi, sizeof(struct fuse_file_info));
        Local<Object> infoObj = Nan::NewInstance(Nan::New<Function>(info->constructor)).ToLocalChecked();//->GetFunction()->NewInstance();
        info->Wrap(infoObj);

        Reply* reply = new Reply();
        reply->request = req;
        Local<Object> replyObj = Nan::NewInstance( Nan::New<Function>(reply->constructor)).ToLocalChecked();//->GetFunction()->NewInstance();
        reply->Wrap(replyObj);

        Local<Value> argv[6] = {context, parentInode,
                                name_, mode_,
                                infoObj, replyObj};

        Nan::TryCatch try_catch;

        create->Call(fsobj, 6, argv);

        if (try_catch.HasCaught()) {
            Nan::FatalException(try_catch);
        }
        //scope.Close(Undefined());

    }

    void FileSystem::GetLock(fuse_req_t req,
                             fuse_ino_t ino,
                             struct fuse_file_info* fi,
                             struct flock* lock) {
        Nan::HandleScope scope;;
        Fuse* fuse = static_cast<Fuse *>(fuse_req_userdata(req));
        Local<Object> fsobj = Nan::New(fuse->fsobj);

        Local<Value> vgetlk = fsobj->Get(Nan::New<String>("getlk").ToLocalChecked());
        Local<Function> getlk = Local<Function>::Cast(vgetlk);

        Local<Object> context = RequestContextToObject(fuse_req_ctx(req))->ToObject();
        Local<Number> inode = Nan::New<Number>(ino);

        FileInfo* info = new FileInfo();
        info->fi = fi;
        Local<Object> infoObj = Nan::NewInstance(Nan::New<Function>(info->constructor)).ToLocalChecked();//->GetFunction()->NewInstance();
        info->Wrap(infoObj);

        Local<Object> lockObj = FlockToObject(lock)->ToObject();

        Reply* reply = new Reply();
        reply->request = req;
        Local<Object> replyObj = Nan::NewInstance( Nan::New<Function>(reply->constructor)).ToLocalChecked();//->GetFunction()->NewInstance();
        reply->Wrap(replyObj);

        Local<Value> argv[5] = {context, inode,
                                infoObj, lockObj, replyObj};

        Nan::TryCatch try_catch;

        getlk->Call(fsobj, 5, argv);

        if (try_catch.HasCaught()) {
            Nan::FatalException(try_catch);
        }
    }

    void FileSystem::SetLock(fuse_req_t req,
                             fuse_ino_t ino,
                             struct fuse_file_info* fi,
                             struct flock* lock,
                             int sleep_) {
        Nan::HandleScope scope;;
        Fuse* fuse = static_cast<Fuse *>(fuse_req_userdata(req));
        Local<Object> fsobj = Nan::New(fuse->fsobj);

        Local<Value> vsetlk = fsobj->Get(Nan::New<String>("setlk").ToLocalChecked());
        Local<Function> setlk = Local<Function>::Cast(vsetlk);

        Local<Object> context = RequestContextToObject(fuse_req_ctx(req))->ToObject();
        Local<Number> inode = Nan::New<Number>(ino);
        Local<Integer> sleep = Nan::New<Integer>(sleep_);

        FileInfo* info = new FileInfo();
        info->fi = fi;
        Local<Function> constructor = Nan::New<Function>(FileInfo::constructor);
        Local<Object> infoObj = Nan::NewInstance(constructor).ToLocalChecked();//->GetFunction()->NewInstance();
        info->Wrap(infoObj);

        Local<Object> lockObj = FlockToObject(lock)->ToObject();

        Reply* reply = new Reply();
        reply->request = req;
        Local<Object> replyObj = Nan::NewInstance( Nan::New<Function>(reply->constructor)).ToLocalChecked();//->GetFunction()->NewInstance();
        reply->Wrap(replyObj);

        Local<Value> argv[6] = {context, inode,
                                infoObj, lockObj,
                                sleep, replyObj};

        Nan::TryCatch try_catch;

        setlk->Call(fsobj, 6, argv);

        if (try_catch.HasCaught()) {
            Nan::FatalException(try_catch);
        }
    }

    void FileSystem::BMap(fuse_req_t req,
                          fuse_ino_t ino,
                          size_t blocksize_,
                          uint64_t idx) {
        Nan::HandleScope scope;;
        Fuse* fuse = static_cast<Fuse *>(fuse_req_userdata(req));
        Local<Object> fsobj = Nan::New(fuse->fsobj);

        Local<Value> vbmap = fsobj->Get(Nan::New<String>("bmap").ToLocalChecked());
        Local<Function> bmap = Local<Function>::Cast(vbmap);

        Local<Object> context = RequestContextToObject(fuse_req_ctx(req))->ToObject();
        Local<Number> inode = Nan::New<Number>(ino);
        Local<Integer> blocksize = Nan::New<Integer>((int)blocksize_);

        // TODO: Check if down casting to integer breaks BMAP
        Local<Integer> index = Nan::New<Integer>( (int) idx);

        Reply* reply = new Reply();
        reply->request = req;
        Local<Object> replyObj = Nan::NewInstance( Nan::New<Function>(reply->constructor)).ToLocalChecked();//->GetFunction()->NewInstance();
        reply->Wrap(replyObj);

        Local<Value> argv[5] = {context, inode,
                                blocksize, index, replyObj};

        Nan::TryCatch try_catch;

        bmap->Call(fsobj, 5, argv);

        if (try_catch.HasCaught()) {
            Nan::FatalException(try_catch);
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


