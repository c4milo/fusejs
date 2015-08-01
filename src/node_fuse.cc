// Copyright 2012, Camilo Aguilar. Cloudescape, LLC.
#include "bindings.h"
#include "reply.h"
#include "file_info.h"

namespace NodeFuse {
    //stat struct symbols
    static Persistent<String> uid_sym;
    static Persistent<String> gid_sym;
    static Persistent<String> pid_sym;
    static Persistent<String> dev_sym;
    static Persistent<String> mode_sym;
    static Persistent<String> nlink_sym;
    static Persistent<String> rdev_sym;
    static Persistent<String> size_sym;
    static Persistent<String> blksize_sym;
    static Persistent<String> blocks_sym;
    static Persistent<String> atime_sym;
    static Persistent<String> mtime_sym;
    static Persistent<String> ctime_sym;

    //statvfs struct symbols
    static Persistent<String> bsize_sym;
    static Persistent<String> frsize_sym;
    //static Persistent<String> blocks_sym;
    static Persistent<String> bfree_sym;
    static Persistent<String> bavail_sym;
    static Persistent<String> files_sym;
    static Persistent<String> ffree_sym;
    static Persistent<String> favail_sym;
    static Persistent<String> fsid_sym;
    static Persistent<String> flag_sym;
    static Persistent<String> namemax_sym;

    //entry symbols
    static Persistent<String> ino_sym;
    static Persistent<String> generation_sym;
    static Persistent<String> attr_sym;
    static Persistent<String> attr_timeout_sym;
    static Persistent<String> entry_timeout_sym;

    //lock symbols
    static Persistent<String> type_sym;
    static Persistent<String> whence_sym;
    static Persistent<String> start_sym;
    static Persistent<String> len_sym;
    //static Persistent<String> pid_sym;


    void InitializeFuse(Handle<Object> target) {
        NanScope();

        Fuse::Initialize(target);
        FileSystem::Initialize();
        Reply::Initialize();
        FileInfo::Initialize();

        target->Set(NanNew<String>("version"),
                    NanNew<String>(NODE_FUSE_VERSION));

        target->Set(NanNew<String>("fuse_version"),
                    NanNew<Integer>(fuse_version()));
        NanAssignPersistent( uid_sym, NanNew("uid"));
        NanAssignPersistent( gid_sym, NanNew("gid"));
        NanAssignPersistent( pid_sym, NanNew("pid"));
        NanAssignPersistent( dev_sym, NanNew("dev"));
        NanAssignPersistent( mode_sym, NanNew("mode"));
        NanAssignPersistent( nlink_sym, NanNew("nlink"));
        NanAssignPersistent( rdev_sym, NanNew("rdev"));
        NanAssignPersistent( size_sym, NanNew("size"));
        NanAssignPersistent( blksize_sym, NanNew("blksize"));
        NanAssignPersistent( blocks_sym, NanNew("blocks"));
        NanAssignPersistent( atime_sym, NanNew("atime"));
        NanAssignPersistent( mtime_sym, NanNew("mtime"));
        NanAssignPersistent( ctime_sym, NanNew("ctime"));
        NanAssignPersistent( bsize_sym, NanNew("bsize"));
        NanAssignPersistent( frsize_sym, NanNew("frsize"));
        NanAssignPersistent( blocks_sym, NanNew("blocks"));
        NanAssignPersistent( bfree_sym, NanNew("bfree"));
        NanAssignPersistent( bavail_sym, NanNew("bavail"));
        NanAssignPersistent( files_sym, NanNew("files"));
        NanAssignPersistent( ffree_sym, NanNew("ffree"));
        NanAssignPersistent( favail_sym, NanNew("favail"));
        NanAssignPersistent( fsid_sym, NanNew("fsid"));
        NanAssignPersistent( flag_sym, NanNew("flag"));
        NanAssignPersistent( namemax_sym, NanNew("namemax"));
        NanAssignPersistent( ino_sym, NanNew("inode"));
        NanAssignPersistent( generation_sym, NanNew("generation"));
        NanAssignPersistent( attr_sym, NanNew("attr"));
        NanAssignPersistent( attr_timeout_sym, NanNew("attr_timeout"));
        NanAssignPersistent( entry_timeout_sym, NanNew("entry_timeout"));
        NanAssignPersistent( type_sym, NanNew("type"));
        NanAssignPersistent( whence_sym, NanNew("whence"));
        NanAssignPersistent( start_sym, NanNew("start"));
        NanAssignPersistent( len_sym, NanNew("len"));
        NanAssignPersistent( pid_sym, NanNew("pid"));
    }

    int ObjectToFuseEntryParam(Handle<Value> value, struct fuse_entry_param* entry) {
        NanScope();
        int ret = -1;

        memset(entry, 0, sizeof(struct fuse_entry_param));

        Local<Object> obj = value->ToObject();
        entry->ino = obj->Get(NanNew(ino_sym))->IntegerValue();
        entry->generation = obj->Get(NanNew(generation_sym))->IntegerValue();
        entry->attr_timeout = obj->Get(NanNew(attr_timeout_sym))->NumberValue();
        entry->entry_timeout = obj->Get(NanNew(entry_timeout_sym))->NumberValue();

        //struct stat statbuf;
        ret = ObjectToStat(obj->Get(NanNew(attr_sym)), &entry->attr);

        return ret;
    }

    int ObjectToStat(Handle<Value> value, struct stat* statbuf) {
        NanScope();

        memset(statbuf, 0, sizeof(struct stat));

        Local<Object> obj = value->ToObject();
        statbuf->st_dev = obj->Get(NanNew(dev_sym))->IntegerValue();
        statbuf->st_ino = obj->Get(NanNew(ino_sym))->IntegerValue();
        statbuf->st_mode = obj->Get(NanNew(mode_sym))->IntegerValue();
        statbuf->st_nlink = obj->Get(NanNew(nlink_sym))->IntegerValue();
        statbuf->st_uid = obj->Get(NanNew(uid_sym))->IntegerValue();
        statbuf->st_gid = obj->Get(NanNew(gid_sym))->IntegerValue();
        statbuf->st_rdev = obj->Get(NanNew(rdev_sym))->IntegerValue();
        statbuf->st_size = obj->Get(NanNew(size_sym))->NumberValue();
        statbuf->st_blksize = obj->Get(NanNew(blksize_sym))->IntegerValue();
        statbuf->st_blocks = obj->Get(NanNew(blocks_sym))->IntegerValue();
        statbuf->st_atime = NODE_V8_UNIXTIME(obj->Get(NanNew(atime_sym)));
        statbuf->st_mtime = NODE_V8_UNIXTIME(obj->Get(NanNew(mtime_sym)));
        statbuf->st_ctime = NODE_V8_UNIXTIME(obj->Get(NanNew(ctime_sym)));

        return 0;
    }

    int ObjectToStatVfs(Handle<Value> value, struct statvfs* statbuf) {
        NanScope();

        memset(statbuf, 0, sizeof(struct statvfs));

        Local<Object> obj = value->ToObject();

        statbuf->f_bsize = obj->Get(NanNew(bsize_sym))->NumberValue();
        statbuf->f_frsize = obj->Get(NanNew(blocks_sym))->NumberValue();

        statbuf->f_blocks = obj->Get(NanNew(blocks_sym))->IntegerValue();
        statbuf->f_bfree = obj->Get(NanNew(bfree_sym))->IntegerValue();
        statbuf->f_bavail = obj->Get(NanNew(bavail_sym))->IntegerValue();
        statbuf->f_files = obj->Get(NanNew(files_sym))->IntegerValue();
        statbuf->f_ffree = obj->Get(NanNew(ffree_sym))->IntegerValue();
        statbuf->f_favail = obj->Get(NanNew(favail_sym))->NumberValue();

        statbuf->f_fsid = obj->Get(NanNew(fsid_sym))->NumberValue();
        statbuf->f_flag = obj->Get(NanNew(flag_sym))->NumberValue();
        statbuf->f_namemax = obj->Get(NanNew(namemax_sym))->NumberValue();

        return 0;
    }

    int ObjectToFlock(Handle<Value> value, struct flock* lock) {
        NanScope();

        memset(lock, 0, sizeof(struct flock));

        Local<Object> obj = value->ToObject();

        lock->l_type = obj->Get(NanNew(type_sym))->IntegerValue();
        lock->l_whence = obj->Get(NanNew(whence_sym))->IntegerValue();
        lock->l_start = obj->Get(NanNew(start_sym))->IntegerValue();
        lock->l_len = obj->Get(NanNew(len_sym))->IntegerValue();
        lock->l_pid = obj->Get(NanNew(pid_sym))->IntegerValue();

        return 0;
    }

    Handle<Value> GetAttrsToBeSet(int to_set, struct stat* stat) {
        Local<Object> attrs = NanNew<Object>();

        if (to_set & FUSE_SET_ATTR_MODE) {
            attrs->Set( NanNew(mode_sym), NanNew<Integer>(stat->st_mode));
        }

        if (to_set & FUSE_SET_ATTR_UID) {
            attrs->Set( NanNew(uid_sym), NanNew<Integer>(stat->st_uid));
        }

        if (to_set & FUSE_SET_ATTR_GID) {
            attrs->Set( NanNew(gid_sym), NanNew<Integer>(stat->st_gid));
        }

        if (to_set & FUSE_SET_ATTR_SIZE) {
            attrs->Set( NanNew(size_sym), NanNew<Number>(stat->st_size));
        }

        if (to_set & FUSE_SET_ATTR_ATIME) {
            attrs->Set( NanNew(atime_sym), NODE_UNIXTIME_V8(stat->st_atime));
        }

        if (to_set & FUSE_SET_ATTR_MTIME) {
            attrs->Set( NanNew(mtime_sym), NODE_UNIXTIME_V8(stat->st_mtime));
        }

        #ifdef FUSE_SET_ATTR_ATIME_NOW
        if (to_set & FUSE_SET_ATTR_ATIME_NOW ){
            attrs->Set( NanNew(atime_sym), NanNew<Integer>(-1));
        }

        if (to_set & FUSE_SET_ATTR_MTIME_NOW ){
            attrs->Set( NanNew(mtime_sym), NanNew<Integer>(-1));
        }
        #endif



        return attrs;
    }

    Handle<Value> RequestContextToObject(const struct fuse_ctx* ctx) {
        Local<Object> context = NanNew<Object>();

        context->Set(NanNew(uid_sym), NanNew<Integer>(ctx->uid));
        context->Set(NanNew(gid_sym), NanNew<Integer>(ctx->gid));
        context->Set(NanNew(pid_sym), NanNew<Integer>(ctx->pid));

        return context;
    }

    Handle<Value> FlockToObject(const struct flock *lock) {
        Local<Object> rv = NanNew<Object>();

        //Specifies the type of the lock; one of F_RDLCK, F_WRLCK, or F_UNLCK.
        rv->Set( NanNew(type_sym), NanNew<Integer>(lock->l_type)); //TODO convert to object with accessors
        //This corresponds to the whence argument to fseek or lseek, and specifies what the offset is relative to. Its value can be one of SEEK_SET, SEEK_CUR, or SEEK_END.
        rv->Set( NanNew(whence_sym), NanNew<Integer>(lock->l_whence)); //TODO convert to object with accessors
        rv->Set( NanNew(start_sym), NanNew<Integer>( (int) lock->l_start));
        rv->Set( NanNew(len_sym), NanNew<Integer>( (int) lock->l_len));
        rv->Set( NanNew(pid_sym), NanNew<Integer>( (int) lock->l_pid));

        return rv;
    }

    NODE_MODULE(fusejs, InitializeFuse)
} //namespace NodeFuse

