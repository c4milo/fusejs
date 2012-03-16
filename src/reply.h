// Copyright 2012, Camilo Aguilar. Cloudescape, LLC.
#ifndef SRC_REPLY_H_
#define SRC_REPLY_H_
#include "node_fuse.h"

namespace NodeFuse {
    class Reply : public ObjectWrap {
        friend class FileSystem;

        public:
            static void Initialize();

            Reply();
            virtual ~Reply();

        protected:
            static Handle<Value> Entry(const Arguments& args);
            static Handle<Value> Attributes(const Arguments& args);

        private:
            fuse_req_t request;
            static Persistent<FunctionTemplate> constructor_template;
    };
} //namespace NodeFuse

#endif  // SRC_REPLY_H
