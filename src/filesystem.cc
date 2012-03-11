#include "filesystem.h"
#include "bindings.h"

namespace NodeFuse {

    static struct fuse_lowlevel_ops fuse_ops = {
        init: FileSystem::Init,
        destroy: FileSystem::Destroy
    };

    static Persistent<String> init_sym = NODE_PSYMBOL("init");
    static Persistent<String> destroy_sym = NODE_PSYMBOL("destroy");

    void FileSystem::Init(void *userdata, struct fuse_conn_info *conn) {
        HandleScope scope;
        Fuse *fuse = static_cast<Fuse *>(userdata);

        Local<Value> vinit = fuse->fsobj->Get(init_sym);
        Local<Function> init = Local<Function>::Cast(vinit);

        TryCatch try_catch;

        init->Call(fuse->fsobj, 0, NULL);

        if (try_catch.HasCaught()) {
            FatalException(try_catch);
        }

    }

    void FileSystem::Destroy(void *userdata) {

    }

    struct fuse_lowlevel_ops* FileSystem::Operations() {
        return &fuse_ops;
    }
}


