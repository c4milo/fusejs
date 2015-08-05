// Copyright 2012, Camilo Aguilar. Cloudescape, LLC.
#ifndef SRC_FILE_INFO_H_
#define SRC_FILE_INFO_H_
#include "node_fuse.h"
//https://github.com/nodejs/nan/blob/4ed41d760313b648b4d212d6ff5374668757be4f/test/cpp/settemplate.cpp
namespace NodeFuse {
    class FileInfo : public ObjectWrap {
        friend class FileSystem;
        friend class Reply;

        public:
            static void Initialize(Handle<Object> target);
            static inline bool HasInstance(v8::Handle<v8::Value> value) {
                if (!value->IsObject()) {
                    return false;
                }
                // v8::Local<v8::Object> object = value->ToObject();
                // return HasInstance(constructor_template, object);
                return true;
            }

            FileInfo();
            virtual ~FileInfo();
            struct fuse_file_info *fi;
            static Nan::Persistent<v8::Function> constructor;
            static Nan::Persistent<v8::FunctionTemplate> constructor_template;
            static NAN_METHOD(New);
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
            static void GetLockOwner(v8::Local<v8::String> property,
                const Nan::PropertyCallbackInfo<v8::Value>& info);

    };
} //namespace NodeFuse

#endif  // SRC_FILE_INFO_H
