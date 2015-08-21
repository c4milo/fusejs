/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "filesystem.h"
#include "reply.h"
#include "file_info.h"
#include "bindings.h"
#include "node_buffer.h"
#include "mpsc_queue.h"

namespace NodeFuse {
    Persistent<Function> FileSystem::constructor;

    // struct vrt_fuse_cmd_value
    // {
    //   struct vrt_value parent;
    //   // struct fuse_cmd value;
    //   int8_t op;
    //   fuse_req_t req;
    //   fuse_ino_t ino;
    //   fuse_ino_t newino; //used for renaming files
    //   size_t size;
    //   off_t off;
    //   dev_t dev;
    //   mode_t mode;
    //   const char* name;
    //   const char* newname; //used for renaming files, and for symlinking
    //   int to_set;
    //   struct fuse_conn_info conn;
    //   struct fuse_file_info fi;
    //   struct stat attr;
    //   void *userdata;
    //   unsigned long nlookup;
    //   #ifdef __APPLE__
    //   uint32_t position;
    //   #endif 
    // };


    // static struct vrt_value *
    // vrt_fuse_cmd__new_value(struct vrt_value_type *type)
    // {
    //     struct vrt_fuse_cmd_value  *self = (struct vrt_fuse_cmd_value *) cork_new(struct vrt_fuse_cmd_value);
    //     return &self->parent;
    // }

    // static void
    // vrt_fuse_cmd__free_value(struct vrt_value_type *type, struct vrt_value *value)
    // {
    //     struct vrt_fuse_cmd_value  *self =
    //         cork_container_of(value, struct vrt_fuse_cmd_value, parent);
    //     free(self);
    // }

    // /* The following hash value is produced by the cork-hash utility function */
    // // #define VRT_FUSE_CMD_TYPE 0x0f0ea3c9

    // static struct vrt_value_type  _vrt_fuse_cmd_type  = {
    //     vrt_fuse_cmd__new_value,
    //     vrt_fuse_cmd__free_value
    // };

    // static struct vrt_value_type *
    // vrt_fuse_cmd_type(void)
    // {
    //     return &_vrt_fuse_cmd_type;
    // }




    // static struct vrt_queue *ring_buffer; // =  vrt_queue_new("queue", vrt_fuse_cmd_type, __RING_SIZE__);
    // static struct vrt_producer **producers;// = (struct vrt_producer *) malloc(sizeof(struct vrt_producer *) * _NUMBER_OF_FUSE_OPERATIONS_)
    // static struct vrt_consumer *consumer;
    static struct fuse_lowlevel_ops fuse_ops = {};

    static mpsc_queue_t<struct fuse_cmd> ring_buffer(__RING_SIZE__);
    
    void FileSystem::DispatchOp(uv_async_t* handle, int status)
    {
    //     struct vrt_fuse_cmd_value op; //(struct fuse_cmd *)malloc(sizeof(struct fuse_cmd));

    //     struct vrt_value *vvalue;
    //     struct vrt_fuse_cmd_value *value;
        struct fuse_cmd *value;
        int result;
        while (   (result = ring_buffer.consume(&value)) != MPSC_QUEUE_EOF ){

            switch(value->op){
                case _FUSE_OPS_LOOKUP_:
                    RemoteLookup(value->req, value->ino, value->name);
                    break;
                case _FUSE_OPS_GETATTR_:
                    RemoteGetAttr(value->req, value->ino, value->fi);
                    break;
                case _FUSE_OPS_OPEN_:
                    RemoteOpen(value->req, value->ino, value->fi);
                    break;
                case _FUSE_OPS_READ_:
                    RemoteRead(value->req, value->ino, value->size, value->off, value->fi);
                    break;
                case _FUSE_OPS_READDIR_:
                    RemoteReadDir(value->req, value->ino, value->size, value->off, value->fi);
                    break;
                case _FUSE_OPS_INIT_:
                    RemoteInit(value->userdata, value->conn);
                    break;
                case _FUSE_OPS_DESTROY_:
                    RemoteDestroy(value->userdata);
                    break;
                case _FUSE_OPS_FORGET_:
                    RemoteForget(value->req, value->ino, value->nlookup);
                    break;
                case _FUSE_OPS_SETATTR_:
                    RemoteSetAttr(value->req, value->ino, value->attr, value->to_set, value->fi);
                    break;
                case _FUSE_OPS_READLINK_:
                    RemoteReadLink(value->req, value->ino);
                    break;
                case _FUSE_OPS_MKNOD_:
                    RemoteMkNod(value->req, value->ino, value->name, value->mode, value->dev);
                    break;
                case _FUSE_OPS_MKDIR_:
                    RemoteMkDir(value->req, value->ino, value->name,value->mode);
                    break;
                case _FUSE_OPS_UNLINK_:
                    RemoteUnlink(value->req, value->ino, value->name);
                    break;
                case _FUSE_OPS_RMDIR_:
                    RemoteRmDir(value->req, value->ino, value->name);
                    break;
                case _FUSE_OPS_SYMLINK_:
                    RemoteSymLink(value->req, value->name, value->ino, value->newname);
                    break;
                case _FUSE_OPS_RENAME_:
                    RemoteRename(value->req, value->ino, value->name, value->newino, value->newname);
                    break;
                case _FUSE_OPS_LINK_:
                    RemoteLink(value->req, value->ino, value->newino, value->name);
                    break;
                case _FUSE_OPS_WRITE_:
                    RemoteWrite(value->req, value->ino, value->name, value->size, value->off, value->fi);
                    break;
                case _FUSE_OPS_FLUSH_:
                    RemoteFlush(value->req, value->ino, value->fi);
                    break;
                case _FUSE_OPS_RELEASE_:
                    RemoteRelease(value->req, value->ino, value->fi);
                    break;
                case _FUSE_OPS_FSYNC_:
                    RemoteFSync(value->req, value->ino, value->to_set, value->fi);
                    break;
                case _FUSE_OPS_OPENDIR_:
                    RemoteOpenDir(value->req, value->ino, value->fi);
                    break;
                case _FUSE_OPS_RELEASEDIR_:
                    RemoteReleaseDir(value->req, value->ino, value->fi);
                    break;
                case _FUSE_OPS_FSYNCDIR_:
                    RemoteFSyncDir(value->req, value->ino, value->to_set, value->fi);
                    break;
                case _FUSE_OPS_STATFS_:
                    RemoteStatFs(value->req, value->ino);
                    break;
                case _FUSE_OPS_SETXATTR_:
                    RemoteSetXAttr(value->req, value->ino,value->name, value->newname, value->size, value->to_set
                            #ifdef __APPLE__
                                              ,value->position
                            #endif
                        );

                    break;
                case _FUSE_OPS_GETXATTR_:
                    RemoteGetXAttr(value->req, value->ino, value->name, value->size
                            #ifdef __APPLE__
                                              ,value->position
                            #endif
                        );

                    break;
                case _FUSE_OPS_LISTXATTR_:
                    RemoteListXAttr(value->req, value->ino, value->size);
                    break;
                case _FUSE_OPS_REMOVEXATTR_:
                    RemoteRemoveXAttr(value->req, value->ino, value->name);
                    break;
                case _FUSE_OPS_ACCESS_:
                    RemoteAccess(value->req, value->ino, value->to_set);
                    break;
                case _FUSE_OPS_CREATE_:
                    RemoteCreate(value->req, value->ino, value->name, value->mode, value->fi);
                    break;
                case _FUSE_OPS_GETLK_:
                    break;
                case _FUSE_OPS_SETLK_:
                    break;
                case _FUSE_OPS_BMAP_:                
                    break;
            }
        }
        
    }


    void FileSystem::Initialize(Handle<Object> target) {

        // ring_buffer =  vrt_queue_new("queue", vrt_fuse_cmd_type(), __RING_SIZE__);
        
        // producers = (struct vrt_producer **) malloc(sizeof(struct vrt_producer *) * 1);
        // int i;
        // for( i = 0; i < 1; ++i){
        //     producers[i] = vrt_producer_new("producer",1, ring_buffer);
        //     producers[i]->yield = vrt_yield_strategy_hybrid();
        // }
        // consumer = vrt_consumer_new("consumer",  ring_buffer);
        // consumer->yield =  vrt_yield_strategy_hybrid();


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

        struct fuse_cmd *value;

        if( ring_buffer.producer_claim_next(&value) != 0 ){
            printf("ring buffer was full while trying to enqueue init\n");
            return;
        }
        value->op = _FUSE_OPS_INIT_;
        value->userdata =  userdata;
        if(conn != NULL){
            memcpy( &(value->conn), conn, sizeof(struct fuse_conn_info));
        }
        ring_buffer.producer_publish();//(producers[ 0/*_FUSE_OPS_INIT_*/]);
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

        struct fuse_cmd *value;

        if( ring_buffer.producer_claim_next(&value) != 0){
            printf("ring buffer was full while trying to enqueue destroy");
            return;
        }

        value->op = _FUSE_OPS_DESTROY_;
        value->userdata =  userdata;        

        ring_buffer.producer_publish();//(producers[  0/*_FUSE_OPS_DESTROY_*/]);
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
        struct fuse_cmd *value;

        if( ring_buffer.producer_claim_next(&value) != 0){
            printf("ring buffer was full while trying to enqueue lookup at inode %d - with child %s\n", (int) parent,name);
            return;
        }


        value->op = _FUSE_OPS_LOOKUP_;
        value->req = req;
        value->ino = parent;
        value->name = name;

        ring_buffer.producer_publish();//(producers[  0/*_FUSE_OPS_LOOKUP_*/]);
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
        struct fuse_cmd *value;

        if( ring_buffer.producer_claim_next(&value) != 0){
            printf("ring buffer was full while trying to enqueue forget at inode %d\n", (int)ino);
            return;
        }

        value->op = _FUSE_OPS_FORGET_;
        value->req = req;
        value->ino = ino;
        value->nlookup = nlookup;

        ring_buffer.producer_publish();//(producers[  0/*_FUSE_OPS_FORGET_*/]);
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
        struct fuse_cmd *value;

        if( ring_buffer.producer_claim_next(&value) != 0){
            printf("ring buffer was full while trying to enqueue getattr at inode %d\n", (int) ino);
            return;
        }

        value->op = _FUSE_OPS_GETATTR_;
        value->req = req;
        value->ino = ino;
        // value->fi = fuse_file_info;
        if(fi != NULL){
            memcpy( (void *) &(value->fi),  fi, sizeof(struct fuse_file_info));
        }

        ring_buffer.producer_publish();//(producers[  0/*_FUSE_OPS_GETATTR_*/ ]);
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

        struct fuse_cmd *value;

        if( ring_buffer.producer_claim_next(&value) != 0){
            printf("ring buffer was full while trying to enqueue setattr at inode %d\n", (int) ino);
            return;
        }


        value->op = _FUSE_OPS_SETATTR_;
        value->req = req;
        value->ino = ino;
        memcpy( (void*) &(value->attr), attr, sizeof(struct stat) );
        value->to_set = to_set;
        if(fi != NULL){
            memcpy( (void*) &(value->fi), fi, sizeof(struct fuse_file_info));
        }


        ring_buffer.producer_publish();//(producers[  0/*_FUSE_OPS_SETATTR_*/]);
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
        struct fuse_cmd *value;

        if( ring_buffer.producer_claim_next(&value) != 0){
            printf("ring buffer was full while trying to enqueue setattr at inode %d\n", (int) ino);
            return;
        }

        value->op = _FUSE_OPS_READLINK_;
        value->req = req;
        value->ino = ino;

        ring_buffer.producer_publish();//(producers[ 0/*_FUSE_OPS_READLINK_*/ ]);
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
        struct fuse_cmd *value;

        if( ring_buffer.producer_claim_next(&value) != 0){
            printf("ring buffer was full while trying to enqueue mknod with parent %d and child %s\n", (int) parent, name);
            return;
        }


        value->op = _FUSE_OPS_MKNOD_;
        value->req = req;
        value->ino= parent;
        value->name = name;
        value->mode = mode;
        value->dev = rdev;

        ring_buffer.producer_publish();//(producers[ 0/*_FUSE_OPS_MKNOD_*/ ]);
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

        struct fuse_cmd *value;

        if( ring_buffer.producer_claim_next(&value) != 0){
            printf("ring buffer was full while trying to enqueue mkdir with parent %d and name %s\n", (int) parent, name);
            return;
        }

        value->op = _FUSE_OPS_MKDIR_;
        value->req = req;
        value->ino = parent;
        value->name = name;
        value->mode = mode;

        ring_buffer.producer_publish();//(producers[  0/*_FUSE_OPS_MKDIR_*/ ]);
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

        struct fuse_cmd *value;

        if( ring_buffer.producer_claim_next(&value) != 0){
            printf("ring buffer was full while trying to unlink %s from parent %d \n",  name, (int) parent);
            return;
        }

        value->op = _FUSE_OPS_UNLINK_;
        value->req = req;
        value->ino = parent;
        value->name = name;

        ring_buffer.producer_publish();//(producers[  0/*_FUSE_OPS_UNLINK_*/]);
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
        struct fuse_cmd *value;

        if( ring_buffer.producer_claim_next(&value) != 0){
            printf("ring buffer was full while trying to unlink folder %s from parent %d \n",  name, (int) parent);
            return;
        }


        value->op = _FUSE_OPS_RMDIR_;
        value->req = req;
        value->ino = parent;
        value->name = name;

        ring_buffer.producer_publish();//(producers[ 0/*_FUSE_OPS_RMDIR_*/ ]);
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
        struct fuse_cmd *value;

        if( ring_buffer.producer_claim_next(&value) != 0){
            printf("ring buffer was full while trying to symlink from %s to %s with parent inode %d \n",  name, link, (int) parent);
            return;
        }

        value->op = _FUSE_OPS_SYMLINK_;
        value->name = link;
        value->ino = parent;
        value->newname = name;


        ring_buffer.producer_publish();//(producers[ 0/*_FUSE_OPS_SYMLINK_*/ ]);
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
        struct fuse_cmd *value;

        if( ring_buffer.producer_claim_next(&value) != 0){
            printf("ring buffer was full while trying to rename file %s from parent %d \n",  name, (int) parent);
            return;
        }


        value->op = _FUSE_OPS_RENAME_;
        value->req = req;
        value->ino = parent;
        value->newino = newparent;
        value->name = name;
        value->newname = newname;

        ring_buffer.producer_publish();//(producers[ 0/*_FUSE_OPS_RENAME_*/ ]);
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
        struct fuse_cmd *value;

        if( ring_buffer.producer_claim_next(&value) != 0){
            printf("ring buffer was full while trying to link inode %d to new parent parent %d with newname %s\n",  (int) ino, (int) newparent, newname);
            return;
        }


        value->op = _FUSE_OPS_LINK_;
        value->req = req;
        value->ino = ino;
        value->newino = newparent;
        value->name = newname;

        ring_buffer.producer_publish();//(producers[ 0/*_FUSE_OPS_LINK_*/ ]);
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
        struct fuse_cmd *value;

        if( ring_buffer.producer_claim_next(&value) != 0){
            printf("ring buffer was full while trying to enqueue open at inode %d\n", (int) ino);
            return;
        }


        value->op = _FUSE_OPS_OPEN_;
        value->req = req;
        value->ino = ino;
        if(fi != NULL){
            memcpy( (void*) &(value->fi), fi, sizeof(struct fuse_file_info));
        }

        ring_buffer.producer_publish();//(producers[  0/*_FUSE_OPS_OPEN_*/]);
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
        struct fuse_cmd *value;

        if( ring_buffer.producer_claim_next(&value) != 0){
            printf("ring buffer was full while trying to enqueue read at inode %d\n", (int) ino);
            return;
        }


        value->op = _FUSE_OPS_READ_;
        value->req = req;
        value->ino = ino;
        value->off = off;
        value->size = size;
        if(fi != NULL){
            memcpy( (void*) &(value->fi), fi, sizeof(struct fuse_file_info));
        }

        ring_buffer.producer_publish();//(producers[ 0/*_FUSE_OPS_READ_*/ ]);
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
        struct fuse_cmd *value;

        if( ring_buffer.producer_claim_next(&value) != 0){
            printf("ring buffer was full while trying to enqueue write at inode %d\n", (int) ino);
            return;
        }

        

        value->op = _FUSE_OPS_WRITE_;
        value->req = req;
        value->ino = ino;
        value->off = off;
        value->size = size;
        value->name = (char *)malloc(size);
        memcpy((void *)value->name, buf, size);

        if(fi != NULL){
            memcpy( (void*) &(value->fi), fi, sizeof(struct fuse_file_info));
        }

        ring_buffer.producer_publish();//(producers[  0/*_FUSE_OPS_WRITE_*/]);
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
        struct fuse_cmd *value;

        if( ring_buffer.producer_claim_next(&value) != 0){
            printf("ring buffer was full while trying to enqueue write at inode %d\n", (int) ino);
            return;
        }

        value->op = _FUSE_OPS_FLUSH_;
        value->req = req;
        value->ino = ino;
        if(fi != NULL){
            memcpy( (void*) &(value->fi), fi, sizeof(struct fuse_file_info));
        }

        ring_buffer.producer_publish();//(producers[  0/*_FUSE_OPS_FLUSH_*/]);
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

        struct fuse_cmd *value;

        if( ring_buffer.producer_claim_next(&value) != 0){
            printf("ring buffer was full while trying to release inode %d\n", (int) ino);
            return;
        }

        
        value->op = _FUSE_OPS_RELEASE_;
        value->req = req;
        value->ino = ino;
        if(fi != NULL){
            memcpy( (void*) &(value->fi), fi, sizeof(struct fuse_file_info));
        }

        ring_buffer.producer_publish();//(producers[  0/*_FUSE_OPS_RELEASE_*/]);
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

        struct fuse_cmd *value;

        if( ring_buffer.producer_claim_next(&value) != 0){
            printf("ring buffer was full while trying to fsync file %d\n",  (int) ino);
            return;
        }

        value->op = _FUSE_OPS_FSYNC_;
        value->req = req;
        value->ino = ino;
        value->to_set = datasync_;
        if(fi != NULL){
            memcpy( (void*) &(value->fi), fi, sizeof(struct fuse_file_info));
        }

        ring_buffer.producer_publish();//(producers[ 0/*_FUSE_OPS_FSYNC_*/]);
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

        struct fuse_cmd *value;

        if( ring_buffer.producer_claim_next(&value) != 0){
            printf("ring buffer was full while trying to opendir with inode %d \n",  (int)ino);
            return;
        }

        value->op = _FUSE_OPS_OPENDIR_;
        value->req = req;
        value->ino = ino;
        if(fi != NULL){
            memcpy( (void*) &(value->fi), fi, sizeof(struct fuse_file_info));
        }

        ring_buffer.producer_publish();//(producers[ 0/*_FUSE_OPS_OPENDIR_*/ ]);
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

        struct fuse_cmd *value;

        if( ring_buffer.producer_claim_next(&value) != 0){
            printf("ring buffer was full while trying to enqueue readdir at inode %d\n", (uint8_t) ino);
            return;
        }


        value->op = _FUSE_OPS_READDIR_;
        value->req = req;
        value->ino = ino;
        value->size = size_;
        value->off = off;
        if(fi != NULL){
            memcpy( (void*) &(value->fi), fi, sizeof(struct fuse_file_info));
        }

        ring_buffer.producer_publish();//(producers[  0/*_FUSE_OPS_READDIR_*/]);
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

        struct fuse_cmd *value;

        if( ring_buffer.producer_claim_next(&value) != 0){
            printf("ring buffer was full while trying to releasedir with inode %d \n",  (int)ino);
            return;
        }

        

        value->op = _FUSE_OPS_RELEASEDIR_;
        value->req = req;
        value->ino = ino;
        if(fi != NULL){
            memcpy( (void*) &(value->fi), fi, sizeof(struct fuse_file_info));
        }


        ring_buffer.producer_publish();//(producers[  0/*_FUSE_OPS_RELEASEDIR_*/]);
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

        struct fuse_cmd *value;

        if( ring_buffer.producer_claim_next(&value) != 0){
            printf("ring buffer was full while trying to fsync dir %d\n",  (int) ino);
            return;
        }

        
        value->op = _FUSE_OPS_FSYNCDIR_;
        value->req = req;
        value->ino = ino;
        value->to_set = datasync_;
        if(fi != NULL){
            memcpy( (void*) &(value->fi), fi, sizeof(struct fuse_file_info));
        }

        ring_buffer.producer_publish();//(producers[ 0/*_FUSE_OPS_FSYNCDIR_*/ ]);
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
        struct fuse_cmd *value;

        if( ring_buffer.producer_claim_next(&value) != 0){
            printf("ring buffer was full while trying to statfs inode %d\n", (int) ino);
            return;
        }

        value->op = _FUSE_OPS_STATFS_;
        value->req = req;
        value->ino = ino;

        ring_buffer.producer_publish();//(producers[  0/*_FUSE_OPS_STATFS_*/]);
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

        struct fuse_cmd *value;

        if( ring_buffer.producer_claim_next(&value) != 0){
            printf("ring buffer was full while trying to setxattr file %s from parent %d \n",  name_, (int) ino);
            return;
        }



        value->op = _FUSE_OPS_SETXATTR_;
        value->req = req;
        value->name = name_;
        value->newname = value_;
        value->size = size_;
        value->to_set = flags_;
        #ifdef __APPLE__
        value->position = position_;
        #endif

        ring_buffer.producer_publish();//(producers[ 0/*_FUSE_OPS_SETXATTR_*/]);
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
    struct fuse_cmd *value;

    if( ring_buffer.producer_claim_next(&value) != 0){
    printf("ring buffer was full while trying to getxattr for inode %d\n",  (int) ino);
        return;
    }

    value->op = _FUSE_OPS_GETXATTR_;
    value->req = req;
    value->ino = ino;
    value->name = name_;
    value->size = size_;
    #ifdef __APPLE__
    value->position = position_;
    #endif


    ring_buffer.producer_publish();//(producers[  0/*_FUSE_OPS_GETXATTR_*/]);
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

        Nan::HandleScope scope;
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

        struct fuse_cmd *value;

        if( ring_buffer.producer_claim_next(&value) != 0){
            printf("ring buffer was full while trying to enqueue write at inode %d\n", (int) ino);
            return;
        }


        value->op = _FUSE_OPS_LISTXATTR_;
        value->req = req;
        value->ino = ino;
        value->size = size_;
        ring_buffer.producer_publish();//(producers[  0/*_FUSE_OPS_LISTXATTR_*/]);
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
        struct fuse_cmd *value;

        if( ring_buffer.producer_claim_next(&value) != 0){
            printf("ring buffer was full while trying to enqueue write at inode %d\n", (int) ino);
            return;
        }

        value->op = _FUSE_OPS_REMOVEXATTR_;
        value->req = req;
        value->ino = ino;
        value->name = name_;
        ring_buffer.producer_publish();//(producers[  0/*_FUSE_OPS_REMOVEXATTR_*/]);
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

        struct fuse_cmd *value;

        if( ring_buffer.producer_claim_next(&value) != 0){
            printf("ring buffer was full while trying to access %d\n", (int) ino);
            return;
        }


        value->op = _FUSE_OPS_ACCESS_;
        value->req = req;
        value->ino = ino;
        value->to_set = mask_;

        ring_buffer.producer_publish();//(producers[  0/*_FUSE_OPS_ACCESS_*/]);
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
        struct fuse_cmd *value;

        if( ring_buffer.producer_claim_next(&value) != 0){
            printf("ring buffer was full while trying to enqueue write at inode %d\n", (int) parent);
            return;
        }

        value->op = _FUSE_OPS_CREATE_;
        value->req = req;
        value->ino = parent;
        value->name = name;
        value->mode = mode;
        if(fi != NULL){
            memcpy( (void*) &(value->fi), fi, sizeof(struct fuse_file_info));
        }

        ring_buffer.producer_publish();//(producers[ 0/*_FUSE_OPS_CREATE_*/ ]);
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


