// Copyright 2012, Camilo Aguilar. Cloudescape, LLC.
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "file_info.h"

namespace NodeFuse {
    Persistent<FunctionTemplate> FileInfo::constructor_template;

    static Persistent<String> flags_sym         = NODE_PSYMBOL("flags");
    static Persistent<String> writepage_sym     = NODE_PSYMBOL("writepage");
    static Persistent<String> direct_io_sym     = NODE_PSYMBOL("direct_io");
    static Persistent<String> keep_cache_sym    = NODE_PSYMBOL("keep_cache");
    static Persistent<String> flush_sym         = NODE_PSYMBOL("flush");
    static Persistent<String> nonseekable_sym   = NODE_PSYMBOL("nonseekable");
    static Persistent<String> file_handle_sym   = NODE_PSYMBOL("fh");
    static Persistent<String> lock_owner_sym    = NODE_PSYMBOL("lock_owner");

    //Open flags
    static Persistent<String> rdonly_sym    = NODE_PSYMBOL("rdonly");
    static Persistent<String> wronly_sym    = NODE_PSYMBOL("wronly");
    static Persistent<String> rdwr_sym      = NODE_PSYMBOL("rdwr");
    static Persistent<String> nonblock_sym  = NODE_PSYMBOL("nonblock");
    static Persistent<String> append_sym    = NODE_PSYMBOL("append");
    static Persistent<String> creat_sym     = NODE_PSYMBOL("creat");
    static Persistent<String> trunc_sym     = NODE_PSYMBOL("trunc");
    static Persistent<String> excl_sym      = NODE_PSYMBOL("excl");
#ifdef O_SHLOCK
    static Persistent<String> shlock_sym    = NODE_PSYMBOL("shlock");
#endif
#ifdef O_EXLOCK
    static Persistent<String> exlock_sym    = NODE_PSYMBOL("exlock");
#endif
    static Persistent<String> nofollow_sym  = NODE_PSYMBOL("nofollow");
#ifdef O_SYMLINK
    static Persistent<String> symlink_sym   = NODE_PSYMBOL("symlink");
#endif
#ifdef O_EVTONLY
    static Persistent<String> evtonly_sym   = NODE_PSYMBOL("evtonly");
#endif


    void FileInfo::Initialize() {
        Local<FunctionTemplate> t = FunctionTemplate::New();
        Local<ObjectTemplate> object_tmpl = t->InstanceTemplate();

        object_tmpl->SetInternalFieldCount(1);

        object_tmpl->SetAccessor(flags_sym, FileInfo::GetFlags);
        object_tmpl->SetAccessor(writepage_sym, FileInfo::GetWritePage);
        object_tmpl->SetAccessor(direct_io_sym, FileInfo::GetDirectIO, FileInfo::SetDirectIO);
        object_tmpl->SetAccessor(keep_cache_sym, FileInfo::GetKeepCache, FileInfo::SetKeepCache);
        object_tmpl->SetAccessor(flush_sym, FileInfo::GetFlush);
        object_tmpl->SetAccessor(nonseekable_sym, FileInfo::GetNonSeekable, FileInfo::SetNonSeekable);
        object_tmpl->SetAccessor(file_handle_sym, FileInfo::GetFileHandle, FileInfo::SetFileHandle);
        object_tmpl->SetAccessor(lock_owner_sym, FileInfo::GetLockOwner);

        constructor_template = Persistent<FunctionTemplate>::New(t);
        constructor_template->SetClassName(String::NewSymbol("FileInfo"));
    }

    FileInfo::FileInfo() : ObjectWrap() {}
    FileInfo::~FileInfo() {}

    Handle<Value> FileInfo::GetFlags(Local<String> property, const AccessorInfo& info) {
        HandleScope scope;
        FileInfo* fileInfo = ObjectWrap::Unwrap<FileInfo>(info.This());
        Local<Object> flagsObj = Object::New();

        //Initializes object
        flagsObj->Set(rdonly_sym, False());
        flagsObj->Set(wronly_sym, False());
        flagsObj->Set(rdwr_sym, False());
        flagsObj->Set(nonblock_sym, False());
        flagsObj->Set(append_sym, False());
        flagsObj->Set(creat_sym, False());
        flagsObj->Set(trunc_sym, False());
        flagsObj->Set(excl_sym, False());
#ifdef O_SHLOCK
        flagsObj->Set(shlock_sym, False());
#endif
#ifdef O_EXLOCK
        flagsObj->Set(exlock_sym, False());
#endif
        flagsObj->Set(nofollow_sym, False());
#ifdef O_SYMLINK
        flagsObj->Set(symlink_sym, False());
#endif
#ifdef O_EVTONLY
        flagsObj->Set(evtonly_sym, False());
#endif
        /*
           O_RDONLY        open for reading only
           O_WRONLY        open for writing only
           O_RDWR          open for reading and writing
           O_NONBLOCK      do not block on open or for data to become available
           O_APPEND        append on each write
           O_CREAT         create file if it does not exist
           O_TRUNC         truncate size to 0
           O_EXCL          error if O_CREAT and the file exists
           O_SHLOCK        atomically obtain a shared lock
           O_EXLOCK        atomically obtain an exclusive lock
           O_NOFOLLOW      do not follow symlinks
           O_SYMLINK       allow open of symlinks
           O_EVTONLY       descriptor requested for event notifications only
        */

        int flags = fileInfo->fi->flags;

        switch( flags & 3){
            case 0:
                flagsObj->Set(rdonly_sym, True());
                break;
            case 1:
                flagsObj->Set(wronly_sym, True());
                break;
            case 2:
                flagsObj->Set(rdwr_sym, True());
                break;
        }
        if (flags & O_NONBLOCK) {
            flagsObj->Set(nonblock_sym, True());
        }

        if (flags & O_APPEND) {
            flagsObj->Set(append_sym, True());
        }

        if (flags & O_CREAT) {
            flagsObj->Set(creat_sym, True());
        }

        if (flags & O_TRUNC) {
            flagsObj->Set(trunc_sym, True());
        }

        if (flags & O_EXCL) {
            flagsObj->Set(excl_sym, True());
        }

#ifdef O_SHLOCK
        if (flags & O_SHLOCK) {
            flagsObj->Set(shlock_sym, True());
        }
#endif

#ifdef O_EXLOCK
        if (flags & O_EXLOCK) {
            flagsObj->Set(exlock_sym, True());
        }
#endif

        if (flags & O_NOFOLLOW) {
            flagsObj->Set(nofollow_sym, True());
        }

#ifdef O_SYMLINK
        if (flags & O_SYMLINK) {
            flagsObj->Set(symlink_sym, True());
        }
#endif

#ifdef O_EVTONLY
        if (flags & O_EVTONLY) {
            flagsObj->Set(evtonly_sym, True());
        }
#endif

        return scope.Close(flagsObj);
    }

    Handle<Value> FileInfo::GetWritePage(Local<String> property, const AccessorInfo& info) {
        FileInfo *fileInfo = ObjectWrap::Unwrap<FileInfo>(info.This());
        return fileInfo->fi->writepage ? True() : False();
    }

    Handle<Value> FileInfo::GetDirectIO(Local<String> property, const AccessorInfo& info) {
        FileInfo *fileInfo = ObjectWrap::Unwrap<FileInfo>(info.This());
        return fileInfo->fi->direct_io ? True() : False();
    }

    void FileInfo::SetDirectIO(Local<String> property, Local<Value> value, const AccessorInfo& info) {
        FileInfo *fileInfo = ObjectWrap::Unwrap<FileInfo>(info.This());

        if (!value->IsBoolean()) {
            // TODO: Check to see if this case will cause errors
            // FUSEJS_THROW_EXCEPTION("Invalid value type: ", "a Boolean was expected");
        }

        fileInfo->fi->direct_io = value->IsTrue() ? 1 : 0;
    }

    Handle<Value> FileInfo::GetKeepCache(Local<String> property, const AccessorInfo& info) {
        FileInfo *fileInfo = ObjectWrap::Unwrap<FileInfo>(info.This());
        return fileInfo->fi->keep_cache ? True() : False();
    }

    void FileInfo::SetKeepCache(Local<String> property, Local<Value> value, const AccessorInfo& info) {
        FileInfo *fileInfo = ObjectWrap::Unwrap<FileInfo>(info.This());

        if (!value->IsBoolean()) {
            // TODO: Check to see if this case will cause errors
            // FUSEJS_THROW_EXCEPTION("Invalid value type: ", "a Boolean was expected");
        }

        fileInfo->fi->keep_cache = value->IsTrue() ? 1 : 0;
    }

    Handle<Value> FileInfo::GetFlush(Local<String> property, const AccessorInfo& info) {
        FileInfo *fileInfo = ObjectWrap::Unwrap<FileInfo>(info.This());
        return fileInfo->fi->flush ? True() : False();
    }

    Handle<Value> FileInfo::GetNonSeekable(Local<String> property, const AccessorInfo& info) {
        FileInfo *fileInfo = ObjectWrap::Unwrap<FileInfo>(info.This());
#if FUSE_USE_VERSION > 27
        return fileInfo->fi->nonseekable ? True() : False();
#else
        return False();
#endif
    }

    void FileInfo::SetNonSeekable(Local<String> property, Local<Value> value, const AccessorInfo& info) {
        FileInfo *fileInfo = ObjectWrap::Unwrap<FileInfo>(info.This());

        if (!value->IsBoolean()) {
            // TODO: Check to see if this case will cause errors
            // FUSEJS_THROW_EXCEPTION("Invalid value type: ", "a Boolean was expected");
        }
#if FUSE_USE_VERSION > 27
        fileInfo->fi->nonseekable = value->IsTrue() ? 1 : 0;
#endif
    }

    void FileInfo::SetFileHandle(Local<String> property, Local<Value> value, const AccessorInfo& info) {
        FileInfo *fileInfo = ObjectWrap::Unwrap<FileInfo>(info.This());

        if (!value->IsNumber()) {
            // TODO: Check to see if this case will cause errors
            // FUSEJS_THROW_EXCEPTION("Invalid value type: ", "a Number was expected");
        }

        fileInfo->fi->fh = value->IntegerValue();
    }

    Handle<Value> FileInfo::GetFileHandle(Local<String> property, const AccessorInfo& info) {
        HandleScope scope;
        FileInfo *fileInfo = ObjectWrap::Unwrap<FileInfo>(info.This());

        return scope.Close(Integer::New(fileInfo->fi->fh));
    }

    Handle<Value> FileInfo::GetLockOwner(Local<String> property, const AccessorInfo& info) {
        HandleScope scope;
        FileInfo *fileInfo = ObjectWrap::Unwrap<FileInfo>(info.This());

        return scope.Close(Integer::New(fileInfo->fi->lock_owner));
    }
} //ends namespace NodeFuse
