// Copyright 2012, Camilo Aguilar. Cloudescape, LLC.
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "file_info.h"

namespace NodeFuse {
    Persistent<FunctionTemplate> FileInfo::constructor_template;

    static Persistent<String> flags_sym;
    static Persistent<String> writepage_sym;
    static Persistent<String> direct_io_sym;
    static Persistent<String> keep_cache_sym;
    static Persistent<String> flush_sym;
    static Persistent<String> nonseekable_sym;
    static Persistent<String> file_handle_sym;
    static Persistent<String> lock_owner_sym;

    //Open flags
    static Persistent<String> rdonly_sym;
    static Persistent<String> wronly_sym;
    static Persistent<String> rdwr_sym;
    static Persistent<String> nonblock_sym;
    static Persistent<String> append_sym;
    static Persistent<String> creat_sym;
    static Persistent<String> trunc_sym;
    static Persistent<String> excl_sym;
#ifdef O_SHLOCK
    static Persistent<String> shlock_sym;
#endif
#ifdef O_EXLOCK
    static Persistent<String> exlock_sym;
#endif
    static Persistent<String> nofollow_sym;
#ifdef O_SYMLINK
    static Persistent<String> symlink_sym;
#endif
#ifdef O_EVTONLY
    static Persistent<String> evtonly_sym;
#endif


    void FileInfo::Initialize() {
        NanAssignPersistent( flags_sym         , NanNew("flags"));
        NanAssignPersistent( writepage_sym     , NanNew("writepage"));
        NanAssignPersistent( direct_io_sym     , NanNew("direct_io"));
        NanAssignPersistent( keep_cache_sym    , NanNew("keep_cache"));
        NanAssignPersistent( flush_sym         , NanNew("flush"));
        NanAssignPersistent( nonseekable_sym   , NanNew("nonseekable"));
        NanAssignPersistent( file_handle_sym   , NanNew("fh"));
        NanAssignPersistent( lock_owner_sym    , NanNew("lock_owner"));
        NanAssignPersistent( rdonly_sym    , NanNew("rdonly"));
        NanAssignPersistent( wronly_sym    , NanNew("wronly"));
        NanAssignPersistent( rdwr_sym      , NanNew("rdwr"));
        NanAssignPersistent( nonblock_sym  , NanNew("nonblock"));
        NanAssignPersistent( append_sym    , NanNew("append"));
        NanAssignPersistent( creat_sym     , NanNew("creat"));
        NanAssignPersistent( trunc_sym     , NanNew("trunc"));
        NanAssignPersistent( excl_sym      , NanNew("excl"));
        NanAssignPersistent( shlock_sym    , NanNew("shlock"));
        NanAssignPersistent( exlock_sym    , NanNew("exlock"));
        NanAssignPersistent( nofollow_sym  , NanNew("nofollow"));
        NanAssignPersistent( symlink_sym   , NanNew("symlink"));
        NanAssignPersistent( evtonly_sym   , NanNew("evtonly"));
        Local<FunctionTemplate> t = NanNew<FunctionTemplate>();
        Local<ObjectTemplate> object_tmpl = t->InstanceTemplate();

        object_tmpl->SetInternalFieldCount(1);

        object_tmpl->SetAccessor(NanNew(flags_sym), FileInfo::GetFlags);
        object_tmpl->SetAccessor(NanNew(writepage_sym), FileInfo::GetWritePage);
        object_tmpl->SetAccessor(NanNew(direct_io_sym), FileInfo::GetDirectIO, FileInfo::SetDirectIO);
        object_tmpl->SetAccessor(NanNew(keep_cache_sym), FileInfo::GetKeepCache, FileInfo::SetKeepCache);
        object_tmpl->SetAccessor(NanNew(flush_sym), FileInfo::GetFlush);
        object_tmpl->SetAccessor(NanNew(nonseekable_sym), FileInfo::GetNonSeekable, FileInfo::SetNonSeekable);
        object_tmpl->SetAccessor(NanNew(file_handle_sym), FileInfo::GetFileHandle, FileInfo::SetFileHandle);
        object_tmpl->SetAccessor(NanNew(lock_owner_sym), FileInfo::GetLockOwner);

        NanAssignPersistent(constructor_template, t);
        // constructor_template->SetClassName(NanNew<String>("FileInfo"));
    }

    FileInfo::FileInfo() : ObjectWrap() {}
    FileInfo::~FileInfo() {}

    NAN_GETTER(FileInfo::GetFlags){
        NanScope();
        FileInfo* fileInfo = ObjectWrap::Unwrap<FileInfo>(args.This());
        Local<Object> flagsObj = NanNew<Object>();

        //Initializes object
        flagsObj->Set(NanNew(rdonly_sym), NanFalse());
        flagsObj->Set(NanNew(wronly_sym), NanFalse());
        flagsObj->Set(NanNew(rdwr_sym), NanFalse());
        flagsObj->Set(NanNew(nonblock_sym), NanFalse());
        flagsObj->Set(NanNew(append_sym), NanFalse());
        flagsObj->Set(NanNew(creat_sym), NanFalse());
        flagsObj->Set(NanNew(trunc_sym), NanFalse());
        flagsObj->Set(NanNew(excl_sym), NanFalse());
#ifdef O_SHLOCK
        flagsObj->Set(NanNew(shlock_sym), NanFalse());
#endif
#ifdef O_EXLOCK
        flagsObj->Set(NanNew(exlock_sym), NanFalse());
#endif
        flagsObj->Set(NanNew(nofollow_sym), NanFalse());
#ifdef O_SYMLINK
        flagsObj->Set(NanNew(symlink_sym), NanFalse());
#endif
#ifdef O_EVTONLY
        flagsObj->Set(NanNew(evtonly_sym), NanFalse());
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
                flagsObj->Set(NanNew(rdonly_sym), NanTrue());
                break;
            case 1:
                flagsObj->Set(NanNew(wronly_sym), NanTrue());
                break;
            case 2:
                flagsObj->Set(NanNew(rdwr_sym), NanTrue());
                break;
        }
        if (flags & O_NONBLOCK) {
            flagsObj->Set(NanNew(nonblock_sym), NanTrue());
        }

        if (flags & O_APPEND) {
            flagsObj->Set(NanNew(append_sym), NanTrue());
        }

        if (flags & O_CREAT) {
            flagsObj->Set(NanNew(creat_sym), NanTrue());
        }

        if (flags & O_TRUNC) {
            flagsObj->Set(NanNew(trunc_sym), NanTrue());
        }

        if (flags & O_EXCL) {
            flagsObj->Set(NanNew(excl_sym), NanTrue());
        }

#ifdef O_SHLOCK
        if (flags & O_SHLOCK) {
            flagsObj->Set(NanNew(shlock_sym), NanTrue());
        }
#endif

#ifdef O_EXLOCK
        if (flags & O_EXLOCK) {
            flagsObj->Set(NanNew(exlock_sym), NanTrue());
        }
#endif

        if (flags & O_NOFOLLOW) {
            flagsObj->Set(NanNew(nofollow_sym), NanTrue());
        }

#ifdef O_SYMLINK
        if (flags & O_SYMLINK) {
            flagsObj->Set(NanNew(symlink_sym), NanTrue());
        }
#endif

#ifdef O_EVTONLY
        if (flags & O_EVTONLY) {
            flagsObj->Set(NanNew(evtonly_sym), NanTrue());
        }
#endif

        NanReturnValue(flagsObj);
    }

    NAN_GETTER(FileInfo::GetWritePage){
        NanScope();
        FileInfo *fileInfo = ObjectWrap::Unwrap<FileInfo>(args.This());
        NanReturnValue(fileInfo->fi->writepage ? NanTrue() : NanFalse());
    }

    NAN_GETTER(FileInfo::GetDirectIO){//(Local<String> property, const AccessorInfo& info) {
        NanScope();
        FileInfo *fileInfo = ObjectWrap::Unwrap<FileInfo>(args.This());
        NanReturnValue(fileInfo->fi->direct_io ? NanTrue() : NanFalse());
    }

    NAN_SETTER(FileInfo::SetDirectIO){
        FileInfo *fileInfo = ObjectWrap::Unwrap<FileInfo>(args.This());

        if (!value->IsBoolean()) {
            // TODO: Check to see if this case will cause errors
            // FUSEJS_THROW_EXCEPTION("Invalid value type: ", "a Boolean was expected");
        }

        fileInfo->fi->direct_io = value->IsTrue() ? 1 : 0;
    }

    NAN_GETTER(FileInfo::GetKeepCache){
        NanScope();
        FileInfo *fileInfo = ObjectWrap::Unwrap<FileInfo>(args.This());
        NanReturnValue(fileInfo->fi->keep_cache ? NanTrue() : NanFalse());
    }

    NAN_SETTER(FileInfo::SetKeepCache){
        FileInfo *fileInfo = ObjectWrap::Unwrap<FileInfo>(args.This());

        if (!value->IsBoolean()) {
            // TODO: Check to see if this case will cause errors
            // FUSEJS_THROW_EXCEPTION("Invalid value type: ", "a Boolean was expected");
        }

        fileInfo->fi->keep_cache = value->IsTrue() ? 1 : 0;
    }

    NAN_GETTER(FileInfo::GetFlush){
        NanScope();
        FileInfo *fileInfo = ObjectWrap::Unwrap<FileInfo>(args.This());
        NanReturnValue( fileInfo->fi->flush ? NanTrue() : NanFalse());
    }

    NAN_GETTER(FileInfo::GetNonSeekable){
        NanScope();     
#if FUSE_USE_VERSION > 27
        FileInfo *fileInfo = ObjectWrap::Unwrap<FileInfo>(args.This());
        NanReturnValue(fileInfo->fi->nonseekable ? NanTrue() : NanFalse());
#else
        NanReturnValue(NanFalse());
#endif
    }

    NAN_SETTER( FileInfo::SetNonSeekable){
        FileInfo *fileInfo = ObjectWrap::Unwrap<FileInfo>(args.This());

        if (!value->IsBoolean()) {
            FUSEJS_THROW_EXCEPTION("Invalid value type: ", "a Boolean was expected");
        }
#if FUSE_USE_VERSION > 27
        fileInfo->fi->nonseekable = value->IsTrue() ? 1 : 0;
#endif
    }

    NAN_SETTER(FileInfo::SetFileHandle){
        FileInfo *fileInfo = ObjectWrap::Unwrap<FileInfo>(args.This());

        if (!value->IsNumber()) {
            NanThrowTypeError("Invalid value type: a Number was expected");        
        }

        fileInfo->fi->fh = value->IntegerValue();
    }

    NAN_GETTER(FileInfo::GetFileHandle){
        NanScope();
        FileInfo *fileInfo = ObjectWrap::Unwrap<FileInfo>(args.This());

        NanReturnValue(NanNew<Integer>(fileInfo->fi->fh));
    }

    NAN_GETTER(FileInfo::GetLockOwner){
        NanScope();
        FileInfo *fileInfo = ObjectWrap::Unwrap<FileInfo>(args.This());

        NanReturnValue(NanNew<Integer>(fileInfo->fi->lock_owner));
    }
} //ends namespace NodeFuse
