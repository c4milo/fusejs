// Copyright 2012, Camilo Aguilar. Cloudescape, LLC.
#ifndef SRC_FILE_INFO_H_
#define SRC_FILE_INFO_H_
#include "node_fuse.h"

namespace NodeFuse {
    class FileInfo : public ObjectWrap {
        friend class FileSystem;
        friend class Reply;

        public:
            static void Initialize();
            static inline bool HasInstance(v8::Handle<v8::Value> value) {
                if (!value->IsObject()) {
                    return false;
                }
                v8::Local<v8::Object> object = value->ToObject();
                return NanHasInstance(constructor_template, object);
            }

            FileInfo();
            virtual ~FileInfo();
            struct fuse_file_info *fi;
            static Persistent<FunctionTemplate> constructor_template;

        protected:
            static NAN_GETTER(GetFlags);
            static NAN_GETTER(GetWritePage);
            static NAN_GETTER(GetDirectIO);
            static NAN_SETTER(SetDirectIO);
            static NAN_GETTER(GetKeepCache);
            static NAN_SETTER(SetKeepCache);
            static NAN_GETTER(GetFlush);
            static NAN_GETTER(GetNonSeekable);
            static NAN_SETTER(SetNonSeekable);
            static NAN_SETTER(SetFileHandle);
            static NAN_GETTER(GetFileHandle);
            static NAN_GETTER(GetLockOwner);

    };
} //namespace NodeFuse

#endif  // SRC_FILE_INFO_H
