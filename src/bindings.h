// Copyright 2012, Camilo Aguilar. Cloudescape, LLC.
#ifndef SRC_BINDINGS_H_
#define SRC_BINDINGS_H_
#include "node_fuse.h"
#include "filesystem.h"

namespace NodeFuse {
    class Fuse : public ObjectWrap {
        public:
            static void Initialize(Handle<Object> target);

            Fuse();
            virtual ~Fuse();
        protected:
            static Handle<Value> New(const Arguments& args);
            static Handle<Value> Mount(const Arguments& args);
            static Handle<Value> Unmount(const Arguments& args);

        private:
            int multithreaded;
            int foreground;
            char *mountpoint;
            struct fuse_args *fargs;
            struct fuse_chan *channel;
            struct fuse_session *session;
            static Persistent<FunctionTemplate> constructor_template;
    };
}//namespace NodeFuse

#endif  // SRC_BINDINGS_H
