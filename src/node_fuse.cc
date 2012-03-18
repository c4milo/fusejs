// Copyright 2012, Camilo Aguilar. Cloudescape, LLC.
#include "bindings.h"
#include "reply.h"

namespace NodeFuse {
    static Persistent<String> uid_sym = NODE_PSYMBOL("uid");
    static Persistent<String> gid_sym = NODE_PSYMBOL("gid");
    static Persistent<String> pid_sym = NODE_PSYMBOL("pid");
    static Persistent<String> dev_sym = NODE_PSYMBOL("dev");
    static Persistent<String> mode_sym = NODE_PSYMBOL("mode");
    static Persistent<String> nlink_sym = NODE_PSYMBOL("nlink");
    static Persistent<String> rdev_sym = NODE_PSYMBOL("rdev");
    static Persistent<String> size_sym = NODE_PSYMBOL("size");
    static Persistent<String> blksize_sym = NODE_PSYMBOL("blksize");
    static Persistent<String> blocks_sym = NODE_PSYMBOL("blocks");
    static Persistent<String> atime_sym = NODE_PSYMBOL("atime");
    static Persistent<String> mtime_sym = NODE_PSYMBOL("mtime");
    static Persistent<String> ctime_sym = NODE_PSYMBOL("ctime");

    static Persistent<String> ino_sym = NODE_PSYMBOL("inode");
    static Persistent<String> generation_sym = NODE_PSYMBOL("generation");
    static Persistent<String> attr_sym = NODE_PSYMBOL("attr");
    static Persistent<String> attr_timeout_sym = NODE_PSYMBOL("attr_timeout");
    static Persistent<String> entry_timeout_sym = NODE_PSYMBOL("entry_timeout");

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

