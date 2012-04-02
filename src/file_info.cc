// Copyright 2012, Camilo Aguilar. Cloudescape, LLC.
#include "file_info.h"

namespace NodeFuse {
    Persistent<FunctionTemplate> FileInfo::constructor_template;

    //FIXME DRY by writing a macro

    static Persistent<String> flags_sym         = NODE_PSYMBOL("flags");
    static Persistent<String> writepage_sym     = NODE_PSYMBOL("writepage");
    static Persistent<String> direct_io_sym     = NODE_PSYMBOL("direct_io");
    static Persistent<String> keep_cache_sym    = NODE_PSYMBOL("keep_cache");
    static Persistent<String> flush_sym         = NODE_PSYMBOL("flush");
    static Persistent<String> nonseekable_sym   = NODE_PSYMBOL("nonseekable");
    static Persistent<String> file_handle_sym   = NODE_PSYMBOL("fh");
    static Persistent<String> lock_owner_sym    = NODE_PSYMBOL("lock_owner");

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

    Handle<Value> FileInfo::GetFlags(Local<String> property, const AccessorInfo& info) {
        FileInfo *fileInfo = ObjectWrap::Unwrap<FileInfo>(info.This());
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
            FUSEJS_THROW_EXCEPTION("Invalid value type: ", "a Boolean was expected");
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
            FUSEJS_THROW_EXCEPTION("Invalid value type: ", "a Boolean was expected");
        }

        fileInfo->fi->keep_cache = value->IsTrue() ? 1 : 0;
    }

    Handle<Value> FileInfo::GetFlush(Local<String> property, const AccessorInfo& info) {
        FileInfo *fileInfo = ObjectWrap::Unwrap<FileInfo>(info.This());
        return fileInfo->fi->flush ? True() : False();
    }

    Handle<Value> FileInfo::GetNonSeekable(Local<String> property, const AccessorInfo& info) {
        FileInfo *fileInfo = ObjectWrap::Unwrap<FileInfo>(info.This());
        //TODO check FUSE VERSION
        //return fileInfo->fi->nonseekable ? True() : False();
        return False();
    }

    void FileInfo::SetNonSeekable(Local<String> property, Local<Value> value, const AccessorInfo& info) {
        FileInfo *fileInfo = ObjectWrap::Unwrap<FileInfo>(info.This());

        if (!value->IsBoolean()) {
            FUSEJS_THROW_EXCEPTION("Invalid value type: ", "a Boolean was expected");
        }
        //fileInfo->fi->nonseekable = value->IsTrue() ? 1 : 0;
    }

    void FileInfo::SetFileHandle(Local<String> property, Local<Value> value, const AccessorInfo& info) {
        FileInfo *fileInfo = ObjectWrap::Unwrap<FileInfo>(info.This());

        if (!value->IsNumber()) {
            FUSEJS_THROW_EXCEPTION("Invalid value type: ", "a Number was expected");
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
