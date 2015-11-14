/**
 * Loopback filesystem
 **/

"use strict";
const FileSystem = require('fusejs').FileSystem;
const PosixError = require('fusejs').PosixError;
const pth = require('path');
const fs = require('fs');
// since the inodes need to be unique, 
// we need to keep track of the inodes and it's associated path
const pathToInode = new Map();
const inodeToPath = new Map();
inodeToPath.set(1, '/');
pathToInode.set('/', 1);

// variable to store the loopback folder
// this will be set later
var loopbackFolder = "";

class LoopbackFS extends FileSystem {
    lookup(context, parentInode, name, reply) {

        // get the parent path
        const parent = inodeToPath.get(parentInode);

        // make sure it exists
        if (!parent) {
            console.log("ENOENT LOOKUP", parentInode, name)
            reply.err(PosixError.ENOENT);
            return;
        }

        // get the full folder path
        const localPath = pth.join(parent, name);
        const path = pth.join(loopbackFolder, parent, name);

        // get the file information 
        try {
            // use sync such that tests can pass. otherwise, use async
            var stat = fs.statSync(path);
        } catch (err) {
            // console.log(err);
            // console.log(path);
            if(!err.errno){
                var path2 = path;
                var idx =path2.indexOf('\u0000'); 
                while(idx >= 0){
                    path2 = path2.slice(idx,1);
                    idx = path2.indexOf('\u0000');                    
                    console.log(idx, path2);
                }
                stat = fs.statSync(path);
                return;
            }else{
                reply.err(-err.errno);
                return;
            }
        }
        // if(stat.size == 5678){
        //     console.log("name",name,"lookup size was 5678",stat);
        // }

        // check to see if the path has been visited before.
        // if not, add it to the map
        if (!pathToInode.has(localPath)) {
            // inode = next_largest_inode;
            pathToInode.set(localPath, stat.ino);
            inodeToPath.set(stat.ino, localPath);
        } 
        
        stat.inode = stat.ino;
        let entry = {
            inode: stat.ino,
            attr: stat,
            generation: 1 //some filesystems rely on this generation number, such as the  Network Filesystem
        };

        reply.entry(entry);


    }

    getattr(context, inode, reply) {
        const localPath = inodeToPath.get(inode);  
        if (localPath) {
            try {
                // use sync such that tests can pass. otherwise, use async
                var stat = fs.statSync(pth.join(loopbackFolder, localPath));

                if(inode != stat.ino){
                    if(inode == 1){
                        stat.ino = 1;
                    }else{
                        console.log("inode",inode,"path", localPath, "lookup size was 5678",stat);
                    }
                }
                stat.atime = Math.floor(stat.atime.getTime() / 1000);
                stat.mtime = Math.floor(stat.mtime.getTime() / 1000);
                stat.ctime = Math.floor(stat.ctime.getTime() / 1000);

                stat.inode = stat.ino;
                reply.attr(stat, 5); //5, timeout value, in seconds, for the validity of this inode. so 5 seconds
            } catch (err) {
                console.log("ENOENT GETATTR", inode, name);
                console.log(err);
                reply.err(err.errno);
                return;
            }


        } else {
            reply.err(PosixError.ENOENT);
        }
        return;
    }
    releasedir(context, inode, fileInfo, reply) {
        // console.log('Releasedir was called!');
        // console.log(fileInfo);
        reply.err(0);
    }

    opendir(context, inode, fileInfo, reply) {
        // console.log(fileInfo);
        reply.open(fileInfo);
    }


    readdir(context, inode, requestedSize, offset, fileInfo, reply) {
        //http://fuse.sourceforge.net/doxygen/structfuse__lowlevel__ops.html#af1ef8e59e0cb0b02dc0e406898aeaa51
        
        /*
        Read directory
        Send a buffer filled using reply.addDirEntry. Send an empty buffer on end of stream.
        fileInfo.fh will contain the value set by the opendir method, or will be undefined if the opendir method didn't set any value.
        Returning a directory entry from readdir() does not affect its lookup count.
        Valid replies: reply.addDirEntry reply.buffer, reply.err
        */

        /*
        size is the maximum memory size of the buffer for the underlying fuse
        filesystem. currently this cannot be determined a priori
        */

        const folder = inodeToPath.get(inode);
        if (!folder) {
            reply.err(PosixError.ENOENT);
            return;
        }

        const path = pth.join(loopbackFolder, folder);
        fs.readdir(path, function (err, files) {
            if (err) {
                reply.err(-err.errno);
                return;
            }

            const size = Math.max(requestedSize, files.length * 256);
            for (let file of files) {
   
                // use sync such that tests can pass. otherwise, use async
                try{
                    let attr = fs.statSync(pth.join(path, file));
                    // console.log(attr.atime,attr.atime.getTime());
    
                    attr.atime = Math.floor(attr.atime.getTime() / 1000);
                    attr.mtime = Math.floor(attr.mtime.getTime() / 1000);
                    attr.ctime = Math.floor(attr.ctime.getTime() / 1000);
                    attr.birthtime = Math.floor(attr.birthtime.getTime() / 1000);
                    attr.inode = attr.ino;
    
                    // keep track of new inodes
                    if (!inodeToPath.has(attr.ino)) {
                        inodeToPath.set(attr.ino, pth.join(folder, file));
                    }
                    reply.addDirEntry(file, size, attr, offset);
                }catch(err){
                    console.log("gettattr err", err);
                    continue;
                }
            }
            reply.buffer(new Buffer(0), requestedSize)

        });

    }

    open(context, inode, fileInfo, reply) {
        if (inode == 3) {
            reply.open(fileInfo);
            return;
        }
        if (inode < 3) {
            reply.err(PosixError.EISDIR);
            return;
        }

        reply.err(PosixError.ENOENT);

    }

    read(context, inode, len, offset, fileInfo, reply) {
        if (inode == 3) {
            const length = file_content.length
            const content = file_content.substr(offset, Math.min(length, offset + len));
            reply.buffer(new Buffer(content), content.length);
            return;
        }

        reply.err(PosixError.ENOENT);
        return;
    }

    release(context, inode, fileInfo, reply) {
        reply.err(0);
    }

};

function setLoopback(folder) {
    loopbackFolder = folder;
    console.log(folder);
}

module.exports = { LoopbackFS, setLoopback };
