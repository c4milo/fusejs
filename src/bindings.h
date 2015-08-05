/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */
#ifndef SRC_BINDINGS_H_
#define SRC_BINDINGS_H_
#include "node_fuse.h"
#include "filesystem.h"

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
            //static Handle<Value> Unmount(const Arguments& args);

        private:
            int multithreaded;
            int foreground;
            char* mountpoint;
            struct fuse_args* fargs;
            struct fuse_chan* channel;
            struct fuse_session* session;
            static Nan::Persistent<Function> constructor;
    };
}//namespace NodeFuse

#endif  // SRC_BINDINGS_H
