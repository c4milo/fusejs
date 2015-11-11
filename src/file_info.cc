/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "file_info.h"

namespace NodeFuse {
    Nan::Persistent<Function> FileInfo::constructor;
    
    static Nan::Persistent<String> flags_sym;
    static Nan::Persistent<String> writepage_sym;
    static Nan::Persistent<String> direct_io_sym;
    static Nan::Persistent<String> keep_cache_sym;
    static Nan::Persistent<String> flush_sym;
    static Nan::Persistent<String> nonseekable_sym;
    static Nan::Persistent<String> file_handle_sym;
    static Nan::Persistent<String> lock_owner_sym;

    //Open flags
    static Nan::Persistent<String> rdonly_sym;
    static Nan::Persistent<String> wronly_sym;
    static Nan::Persistent<String> rdwr_sym;
    static Nan::Persistent<String> nonblock_sym;
    static Nan::Persistent<String> append_sym;
    static Nan::Persistent<String> creat_sym;
    static Nan::Persistent<String> trunc_sym;
    static Nan::Persistent<String> excl_sym;
#ifdef O_SHLOCK
    static Nan::Persistent<String> shlock_sym;
#endif
#ifdef O_EXLOCK
    static Nan::Persistent<String> exlock_sym;
#endif
    static Nan::Persistent<String> nofollow_sym;
#ifdef O_SYMLINK
    static Nan::Persistent<String> symlink_sym;
#endif
#ifdef O_EVTONLY
    static Nan::Persistent<String> evtonly_sym;
#endif

    NAN_METHOD(FileInfo::New){
        if (info.IsConstructCall()) {
          FileInfo *fi = new FileInfo();
          Local<Object> obj = info.This();
          fi->Wrap(obj);
          info.GetReturnValue().Set( obj );
        } else {
          Local<Function> cons = Nan::New<Function>(constructor);
          info.GetReturnValue().Set(cons->NewInstance());
        }

    }

    void FileInfo::Initialize(Handle<Object> target) {
        
        flags_sym.Reset( Nan::New("flags").ToLocalChecked());
        writepage_sym.Reset( Nan::New("writepage").ToLocalChecked());
        direct_io_sym.Reset( Nan::New("direct_io").ToLocalChecked());
        keep_cache_sym.Reset( Nan::New("keep_cache").ToLocalChecked());
        flush_sym.Reset( Nan::New("flush").ToLocalChecked());
        nonseekable_sym.Reset( Nan::New("nonseekable").ToLocalChecked());
        file_handle_sym.Reset( Nan::New("file_handle").ToLocalChecked());
        lock_owner_sym.Reset( Nan::New("lock_owner").ToLocalChecked());
        rdonly_sym.Reset( Nan::New("rdonly").ToLocalChecked());
        wronly_sym.Reset( Nan::New("wronly").ToLocalChecked());
        rdwr_sym.Reset( Nan::New("rdwr").ToLocalChecked());
        nonblock_sym.Reset( Nan::New("nonblock").ToLocalChecked());
        append_sym.Reset( Nan::New("append").ToLocalChecked());
        creat_sym.Reset( Nan::New("creat").ToLocalChecked());
        trunc_sym.Reset( Nan::New("trunc").ToLocalChecked());
        excl_sym.Reset( Nan::New("excl").ToLocalChecked());
        #ifdef O_SHLOCK
        shlock_sym.Reset( Nan::New("shlock").ToLocalChecked());
        #endif
        #ifdef O_EXLOCK
        exlock_sym.Reset( Nan::New("exlock").ToLocalChecked());
        #endif
        nofollow_sym.Reset( Nan::New("nofollow").ToLocalChecked());
        #ifdef O_SYMLINK
        symlink_sym.Reset( Nan::New("symlink").ToLocalChecked());
        #endif
        #ifdef O_EVTONLY
        evtonly_sym.Reset( Nan::New("evtonly").ToLocalChecked());
        #endif
        
        Local<v8::FunctionTemplate> tpl = Nan::New<v8::FunctionTemplate>(New);
        tpl->SetClassName(Nan::New<v8::String>("FileInfo").ToLocalChecked());
        tpl->InstanceTemplate()->SetInternalFieldCount(1);

        Local<ObjectTemplate> object_tmpl = tpl->InstanceTemplate();

        object_tmpl->SetInternalFieldCount(1);

        Nan::SetAccessor(object_tmpl, Nan::New(flags_sym), FileInfo::GetFlags);
        Nan::SetAccessor(object_tmpl, Nan::New(writepage_sym), FileInfo::GetWritePage);
        Nan::SetAccessor(object_tmpl, Nan::New(direct_io_sym), FileInfo::GetDirectIO, FileInfo::SetDirectIO);
        Nan::SetAccessor(object_tmpl, Nan::New(keep_cache_sym), FileInfo::GetKeepCache, FileInfo::SetKeepCache);
        Nan::SetAccessor(object_tmpl, Nan::New(flush_sym), FileInfo::GetFlush);
        Nan::SetAccessor(object_tmpl, Nan::New(nonseekable_sym), FileInfo::GetNonSeekable, FileInfo::SetNonSeekable);
        Nan::SetAccessor(object_tmpl, Nan::New(file_handle_sym), FileInfo::GetFileHandle, FileInfo::SetFileHandle);
        Nan::SetAccessor(object_tmpl, Nan::New(lock_owner_sym), FileInfo::GetLockOwner);
        constructor.Reset(tpl->GetFunction());

    }

    FileInfo::FileInfo() : ObjectWrap() {}
    FileInfo::~FileInfo() {
        // if(this->fi != nullptr)
        // {
        //     free(this->fi);
        // }
        
    }

    NAN_GETTER(FileInfo::GetFlags){
        FileInfo* fileInfo = ObjectWrap::Unwrap<FileInfo>(info.This());
        Local<Object> flagsObj = Nan::New<Object>();

        //Initializes object
        Nan::Set(flagsObj, Nan::New(rdonly_sym), Nan::False());
        Nan::Set(flagsObj, Nan::New(wronly_sym), Nan::False());
        Nan::Set(flagsObj, Nan::New(rdwr_sym), Nan::False());
        Nan::Set(flagsObj, Nan::New(nonblock_sym), Nan::False());
        Nan::Set(flagsObj, Nan::New(append_sym), Nan::False());
        Nan::Set(flagsObj, Nan::New(creat_sym), Nan::False());
        Nan::Set(flagsObj, Nan::New(trunc_sym), Nan::False());
        Nan::Set(flagsObj, Nan::New(excl_sym), Nan::False());
#ifdef O_SHLOCK
        Nan::Set(flagsObj, Nan::New(shlock_sym), Nan::False());
#endif
#ifdef O_EXLOCK
        Nan::Set(flagsObj, Nan::New(exlock_sym), Nan::False());
#endif
        Nan::Set(flagsObj, Nan::New(nofollow_sym), Nan::False());
#ifdef O_SYMLINK
        Nan::Set(flagsObj, Nan::New(symlink_sym), Nan::False());
#endif
#ifdef O_EVTONLY
        Nan::Set(flagsObj, Nan::New(evtonly_sym), Nan::False());
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

        int flags = fileInfo->fi.flags;

        switch( flags & 3){
            case 0:
                Nan::Set(flagsObj, Nan::New(rdonly_sym), Nan::True());
                break;
            case 1:
                Nan::Set(flagsObj, Nan::New(wronly_sym), Nan::True());
                break;
            case 2:
                Nan::Set(flagsObj, Nan::New(rdwr_sym), Nan::True());
                break;
        }
        if (flags & O_NONBLOCK) {
            Nan::Set(flagsObj, Nan::New(nonblock_sym), Nan::True());
        }

        if (flags & O_APPEND) {
            Nan::Set(flagsObj, Nan::New(append_sym), Nan::True());
        }

        if (flags & O_CREAT) {
            Nan::Set(flagsObj, Nan::New(creat_sym), Nan::True());
        }

        if (flags & O_TRUNC) {
            Nan::Set(flagsObj, Nan::New(trunc_sym), Nan::True());
        }

        if (flags & O_EXCL) {
            Nan::Set(flagsObj, Nan::New(excl_sym), Nan::True());
        }

#ifdef O_SHLOCK
        if (flags & O_SHLOCK) {
            Nan::Set(flagsObj, Nan::New(shlock_sym), Nan::True());
        }
#endif

#ifdef O_EXLOCK
        if (flags & O_EXLOCK) {
            Nan::Set(flagsObj, Nan::New(exlock_sym), Nan::True());
        }
#endif

        if (flags & O_NOFOLLOW) {
            Nan::Set(flagsObj, Nan::New(nofollow_sym), Nan::True());
        }

#ifdef O_SYMLINK
        if (flags & O_SYMLINK) {
            Nan::Set(flagsObj, Nan::New(symlink_sym), Nan::True());
        }
#endif

#ifdef O_EVTONLY
        if (flags & O_EVTONLY) {
            Nan::Set(flagsObj, Nan::New(evtonly_sym), Nan::True());
        }
#endif

        info.GetReturnValue().Set(flagsObj);
    }

    NAN_GETTER(FileInfo::GetWritePage){
        FileInfo *fileInfo = ObjectWrap::Unwrap<FileInfo>(info.This());
        info.GetReturnValue().Set(fileInfo->fi.writepage ? Nan::True() : Nan::False());
    }

    NAN_GETTER(FileInfo::GetDirectIO){//(Local<String> property, const AccessorInfo& info) {
        FileInfo *fileInfo = ObjectWrap::Unwrap<FileInfo>(info.This());
        info.GetReturnValue().Set(fileInfo->fi.direct_io ? Nan::True() : Nan::False());
    }

    NAN_SETTER(FileInfo::SetDirectIO){
        FileInfo *fileInfo = ObjectWrap::Unwrap<FileInfo>(info.This());

        if (!value->IsBoolean()) {
            // TODO: Check to see if this case will cause errors
            // FUSEJS_THROW_EXCEPTION("Invalid value type: ", "a Boolean was expected");
        }

        fileInfo->fi.direct_io = value->IsTrue() ? 1 : 0;
    }

    NAN_GETTER(FileInfo::GetKeepCache){
        FileInfo *fileInfo = ObjectWrap::Unwrap<FileInfo>(info.This());
        info.GetReturnValue().Set(fileInfo->fi.keep_cache ? Nan::True() : Nan::False());
    }

    NAN_SETTER(FileInfo::SetKeepCache){
        FileInfo *fileInfo = ObjectWrap::Unwrap<FileInfo>(info.This());

        if (!value->IsBoolean()) {
            // TODO: Check to see if this case will cause errors
            // FUSEJS_THROW_EXCEPTION("Invalid value type: ", "a Boolean was expected");
        }

        fileInfo->fi.keep_cache = value->IsTrue() ? 1 : 0;
    }

    NAN_GETTER(FileInfo::GetFlush){
        FileInfo *fileInfo = ObjectWrap::Unwrap<FileInfo>(info.This());
        info.GetReturnValue().Set( fileInfo->fi.flush ? Nan::True() : Nan::False());
    }

    NAN_GETTER(FileInfo::GetNonSeekable){     

#if FUSE_VERSION > 27 && !__APPLE__
        FileInfo *fileInfo = ObjectWrap::Unwrap<FileInfo>(info.This());
        info.GetReturnValue().Set(fileInfo->fi.nonseekable ? Nan::True() : Nan::False());
#else
        info.GetReturnValue().Set(Nan::False());
#endif
    }

    NAN_SETTER( FileInfo::SetNonSeekable){

        if (!value->IsBoolean()) {
            FUSEJS_THROW_EXCEPTION("Invalid value type: ", "a Boolean was expected");
        }
#if FUSE_VERSION > 28 && !__APPLE__
        FileInfo *fileInfo = ObjectWrap::Unwrap<FileInfo>(info.This());
        fileInfo->fi.nonseekable = value->IsTrue() ? 1 : 0;
#endif
    }

    NAN_SETTER(FileInfo::SetFileHandle){
        FileInfo *fileInfo = ObjectWrap::Unwrap<FileInfo>(info.This());

        if (!value->IsNumber()) {
            Nan::ThrowTypeError("Invalid value type: a Number was expected");        
        }

        fileInfo->fi.fh = Nan::To<uint32_t>(value).FromJust();
    }

    NAN_GETTER(FileInfo::GetFileHandle){
        FileInfo *fileInfo = ObjectWrap::Unwrap<FileInfo>(info.This());

        info.GetReturnValue().Set(Nan::New<Integer>(  (uint32_t) (fileInfo->fi.fh) ));
    }

    NAN_GETTER(FileInfo::GetLockOwner){
        FileInfo *fileInfo = ObjectWrap::Unwrap<FileInfo>(info.This());

        info.GetReturnValue().Set(Nan::New<Integer>(  (uint32_t) (fileInfo->fi.lock_owner) ));
    }
} //ends namespace NodeFuse
