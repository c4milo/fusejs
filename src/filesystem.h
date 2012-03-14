#ifndef SRC_FILESYSTEM_H_
#define SRC_FILESYSTEM_H_
#include "node_fuse.h"

namespace NodeFuse {
    class FileSystem {
        public:
            FileSystem();
            virtual ~FileSystem();

            static struct fuse_lowlevel_ops* GetOperations();
            static void Init(void *userdata, struct fuse_conn_info *conn);
            static void Destroy(void *userdata);
            static void Lookup(fuse_req_t req, fuse_ino_t parent, const char *name);
            //static void Forget(fuse_req_t req, fuse_ino_t ino, unsigned long nlookup);
            //static void GetAttr(fuse_req_t req, fuse_ino_t ino, struct fuse_file_info *fi);
    };
}//namespace NodeFuse

#endif  // SRC_FILESYSTEM_H
