#include "filesystem.h"

namespace NodeFuse {

    static struct fuse_lowlevel_ops fuse_ops = {
        init: FileSystem::Init,
        destroy: FileSystem::Destroy
    };

    void FileSystem::Init(void *userdata, struct fuse_conn_info *conn) {

    }

    void FileSystem::Destroy(void *userdata) {

    }

    struct fuse_lowlevel_ops* FileSystem::Operations() {
        return &fuse_ops;
    }
}


