/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef SRC_REPLY_H_
#define SRC_REPLY_H_
#include "node_fuse.h"

namespace NodeFuse {
    class Reply : public Nan::ObjectWrap {
        friend class FileSystem;

        public:
            static void Initialize(Handle<Object> target);

            Reply();
            virtual ~Reply();
            static NAN_METHOD(New);
        
            void HookInterrupt();
            static void HandleInterrupt(fuse_req_t req, void *data);

            // moved to public so the static handler can access it
            bool b_hasReplied;
            bool b_wasInterrupted;
            fuse_req_t request;


            //Persistent<Function> b_interruptCallback;
            //static void CallInterruptHandler(uv_async_t *async_data);

        protected:
            static NAN_METHOD(Entry);
            static NAN_METHOD(Attributes);
            static NAN_METHOD(ReadLink);
            static NAN_METHOD(Error);
            static NAN_METHOD(Open);
            static NAN_METHOD(Buffer);
            static NAN_METHOD(Write);
            static NAN_METHOD(StatFs);
            static NAN_METHOD(Create);
            static NAN_METHOD(XAttributes);
            static NAN_METHOD(Lock);
            static NAN_METHOD(BMap);
            static NAN_METHOD(AddDirEntry);
            static NAN_METHOD(None);
            static NAN_GETTER(hasReplied);

            static NAN_GETTER(wasInterrupted);
        
//            static NAN_METHOD(RegisterInterruptHandler);
//            static NAN_GETTER(interruptCallback);
//            static NAN_SETTER(interruptCallback);
        

        private:
            size_t dentry_acc_size;
            size_t dentry_cur_length;
            size_t dentry_offset;
            size_t dentry_size;
            char* dentry_buffer;
            static Nan::Persistent<Function> constructor;

    };
} //namespace NodeFuse

#endif  // SRC_REPLY_H
