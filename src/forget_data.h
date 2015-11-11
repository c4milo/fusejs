/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef SRC_FORGET_DATA_H_
#define SRC_FORGET_DATA_H_

#include "node_fuse.h"

#if FUSE_VERSION > 28 

namespace NodeFuse {
    class ForgetData : public ObjectWrap {
        friend class FileSystem;
        public:
            static void Initialize(Handle<Object> target);

            ForgetData();
            virtual ~ForgetData();
            struct fuse_forget_data *fd;
            static Nan::Persistent<v8::Function> constructor;
            static Nan::Persistent<v8::FunctionTemplate> constructor_template;
            static NAN_METHOD(New);
        protected:
            static void GetIno(v8::Local<v8::String> property,
                const Nan::PropertyCallbackInfo<v8::Value>& info) ;
            static void GetNLookup(v8::Local<v8::String> property,
                const Nan::PropertyCallbackInfo<v8::Value>& info) ;


    };
} //namespace NodeFuse


#endif // if FUSE_VERSION

#endif  // ifdef SRC_FILE_INFO_H
