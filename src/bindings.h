/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */
#ifndef SRC_BINDINGS_H_
#define SRC_BINDINGS_H_
#include "node_fuse.h"

namespace NodeFuse {
    class Fuse : public ObjectWrap {
        public:
            static void Initialize(Handle<Object> target);
            Nan::Persistent<Object> fsobj;

            Fuse();
            virtual ~Fuse();

        protected:
            static NAN_METHOD( New );
            static NAN_METHOD( Mount );
            static void RemoteMount(void* args);
            static void Unmount(const Nan::FunctionCallbackInfo<v8::Value>& args);

        private:
            int multithreaded;
            int foreground;
            char* mountpoint;
            uv_thread_t fuse_thread;
            struct fuse_session* session;
            struct fuse_args* fargs;
            struct fuse_chan* channel;
            static Nan::Persistent<Function> constructor;
    };
}//namespace NodeFuse

#endif  // SRC_BINDINGS_H
