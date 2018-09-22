/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "bindings.h"
#include "reply.h"
#include "file_info.h"
#include "filesystem.h"
#include "forget_data.h"
namespace NodeFuse {
    //stat struct symbols
    
    static Nan::Persistent<String> uid_sym;
    static Nan::Persistent<String> gid_sym;
    static Nan::Persistent<String> pid_sym;
    static Nan::Persistent<String> dev_sym;
    static Nan::Persistent<String> mode_sym;
    static Nan::Persistent<String> nlink_sym;
    static Nan::Persistent<String> rdev_sym;
    static Nan::Persistent<String> size_sym;
    static Nan::Persistent<String> blksize_sym;
    static Nan::Persistent<String> blocks_sym;
    static Nan::Persistent<String> atime_sym;
    static Nan::Persistent<String> mtime_sym;
    static Nan::Persistent<String> ctime_sym;
    //static Nan::Persistent<String> crtime_sym;

    //statvfs struct symbols
    static Nan::Persistent<String> bsize_sym;
    static Nan::Persistent<String> frsize_sym;
    //static Nan::Persistent<String> blocks_sym;
    static Nan::Persistent<String> bfree_sym;
    static Nan::Persistent<String> bavail_sym;
    static Nan::Persistent<String> files_sym;
    static Nan::Persistent<String> ffree_sym;
    static Nan::Persistent<String> favail_sym;
    static Nan::Persistent<String> fsid_sym;
    static Nan::Persistent<String> flag_sym;
    static Nan::Persistent<String> namemax_sym;

    //entry symbols
    static Nan::Persistent<String> ino_sym;
    static Nan::Persistent<String> generation_sym;
    static Nan::Persistent<String> attr_sym;
    static Nan::Persistent<String> attr_timeout_sym;
    static Nan::Persistent<String> entry_timeout_sym;

    //lock symbols
    static Nan::Persistent<String> type_sym;
    static Nan::Persistent<String> whence_sym;
    static Nan::Persistent<String> start_sym;
    static Nan::Persistent<String> len_sym;
    static Nan::Persistent<String> umask_sym;
    //static Nan::Persistent<String> pid_sym;


    void InitializeFuse(Handle<Object> target) {
        Nan::HandleScope scope;;
        
        /* Initialize persistent references */
        uid_sym.Reset( Nan::New<String>("uid").ToLocalChecked());
        gid_sym.Reset( Nan::New<String>("gid").ToLocalChecked());
        pid_sym.Reset( Nan::New<String>("pid").ToLocalChecked());
        dev_sym.Reset( Nan::New<String>("dev").ToLocalChecked());
        mode_sym.Reset( Nan::New<String>("mode").ToLocalChecked());
        nlink_sym.Reset( Nan::New<String>("nlink").ToLocalChecked());
        rdev_sym.Reset( Nan::New<String>("rdev").ToLocalChecked());
        size_sym.Reset( Nan::New<String>("size").ToLocalChecked());
        blksize_sym.Reset( Nan::New<String>("blksize").ToLocalChecked());
        blocks_sym.Reset( Nan::New<String>("blocks").ToLocalChecked());
        atime_sym.Reset( Nan::New<String>("atime").ToLocalChecked());
        mtime_sym.Reset( Nan::New<String>("mtime").ToLocalChecked());
        ctime_sym.Reset( Nan::New<String>("ctime").ToLocalChecked());
        /*
        #ifdef __APPLE__
        crtime_sym.Reset( Nan::New<String>("crtime").ToLocalChecked());
        #endif
        */
        bsize_sym.Reset( Nan::New<String>("bsize").ToLocalChecked());
        frsize_sym.Reset( Nan::New<String>("frsize").ToLocalChecked());
        blocks_sym.Reset( Nan::New<String>("blocks").ToLocalChecked());
        bfree_sym.Reset( Nan::New<String>("bfree").ToLocalChecked());
        bavail_sym.Reset( Nan::New<String>("bavail").ToLocalChecked());
        files_sym.Reset( Nan::New<String>("files").ToLocalChecked());
        ffree_sym.Reset( Nan::New<String>("ffree").ToLocalChecked());
        favail_sym.Reset( Nan::New<String>("favail").ToLocalChecked());
        fsid_sym.Reset( Nan::New<String>("fsid").ToLocalChecked());
        flag_sym.Reset( Nan::New<String>("flag").ToLocalChecked());
        namemax_sym.Reset( Nan::New<String>("namemax").ToLocalChecked());
        ino_sym.Reset( Nan::New<String>("inode").ToLocalChecked());
        generation_sym.Reset( Nan::New<String>("generation").ToLocalChecked());
        attr_sym.Reset( Nan::New<String>("attr").ToLocalChecked());
        attr_timeout_sym.Reset( Nan::New<String>("attr_timeout").ToLocalChecked());
        entry_timeout_sym.Reset( Nan::New<String>("entry_timeout").ToLocalChecked());
        type_sym.Reset( Nan::New<String>("type").ToLocalChecked());
        whence_sym.Reset( Nan::New<String>("whence").ToLocalChecked());
        start_sym.Reset( Nan::New<String>("start").ToLocalChecked());
        len_sym.Reset( Nan::New<String>("len").ToLocalChecked());
        pid_sym.Reset( Nan::New<String>("pid").ToLocalChecked());
        umask_sym.Reset( Nan::New<String>("umask").ToLocalChecked());
        
        Fuse::Initialize(target);
        FileSystem::Initialize(target);
        Reply::Initialize(target);
        FileInfo::Initialize(target);

        #if FUSE_VERSION > 28
        #ifndef __APPLE__
        ForgetData::Initialize(target);
        #endif 
        #endif
        
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
        entry->ino = obj->Get(Nan::New(ino_sym))->IntegerValue();
        entry->generation = obj->Get(Nan::New(generation_sym))->IntegerValue();
        entry->attr_timeout = obj->Get(Nan::New(attr_timeout_sym))->IntegerValue();
        entry->entry_timeout = obj->Get(Nan::New(entry_timeout_sym))->IntegerValue();

        //struct stat statbuf;
        ret = ObjectToStat(obj->Get(Nan::New(attr_sym)), &entry->attr);

        return ret;
    }

    int ObjectToStat(Handle<Value> value, struct stat* statbuf) {
        Nan::HandleScope scope;;

        memset(statbuf, 0, sizeof(struct stat));

        Local<Object> obj = value->ToObject();

        //statbuf->st_dev = obj->Get(Nan::New(dev_sym))->IntegerValue();
        // statbuf->st_dev = Nan::Get(obj, Nan::New(dev_sym).ToLocalChecked())->IntegerValue();
        statbuf->st_ino = obj->Get(Nan::New(ino_sym))->IntegerValue();
        // statbuf->st_ino = Nan::GetRealNamedProperty(obj, Nan::New(inode_sym).ToLocalChecked()).ToLocalChecked()->IntegerValue();
        statbuf->st_mode = obj->Get(Nan::New(mode_sym))->IntegerValue();
        statbuf->st_nlink = obj->Get(Nan::New(nlink_sym))->IntegerValue();
        statbuf->st_uid = obj->Get(Nan::New(uid_sym))->IntegerValue();
        statbuf->st_gid = obj->Get(Nan::New(gid_sym))->IntegerValue();
        statbuf->st_rdev = obj->Get(Nan::New(rdev_sym))->IntegerValue();
        statbuf->st_size = obj->Get(Nan::New(size_sym))->IntegerValue();
        // Local<String> sizeStr = obj->Get(Nan::New(size_sym))->ToString();
        // Nan::Utf8String sizeBuf(sizeStr);
        // statbuf->st_size = atoll( *sizeBuf );

        //statbuf->st_blksize = obj->Get(Nan::New(blksize_sym))->IntegerValue();
        //statbuf->st_blocks = obj->Get(Nan::New(blocks_sym))->IntegerValue();
        statbuf->st_atime = obj->Get(Nan::New(atime_sym))->IntegerValue();
        statbuf->st_mtime = obj->Get(Nan::New(mtime_sym))->IntegerValue();
        statbuf->st_ctime = obj->Get(Nan::New(ctime_sym))->IntegerValue();
        
        /*
        #ifdef __APPLE__
        #if FUSE_SET_ATTR_CRTIME
        statbuf->st_crtime = obj->Get(Nan::New(crtime_sym))->IntegerValue();
        #endif
        #endif
        */

        return 0;
    }

    int ObjectToStatVfs(Handle<Value> value, struct statvfs* statbuf) {
        Nan::HandleScope scope;;

        memset(statbuf, 0, sizeof(struct statvfs));

        Local<Object> obj = value->ToObject();

        statbuf->f_bsize = obj->Get(Nan::New(bsize_sym))->IntegerValue();
        statbuf->f_frsize = obj->Get(Nan::New(frsize_sym))->IntegerValue();

        statbuf->f_blocks = obj->Get(Nan::New(blocks_sym))->IntegerValue();
        statbuf->f_bfree = obj->Get(Nan::New(bfree_sym))->IntegerValue();
        statbuf->f_bavail = obj->Get(Nan::New(bavail_sym))->IntegerValue();
        statbuf->f_files = obj->Get(Nan::New(files_sym))->IntegerValue();
        statbuf->f_ffree = obj->Get(Nan::New(ffree_sym))->IntegerValue();
        statbuf->f_favail = obj->Get(Nan::New(favail_sym))->IntegerValue();

        statbuf->f_fsid = obj->Get(Nan::New(fsid_sym))->IntegerValue();
        statbuf->f_flag = obj->Get(Nan::New(flag_sym))->IntegerValue();
        statbuf->f_namemax = obj->Get(Nan::New(namemax_sym))->IntegerValue();

        return 0;
    }

    int ObjectToFlock(Handle<Value> value, struct flock* lock) {
        Nan::HandleScope scope;;

        memset(lock, 0, sizeof(struct flock));

        Local<Object> obj = value->ToObject();

        lock->l_type = obj->Get(Nan::New(type_sym))->IntegerValue();
        lock->l_whence = obj->Get(Nan::New(whence_sym))->IntegerValue();
        lock->l_start = obj->Get(Nan::New(start_sym))->IntegerValue();
        lock->l_len = obj->Get(Nan::New(len_sym))->IntegerValue();
        lock->l_pid = obj->Get(Nan::New(pid_sym))->IntegerValue();

        return 0;
    }

    Handle<Value> GetAttrsToBeSet(int to_set, struct stat* stat) {
        Local<Object> attrs = Nan::New<Object>();

        if (to_set & FUSE_SET_ATTR_MODE) {
            attrs->Set( Nan::New(mode_sym), Nan::New<Integer>(stat->st_mode));
        }

        if (to_set & FUSE_SET_ATTR_UID) {
            attrs->Set( Nan::New(uid_sym), Nan::New<Integer>(stat->st_uid));
        }

        if (to_set & FUSE_SET_ATTR_GID) {
            attrs->Set( Nan::New(gid_sym), Nan::New<Integer>(stat->st_gid));
        }

        if (to_set & FUSE_SET_ATTR_SIZE) {
            attrs->Set( Nan::New(size_sym), Nan::New<Number>((double)stat->st_size));
        }

        if (to_set & FUSE_SET_ATTR_ATIME) {
            attrs->Set( Nan::New(atime_sym), NODE_UNIXTIME_V8(stat->st_atime));
        }

        if (to_set & FUSE_SET_ATTR_MTIME) {
            attrs->Set( Nan::New(mtime_sym), NODE_UNIXTIME_V8(stat->st_mtime));
        }

        /*
        #ifdef __APPLE__
        #if FUSE_SET_ATTR_CRTIME
         if (to_set & FUSE_SET_ATTR_CRTIME) {
            attrs->Set( Nan::New(crtime_sym), Nan::New<Integer>(stat->st_crtime));
        }
        #endif
        #endif
        */

        /* this will be decided on the upper level'
        #ifdef FUSE_SET_ATTR_ATIME_NOW
        if (to_set & FUSE_SET_ATTR_ATIME_NOW ){
            attrs->Set( Nan::New(atime_sym), Nan::New<Integer>(-1));
        }

        if (to_set & FUSE_SET_ATTR_MTIME_NOW ){
            attrs->Set( Nan::New(mtime_sym), Nan::New<Integer>(-1));
        }
        #endif
        */



        return attrs;
    }

    Handle<Value> RequestContextToObject(const struct fuse_ctx* ctx) {
        Local<Object> context = Nan::New<Object>();

        context->Set( Nan::New(uid_sym), Nan::New<Integer>(ctx->uid));
        context->Set( Nan::New(gid_sym), Nan::New<Integer>(ctx->gid));
        context->Set( Nan::New(pid_sym), Nan::New<Integer>(ctx->pid));

        #if FUSE_VERSION > 28 && !__APPLE__

        context->Set( Nan::New(umask_sym), Nan::New<Integer>(ctx->umask));
        #endif

        return context;
    }

    Handle<Value> FlockToObject(const struct flock *lock) {
        Local<Object> rv = Nan::New<Object>();

        //Specifies the type of the lock; one of F_RDLCK, F_WRLCK, or F_UNLCK.
        rv->Set( Nan::New(type_sym), Nan::New<Integer>(lock->l_type)); //TODO convert to object with accessors
        //This corresponds to the whence argument to fseek or lseek, and specifies what the offset is relative to. Its value can be one of SEEK_SET, SEEK_CUR, or SEEK_END.
        rv->Set( Nan::New(whence_sym), Nan::New<Integer>(lock->l_whence)); //TODO convert to object with accessors
        rv->Set( Nan::New(start_sym), Nan::New<Integer>( (int) lock->l_start));
        rv->Set( Nan::New(len_sym), Nan::New<Integer>( (int) lock->l_len));
        rv->Set( Nan::New(pid_sym), Nan::New<Integer>( (int) lock->l_pid));

        return rv;
    }

    NODE_MODULE(fusejs, InitializeFuse)
} //namespace NodeFuse

