// Copyright 2012, Camilo Aguilar. Cloudescape, LLC.
#ifndef SRC_FILE_INFO_H_
#define SRC_FILE_INFO_H_
#include "node_fuse.h"

namespace NodeFuse {
    class FileInfo : public ObjectWrap {
        friend class FileSystem;

        public:
            static void Initialize();

            FileInfo();
            virtual ~FileInfo();

        protected:
            static Handle<Value> GetFlags(Local<String> property, const AccessorInfo& info);
            static Handle<Value> GetWritePage(Local<String> property, const AccessorInfo& info);
            static Handle<Value> GetDirectIO(Local<String> property, const AccessorInfo& info);
            static void SetDirectIO(Local<String> property, Local<Value> value, const AccessorInfo& info);
            static Handle<Value> GetKeepCache(Local<String> property, const AccessorInfo& info);
            static void SetKeepCache(Local<String> property, Local<Value> value, const AccessorInfo& info);
            static Handle<Value> GetFlush(Local<String> property, const AccessorInfo& info);
            static Handle<Value> GetNonSeekable(Local<String> property, const AccessorInfo& info);
            static void SetNonSeekable(Local<String> property, Local<Value> value, const AccessorInfo& info);
            static void SetFileHandle(Local<String> property, Local<Value> value, const AccessorInfo& info);
            static Handle<Value> GetFileHandle(Local<String> property, const AccessorInfo& info);
            static Handle<Value> GetLockOwner(Local<String> property, const AccessorInfo& info);

        private:
            struct fuse_file_info *fi;
            static Persistent<FunctionTemplate> constructor_template;
    };
} //namespace NodeFuse

#endif  // SRC_FILE_INFO_H
