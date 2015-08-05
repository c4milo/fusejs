// Copyright 2012, Camilo Aguilar. Cloudescape, LLC.
#include "bindings.h"
#include "reply.h"
#include "file_info.h"

namespace NodeFuse {
    //stat struct symbols

    void InitializeFuse(Handle<Object> target) {
        Nan::HandleScope scope;;

        Fuse::Initialize(target);
        FileSystem::Initialize(target);
        Reply::Initialize(target);
        FileInfo::Initialize(target);

        Nan::Set(target ,
            Nan::New<String>("version").ToLocalChecked(),
            Nan::New<String>(NODE_FUSE_VERSION).ToLocalChecked());

        Nan::Set(target, Nan::New<String>("fuse_version").ToLocalChecked(),
                    Nan::New<Integer>(fuse_version()));
    }

    int ObjectToFuseEntryParam(Handle<Value> value, struct fuse_entry_param* entry) {
        Nan::HandleScope scope;;
        int ret = -1;

        memset(entry, 0, sizeof(struct fuse_entry_param));

        Local<Object> obj = value->ToObject();
        entry->ino = obj->Get(Nan::New<String>("ino").ToLocalChecked())->IntegerValue();
        entry->generation = obj->Get(Nan::New<String>("generation").ToLocalChecked())->IntegerValue();
        entry->attr_timeout = obj->Get(Nan::New<String>("attr_timeout").ToLocalChecked())->NumberValue();
        entry->entry_timeout = obj->Get(Nan::New<String>("entry_timeout").ToLocalChecked())->NumberValue();

        //struct stat statbuf;
        ret = ObjectToStat(obj->Get(Nan::New<String>("attr").ToLocalChecked()), &entry->attr);

        return ret;
    }

    int ObjectToStat(Handle<Value> value, struct stat* statbuf) {
        Nan::HandleScope scope;;

        memset(statbuf, 0, sizeof(struct stat));

        Local<Object> obj = value->ToObject();
        statbuf->st_dev = obj->Get(Nan::New<String>("dev").ToLocalChecked())->IntegerValue();
        statbuf->st_ino = obj->Get(Nan::New<String>("ino").ToLocalChecked())->IntegerValue();
        statbuf->st_mode = obj->Get(Nan::New<String>("mode").ToLocalChecked())->IntegerValue();
        statbuf->st_nlink = obj->Get(Nan::New<String>("nlink").ToLocalChecked())->IntegerValue();
        statbuf->st_uid = obj->Get(Nan::New<String>("uid").ToLocalChecked())->IntegerValue();
        statbuf->st_gid = obj->Get(Nan::New<String>("gid").ToLocalChecked())->IntegerValue();
        statbuf->st_rdev = obj->Get(Nan::New<String>("rdev").ToLocalChecked())->IntegerValue();
        statbuf->st_size = obj->Get(Nan::New<String>("size").ToLocalChecked())->NumberValue();
        statbuf->st_blksize = obj->Get(Nan::New<String>("blksize").ToLocalChecked())->IntegerValue();
        statbuf->st_blocks = obj->Get(Nan::New<String>("blocks").ToLocalChecked())->IntegerValue();
        statbuf->st_atime = NODE_V8_UNIXTIME(obj->Get(Nan::New<String>("atime").ToLocalChecked()));
        statbuf->st_mtime = NODE_V8_UNIXTIME(obj->Get(Nan::New<String>("mtime").ToLocalChecked()));
        statbuf->st_ctime = NODE_V8_UNIXTIME(obj->Get(Nan::New<String>("ctime").ToLocalChecked()));

        return 0;
    }

    int ObjectToStatVfs(Handle<Value> value, struct statvfs* statbuf) {
        Nan::HandleScope scope;;

        memset(statbuf, 0, sizeof(struct statvfs));

        Local<Object> obj = value->ToObject();

        statbuf->f_bsize = obj->Get(Nan::New<String>("bsize").ToLocalChecked())->NumberValue();
        statbuf->f_frsize = obj->Get(Nan::New<String>("blocks").ToLocalChecked())->NumberValue();

        statbuf->f_blocks = obj->Get(Nan::New<String>("blocks").ToLocalChecked())->IntegerValue();
        statbuf->f_bfree = obj->Get(Nan::New<String>("bfree").ToLocalChecked())->IntegerValue();
        statbuf->f_bavail = obj->Get(Nan::New<String>("bavail").ToLocalChecked())->IntegerValue();
        statbuf->f_files = obj->Get(Nan::New<String>("files").ToLocalChecked())->IntegerValue();
        statbuf->f_ffree = obj->Get(Nan::New<String>("ffree").ToLocalChecked())->IntegerValue();
        statbuf->f_favail = obj->Get(Nan::New<String>("favail").ToLocalChecked())->NumberValue();

        statbuf->f_fsid = obj->Get(Nan::New<String>("fsid").ToLocalChecked())->NumberValue();
        statbuf->f_flag = obj->Get(Nan::New<String>("flag").ToLocalChecked())->NumberValue();
        statbuf->f_namemax = obj->Get(Nan::New<String>("namemax").ToLocalChecked())->NumberValue();

        return 0;
    }

    int ObjectToFlock(Handle<Value> value, struct flock* lock) {
        Nan::HandleScope scope;;

        memset(lock, 0, sizeof(struct flock));

        Local<Object> obj = value->ToObject();

        lock->l_type = obj->Get(Nan::New<String>("type").ToLocalChecked())->IntegerValue();
        lock->l_whence = obj->Get(Nan::New<String>("whence").ToLocalChecked())->IntegerValue();
        lock->l_start = obj->Get(Nan::New<String>("start").ToLocalChecked())->IntegerValue();
        lock->l_len = obj->Get(Nan::New<String>("len").ToLocalChecked())->IntegerValue();
        lock->l_pid = obj->Get(Nan::New<String>("pid").ToLocalChecked())->IntegerValue();

        return 0;
    }

    Handle<Value> GetAttrsToBeSet(int to_set, struct stat* stat) {
        Local<Object> attrs = Nan::New<Object>();

        if (to_set & FUSE_SET_ATTR_MODE) {
            attrs->Set( Nan::New<String>("mode").ToLocalChecked(), Nan::New<Integer>(stat->st_mode));
        }

        if (to_set & FUSE_SET_ATTR_UID) {
            attrs->Set( Nan::New<String>("uid").ToLocalChecked(), Nan::New<Integer>(stat->st_uid));
        }

        if (to_set & FUSE_SET_ATTR_GID) {
            attrs->Set( Nan::New<String>("gid").ToLocalChecked(), Nan::New<Integer>(stat->st_gid));
        }

        if (to_set & FUSE_SET_ATTR_SIZE) {
            attrs->Set( Nan::New<String>("size").ToLocalChecked(), Nan::New<Number>(stat->st_size));
        }

        if (to_set & FUSE_SET_ATTR_ATIME) {
            attrs->Set( Nan::New<String>("atime").ToLocalChecked(), NODE_UNIXTIME_V8(stat->st_atime));
        }

        if (to_set & FUSE_SET_ATTR_MTIME) {
            attrs->Set( Nan::New<String>("mtime").ToLocalChecked(), NODE_UNIXTIME_V8(stat->st_mtime));
        }

        #ifdef FUSE_SET_ATTR_ATIME_NOW
        if (to_set & FUSE_SET_ATTR_ATIME_NOW ){
            attrs->Set( Nan::New<String>("atime").ToLocalChecked(), Nan::New<Integer>(-1));
        }

        if (to_set & FUSE_SET_ATTR_MTIME_NOW ){
            attrs->Set( Nan::New<String>("mtime").ToLocalChecked(), Nan::New<Integer>(-1));
        }
        #endif



        return attrs;
    }

    Handle<Value> RequestContextToObject(const struct fuse_ctx* ctx) {
        Local<Object> context = Nan::New<Object>();

        context->Set( Nan::New<String>("uid").ToLocalChecked(), Nan::New<Integer>(ctx->uid));
        context->Set( Nan::New<String>("gid").ToLocalChecked(), Nan::New<Integer>(ctx->gid));
        context->Set( Nan::New<String>("pid").ToLocalChecked(), Nan::New<Integer>(ctx->pid));

        return context;
    }

    Handle<Value> FlockToObject(const struct flock *lock) {
        Local<Object> rv = Nan::New<Object>();

        //Specifies the type of the lock; one of F_RDLCK, F_WRLCK, or F_UNLCK.
        rv->Set( Nan::New<String>("type").ToLocalChecked(), Nan::New<Integer>(lock->l_type)); //TODO convert to object with accessors
        //This corresponds to the whence argument to fseek or lseek, and specifies what the offset is relative to. Its value can be one of SEEK_SET, SEEK_CUR, or SEEK_END.
        rv->Set( Nan::New<String>("whence").ToLocalChecked(), Nan::New<Integer>(lock->l_whence)); //TODO convert to object with accessors
        rv->Set( Nan::New<String>("start").ToLocalChecked(), Nan::New<Integer>( (int) lock->l_start));
        rv->Set( Nan::New<String>("len").ToLocalChecked(), Nan::New<Integer>( (int) lock->l_len));
        rv->Set( Nan::New<String>("pid").ToLocalChecked(), Nan::New<Integer>( (int) lock->l_pid));

        return rv;
    }

    NODE_MODULE(fusejs, InitializeFuse)
} //namespace NodeFuse

