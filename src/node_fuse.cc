// Copyright 2012, Camilo Aguilar. Cloudescape, LLC.
#include "bindings.h"
#include "reply.h"

namespace NodeFuse {
    //stat struct symbols
    static Persistent<String> uid_sym       = NODE_PSYMBOL("uid");
    static Persistent<String> gid_sym       = NODE_PSYMBOL("gid");
    static Persistent<String> pid_sym       = NODE_PSYMBOL("pid");
    static Persistent<String> dev_sym       = NODE_PSYMBOL("dev");
    static Persistent<String> mode_sym      = NODE_PSYMBOL("mode");
    static Persistent<String> nlink_sym     = NODE_PSYMBOL("nlink");
    static Persistent<String> rdev_sym      = NODE_PSYMBOL("rdev");
    static Persistent<String> size_sym      = NODE_PSYMBOL("size");
    static Persistent<String> blksize_sym   = NODE_PSYMBOL("blksize");
    static Persistent<String> blocks_sym    = NODE_PSYMBOL("blocks");
    static Persistent<String> atime_sym     = NODE_PSYMBOL("atime");
    static Persistent<String> mtime_sym     = NODE_PSYMBOL("mtime");
    static Persistent<String> ctime_sym     = NODE_PSYMBOL("ctime");

    //entry symbols
    static Persistent<String> ino_sym           = NODE_PSYMBOL("inode");
    static Persistent<String> generation_sym    = NODE_PSYMBOL("generation");
    static Persistent<String> attr_sym          = NODE_PSYMBOL("attr");
    static Persistent<String> attr_timeout_sym  = NODE_PSYMBOL("attr_timeout");
    static Persistent<String> entry_timeout_sym = NODE_PSYMBOL("entry_timeout");

    //file info symbols
    static Persistent<String> flags_sym         = NODE_PSYMBOL("flags");
    static Persistent<String> writepage_sym     = NODE_PSYMBOL("writepage");
    static Persistent<String> direct_io_sym     = NODE_PSYMBOL("direct_io");
    static Persistent<String> keep_cache_sym    = NODE_PSYMBOL("keep_cache");
    static Persistent<String> flush_sym         = NODE_PSYMBOL("flush");
    static Persistent<String> nonseekable_sym   = NODE_PSYMBOL("nonseekable");
    static Persistent<String> file_handle_sym   = NODE_PSYMBOL("fh");
    static Persistent<String> lock_owner_sym    = NODE_PSYMBOL("lock_owner");

    void InitializeFuse(Handle<Object> target) {
        HandleScope scope;

        Fuse::Initialize(target);
        Reply::Initialize();

        target->Set(String::NewSymbol("version"),
                    String::New(NODE_FUSE_VERSION));

        target->Set(String::NewSymbol("fuse_version"),
                    Integer::New(fuse_version()));
    }

    int ObjectToFuseEntryParam(Handle<Value> value, struct fuse_entry_param* entry) {
        HandleScope scope;
        int ret = -1;

        memset(entry, 0, sizeof(entry));

        Local<Object> obj = value->ToObject();
        entry->ino = obj->Get(ino_sym)->IntegerValue();
        entry->generation = obj->Get(generation_sym)->IntegerValue();
        entry->attr_timeout = obj->Get(attr_timeout_sym)->NumberValue();
        entry->entry_timeout = obj->Get(entry_timeout_sym)->NumberValue();

        struct stat statbuf;
        ret = ObjectToStat(obj->Get(attr_sym), &entry->attr);

        return ret;
    }

    int ObjectToStat(Handle<Value> value, struct stat* statbuf) {
        HandleScope scope;

        memset(statbuf, 0, sizeof(statbuf));

        Local<Object> obj = value->ToObject();

        statbuf->st_dev = obj->Get(dev_sym)->IntegerValue();
        statbuf->st_ino = obj->Get(ino_sym)->IntegerValue();
        statbuf->st_mode = obj->Get(mode_sym)->IntegerValue();
        statbuf->st_nlink = obj->Get(nlink_sym)->IntegerValue();
        statbuf->st_uid = obj->Get(uid_sym)->IntegerValue();
        statbuf->st_gid = obj->Get(gid_sym)->IntegerValue();
        statbuf->st_rdev = obj->Get(rdev_sym)->IntegerValue();
        statbuf->st_size = obj->Get(size_sym)->NumberValue();
        statbuf->st_blksize = obj->Get(blksize_sym)->IntegerValue();
        statbuf->st_blocks = obj->Get(blocks_sym)->IntegerValue();
        statbuf->st_atime = NODE_V8_UNIXTIME(obj->Get(atime_sym));
        statbuf->st_mtime = NODE_V8_UNIXTIME(obj->Get(mtime_sym));
        statbuf->st_ctime = NODE_V8_UNIXTIME(obj->Get(ctime_sym));

        return 0;
    }

    Handle<Value> GetAttrsToBeSet(int to_set, struct stat* stat) {
        HandleScope scope;
        Local<Object> attrs = Object::New();

        if (to_set & FUSE_SET_ATTR_MODE) {
            attrs->Set(mode_sym, Integer::New(stat->st_mode));
        }

        if (to_set & FUSE_SET_ATTR_UID) {
            attrs->Set(uid_sym, Integer::New(stat->st_uid));
        }

        if (to_set & FUSE_SET_ATTR_GID) {
            attrs->Set(gid_sym, Integer::New(stat->st_gid));
        }

        if (to_set & FUSE_SET_ATTR_SIZE) {
            attrs->Set(size_sym, Number::New(stat->st_size));
        }

        if (to_set & FUSE_SET_ATTR_ATIME) {
            attrs->Set(atime_sym, NODE_UNIXTIME_V8(stat->st_atime));
        }

        if (to_set & FUSE_SET_ATTR_MTIME) {
            attrs->Set(mtime_sym, NODE_UNIXTIME_V8(stat->st_mtime));
        }

        return scope.Close(attrs);
    }

    Handle<Value> FileInfoToObject(struct fuse_file_info* fi) {
        HandleScope scope;
        Local<Object> info = Object::New();
        //TODO set accessors for info.fh

        info->Set(flags_sym, Integer::New(fi->flags));
        info->Set(writepage_sym, Integer::New(fi->writepage));
        info->Set(direct_io_sym, Integer::NewFromUnsigned(fi->direct_io));
        info->Set(keep_cache_sym, Integer::NewFromUnsigned(fi->keep_cache));
        info->Set(flush_sym, Integer::NewFromUnsigned(fi->flush));
        //info->Set(nonseekable_sym, Integer::NewFromUnsigned(fi->nonseekable));
        info->Set(file_handle_sym, Number::New(fi->fh));
        info->Set(lock_owner_sym, Number::New(fi->lock_owner));

        return scope.Close(info);
    }

    Handle<Value> FuseEntryParamToObject(const struct fuse_entry_param* entry) {
        HandleScope scope;

    }

    Handle<Value> RequestContextToObject(const struct fuse_ctx* ctx) {
        HandleScope scope;
        Local<Object> context = Object::New();

        context->Set(uid_sym, Integer::New(ctx->uid));
        context->Set(gid_sym, Integer::New(ctx->gid));
        context->Set(pid_sym, Integer::New(ctx->pid));

        return scope.Close(context);
    }

    NODE_MODULE(fuse, InitializeFuse)
} //namespace NodeFuse

