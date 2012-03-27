// Copyright 2012, Camilo Aguilar. Cloudescape, LLC.
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

    void FileInfo::Initialize() {
        Local<FunctionTemplate> t = FunctionTemplate::New();
        Local<ObjectTemplate> object_tmpl = t->InstanceTemplate();

        object_tmpl->SetInternalFieldCount(1);

        //SetAccessors
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

    Handle<Value> GetFlags(Local<String> property, const AccessorInfo& info) {

    }

    Handle<Value> GetWritePage(Local<String> property, const AccessorInfo& info) {

    }

    Handle<Value> GetDirectIO(Local<String> property, const AccessorInfo& info) {

    }

    void SetDirectIO(Local<String> property, Local<Value> value, const AccessorInfo& info) {

    }

    Handle<Value> GetKeepCache(Local<String> property, const AccessorInfo& info) {

    }

    void SetKeepCache(Local<String> property, Local<Value> value, const AccessorInfo& info) {

    }

    Handle<Value> GetFlush(Local<String> property, const AccessorInfo& info) {

    }

    Handle<Value> GetNonSeekable(Local<String> property, const AccessorInfo& info) {

    }

    void SetNonSeekable(Local<String> property, Local<Value> value, const AccessorInfo& info) {

    }

    void SetFileHandle(Local<String> property, Local<Value> value, const AccessorInfo& info) {

    }

    Handle<Value> GetFileHandle(Local<String> property, const AccessorInfo& info) {

    }

    Handle<Value> GetLockOwner(Local<String> property, const AccessorInfo& info) {

    }
} //ends namespace NodeFuse
