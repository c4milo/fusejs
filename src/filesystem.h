/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef _SRC_FILESYSTEM_H_
#define _SRC_FILESYSTEM_H_
#include "node_fuse.h"
#include <stdlib.h>
#include <stdint.h>
#define _FUSE_OPS_LOOKUP_      0
#define _FUSE_OPS_GETATTR_     1
#define _FUSE_OPS_OPEN_        2
#define _FUSE_OPS_READ_        3
#define _FUSE_OPS_READDIR_     4
#define _FUSE_OPS_INIT_        5
#define _FUSE_OPS_DESTROY_     6
#define _FUSE_OPS_FORGET_      7
#define _FUSE_OPS_SETATTR_     8
#define _FUSE_OPS_READLINK_    9
#define _FUSE_OPS_MKNOD_       10
#define _FUSE_OPS_MKDIR_       11
#define _FUSE_OPS_UNLINK_      12
#define _FUSE_OPS_RMDIR_       13
#define _FUSE_OPS_SYMLINK_     14
#define _FUSE_OPS_RENAME_      15
#define _FUSE_OPS_LINK_        16
#define _FUSE_OPS_WRITE_       17
#define _FUSE_OPS_FLUSH_       18
#define _FUSE_OPS_RELEASE_     19
#define _FUSE_OPS_FSYNC_       20
#define _FUSE_OPS_OPENDIR_     21
#define _FUSE_OPS_RELEASEDIR_  22
#define _FUSE_OPS_FSYNCDIR_    23
#define _FUSE_OPS_STATFS_      24
#define _FUSE_OPS_SETXATTR_    25
#define _FUSE_OPS_GETXATTR_    26
#define _FUSE_OPS_LISTXATTR_   27
#define _FUSE_OPS_REMOVEXATTR_ 28
#define _FUSE_OPS_ACCESS_      29
#define _FUSE_OPS_CREATE_      30
#define _FUSE_OPS_GETLK_       31
#define _FUSE_OPS_SETLK_       32
#define _FUSE_OPS_BMAP_        33
#define _NUMBER_OF_FUSE_OPERATIONS_ 33

#define _FUSE_UNMOUNT_         34
#define __RING_SIZE__          1024
extern uv_async_t uv_async_handle;

#include "bindings.h"

namespace NodeFuse {
    struct fuse_cmd
    {
      int8_t op;
      fuse_req_t req;
      fuse_ino_t ino;
      fuse_ino_t newino; //used for renaming files
      size_t size;
      off_t off;
      dev_t dev;
      mode_t mode;
      const char* name;
      const char* newname; //used for renaming files, and for symlinking
      int to_set;
      struct fuse_conn_info conn;
      struct fuse_file_info fi;
      struct stat attr;
      void *userdata;
      unsigned long nlookup;
      #ifdef __APPLE__
      uint32_t position;
      #endif 
    };
    class FileSystem {
        private:
          static Persistent<Function> constructor;
        public:
            FileSystem();
            virtual ~FileSystem();

            static struct fuse_lowlevel_ops* GetOperations();
            static void Unmount(Fuse *fuse);
            static void Initialize(Handle<Object> target);
            static void DispatchOp(uv_async_t* handle, 
                      int status);

            static void Init(void* userdata,
                             struct fuse_conn_info* conn);
            static void Destroy(void* userdata);
            static void Lookup(fuse_req_t req,
                               fuse_ino_t parent,
                               const char* name);
            static void Forget(fuse_req_t req,
                                fuse_ino_t ino,
                                unsigned long nlookup);
            static void GetAttr(fuse_req_t req,
                                fuse_ino_t ino,
                                struct fuse_file_info* fi);
            static void SetAttr(fuse_req_t req,
                                fuse_ino_t ino,
                                struct stat* attr,
                                int to_set,
                                struct fuse_file_info* fi);
            static void ReadLink(fuse_req_t req, fuse_ino_t ino);
            static void MkNod(fuse_req_t req,
                              fuse_ino_t parent,
                              const char* name,
                              mode_t mode,
                              dev_t rdev);
            static void MkDir(fuse_req_t req,
                              fuse_ino_t parent,
                              const char *name,
                              mode_t mode);
            static void Unlink(fuse_req_t req,
                               fuse_ino_t parent,
                               const char* name);
            static void RmDir(fuse_req_t req,
                              fuse_ino_t parent,
                              const char* name);
            static void SymLink(fuse_req_t req,
                                const char* link,
                                fuse_ino_t parent,
                                const char* name);
            static void Rename(fuse_req_t req,
                               fuse_ino_t parent,
                               const char* name,
                               fuse_ino_t newparent,
                               const char* newname);
            static void Link(fuse_req_t req,
                             fuse_ino_t ino,
                             fuse_ino_t newparent,
                             const char* newname);
            static void Open(fuse_req_t req,
                             fuse_ino_t ino,
                             struct fuse_file_info* fi);
            static void Read(fuse_req_t req,
                             fuse_ino_t ino,
                             size_t size,
                             off_t off,
                             struct fuse_file_info* fi);
            static void Write(fuse_req_t req,
                              fuse_ino_t ino,
                              const char *buf,
                              size_t size,
                              off_t off,
                              struct fuse_file_info* fi);
            static void Flush(fuse_req_t req,
                              fuse_ino_t ino,
                              struct fuse_file_info* fi);
            static void Release(fuse_req_t req,
                                fuse_ino_t ino,
                                struct fuse_file_info* fi);
            static void FSync(fuse_req_t req,
                              fuse_ino_t ino,
                              int datasync,
                              struct fuse_file_info* fi);
            static void OpenDir(fuse_req_t req,
                                fuse_ino_t ino,
                                struct fuse_file_info* fi);
            static void ReadDir(fuse_req_t req,
                                fuse_ino_t ino,
                                size_t size,
                                off_t off,
                                struct fuse_file_info* fi);
            static void ReleaseDir(fuse_req_t req,
                                   fuse_ino_t ino,
                                   struct fuse_file_info* fi);
            static void FSyncDir(fuse_req_t req,
                                 fuse_ino_t ino,
                                 int datasync,
                                 struct fuse_file_info* fi);
            static void StatFs(fuse_req_t req, fuse_ino_t ino);
            static void SetXAttr(fuse_req_t req,
                                 fuse_ino_t ino,
                                 const char* name,
                                 const char* value,
                                 size_t size,
#ifdef __APPLE__
                                 int flags,
                                 uint32_t position);
#else
                                 int flags);

#endif
            static void GetXAttr(fuse_req_t req,
                                 fuse_ino_t ino,
                                 const char* name,
#ifdef __APPLE__
                                 size_t size,
                                 uint32_t position);
#else
                                 size_t size);
#endif
            static void ListXAttr(fuse_req_t req,
                                  fuse_ino_t ino,
                                  size_t size);
            static void RemoveXAttr(fuse_req_t req,
                                    fuse_ino_t ino,
                                    const char* name);
            static void Access(fuse_req_t req,
                               fuse_ino_t ino,
                               int mask);
            static void Create(fuse_req_t req,
                               fuse_ino_t parent,
                               const char* name,
                               mode_t mode,
                               struct fuse_file_info* fi);
            static void GetLock(fuse_req_t req,
                              fuse_ino_t ino,
                              struct fuse_file_info* fi,
                              struct flock* lock);
            static void SetLock(fuse_req_t req,
                              fuse_ino_t ino,
                              struct fuse_file_info* fi,
                              struct flock* lock,
                              int sleep);
            static void BMap(fuse_req_t req,
                             fuse_ino_t ino,
                             size_t blocksize,
                             uint64_t idx);
            static void IOCtl(fuse_req_t req,
                              fuse_ino_t ino,
                              int cmd,
                              void* arg,
                              struct fuse_file_info* fi,
                              unsigned* flagsp,
                              const void* in_buf,
                              size_t in_bufsz,
                              size_t out_bufszp);
            static void Poll(fuse_req_t req,
                             fuse_ino_t ino,
                             struct fuse_file_info* fi,
                             struct fuse_pollhandle* ph);
            static void RemoteInitialize();

            static void RemoteInit(void* userdata,
                             struct fuse_conn_info conn);
            static void RemoteDestroy(void* userdata);
            static void RemoteLookup(fuse_req_t req,
                               fuse_ino_t parent,
                               const char* name);
            static void RemoteForget(fuse_req_t req,
                                fuse_ino_t ino,
                                unsigned long nlookup);
            static void RemoteGetAttr(fuse_req_t req,
                                fuse_ino_t ino,
                                struct fuse_file_info fi);
            static void RemoteSetAttr(fuse_req_t req,
                                fuse_ino_t ino,
                                struct stat attr,
                                int to_set,
                                struct fuse_file_info fi);
            static void RemoteReadLink(fuse_req_t req, fuse_ino_t ino);
            static void RemoteMkNod(fuse_req_t req,
                              fuse_ino_t parent,
                              const char* name,
                              mode_t mode,
                              dev_t rdev);
            static void RemoteMkDir(fuse_req_t req,
                              fuse_ino_t parent,
                              const char *name,
                              mode_t mode);
            static void RemoteUnlink(fuse_req_t req,
                               fuse_ino_t parent,
                               const char* name);
            static void RemoteRmDir(fuse_req_t req,
                              fuse_ino_t parent,
                              const char* name);
            static void RemoteSymLink(fuse_req_t req,
                                const char* link,
                                fuse_ino_t parent,
                                const char* name);
            static void RemoteRename(fuse_req_t req,
                               fuse_ino_t parent,
                               const char* name,
                               fuse_ino_t newparent,
                               const char* newname);
            static void RemoteLink(fuse_req_t req,
                             fuse_ino_t ino,
                             fuse_ino_t newparent,
                             const char* newname);
            static void RemoteOpen(fuse_req_t req,
                             fuse_ino_t ino,
                             struct fuse_file_info fi);
            static void RemoteRead(fuse_req_t req,
                             fuse_ino_t ino,
                             size_t size,
                             off_t off,
                             struct fuse_file_info fi);
            static void RemoteWrite(fuse_req_t req,
                              fuse_ino_t ino,
                              const char *buf,
                              size_t size,
                              off_t off,
                              struct fuse_file_info fi);
            static void RemoteFlush(fuse_req_t req,
                              fuse_ino_t ino,
                              struct fuse_file_info fi);
            static void RemoteRelease(fuse_req_t req,
                                fuse_ino_t ino,
                                struct fuse_file_info fi);
            static void RemoteFSync(fuse_req_t req,
                              fuse_ino_t ino,
                              int datasync,
                              struct fuse_file_info fi);
            static void RemoteOpenDir(fuse_req_t req,
                                fuse_ino_t ino,
                                struct fuse_file_info fi);
            static void RemoteReadDir(fuse_req_t req,
                                fuse_ino_t ino,
                                size_t size,
                                off_t off,
                                struct fuse_file_info fi);
            static void RemoteReleaseDir(fuse_req_t req,
                                   fuse_ino_t ino,
                                   struct fuse_file_info fi);
            static void RemoteFSyncDir(fuse_req_t req,
                                 fuse_ino_t ino,
                                 int datasync,
                                 struct fuse_file_info fi);
            static void RemoteStatFs(fuse_req_t req, fuse_ino_t ino);
            static void RemoteSetXAttr(fuse_req_t req,
                                 fuse_ino_t ino,
                                 const char* name,
                                 const char* value,
                                 size_t size,
#ifdef __APPLE__
                                 int flags,
                                 uint32_t position);
#else
                                 int flags);

#endif
            static void RemoteGetXAttr(fuse_req_t req,
                                 fuse_ino_t ino,
                                 const char* name,
#ifdef __APPLE__
                                 size_t size,
                                 uint32_t position);
#else
                                 size_t size);
#endif
            static void RemoteListXAttr(fuse_req_t req,
                                  fuse_ino_t ino,
                                  size_t size);
            static void RemoteRemoveXAttr(fuse_req_t req,
                                    fuse_ino_t ino,
                                    const char* name);
            static void RemoteAccess(fuse_req_t req,
                               fuse_ino_t ino,
                               int mask);
            static void RemoteCreate(fuse_req_t req,
                               fuse_ino_t parent,
                               const char* name,
                               mode_t mode,
                               struct fuse_file_info fi);
            static void RemoteGetLock(fuse_req_t req,
                              fuse_ino_t ino,
                              struct fuse_file_info fi,
                              struct flock* lock);
            static void RemoteSetLock(fuse_req_t req,
                              fuse_ino_t ino,
                              struct fuse_file_info fi,
                              struct flock* lock,
                              int sleep);
            static void RemoteBMap(fuse_req_t req,
                             fuse_ino_t ino,
                             size_t blocksize,
                             uint64_t idx);
            static void RemoteIOCtl(fuse_req_t req,
                              fuse_ino_t ino,
                              int cmd,
                              void* arg,
                              struct fuse_file_info fi,
                              unsigned* flagsp,
                              const void* in_buf,
                              size_t in_bufsz,
                              size_t out_bufszp);
            static void RemotePoll(fuse_req_t req,
                             fuse_ino_t ino,
                             struct fuse_file_info fi,
                             struct fuse_pollhandle* ph);            
    };
}//namespace NodeFuse

#endif  // SRC_FILESYSTEM_H
