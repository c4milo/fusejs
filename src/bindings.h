// Copyright 2012, Camilo Aguilar. Cloudescape, LLC.
#ifndef SRC_BINDINGS_H_
#define SRC_BINDINGS_H_
#include "node_fuse.h"

namespace NodeFuse {

    class Fuse : public ObjectWrap {
        public:
            static void Initialize(Handle<Object> target);

            Fuse();
            virtual ~Fuse();
        protected:
            static Handle<Value> New(const Arguments& args);
            static Handle<Value> Mount(const Arguments& args);
            static Handle<Value> Umount(const Arguments& args);

        private:
    };

}//namespace NodeFuse

#endif  // SRC_BINDINGS_H
