/**
 * Loopback filesystem
 **/
var FileSystem = require('../fuse').FileSystem;
var PosixError = require('../fuse').PosixError;

var util = require('util');

var Loopback = function(fuse, options) {
    this.fuse = fuse;
    this.options = options;

    //console.log(options);
    FileSystem.call(this);
};

util.inherits(Loopback, FileSystem);

(function() {
    this.init = function(connInfo) {
        console.log(connInfo);
        console.log('Initializing Loopback filesystem!!');
        console.log(this.options);
    };

    this.destroy = function() {
        console.log('Cleaning up filesystem...');
    };

    this.lookup = function(context, parent, name, reply) {
        console.log('Lookup!');
        console.log(context);
        console.log('Name -> ' + name);
        var entry = {
            inode: 1234,
            generation: 2,
            attr: {
                dev: 234881026,
                ino: 13420595,
                mode: 33188,
                nlink: 1,
                uid: 501,
                gid: 20,
                rdev: 0,
                size: 11,
                blksize: 4096,
                blocks: 8,
                atime: 1331780451475, //Date.now();
                mtime: 1331780451475, //Date.now();
                ctime: 1331780451475, //Date.now();
            },
            attr_timeout: 30, //in seconds
            entry_timeout: 60 //in seconds
        };

        reply.entry(entry);
        //reply.err(PosixError.ENOENT);
    };

    this.forget = function(context, inode, nlookup) {
        console.log('Forget was called!!');
    };

    this.getattr = function(context, inode, reply) {
        console.log('Getattr was called!!');
        console.log(context);
        console.log(inode);

        var hello = "Hello World!\n";
        //stat object
        var attrs = {
            //dev: 0,
            ino: 1,
            mode: 16877,
            nlink: 1,
            //uid: 501,
            //gid: 20,
            //rdev: 0,
            size: hello.length,
            //blksize: 4096,
            //blocks: 8,
            //atime: 1331780451475, //Date.now();
            //mtime: 1331780451475, //Date.now();
            //ctime: 1331780451475, //Date.now();
        };
        reply.attr(attrs, 1000);
        //reply.err(PosixError.EIO);
    };

    this.setattr = function(context, inode, attrs, reply) {
        console.log('Setattr was called!!');
        console.log(attrs);

        //reply.attr(attrs, 1000);
        reply.err(PosixError.EIO);
    };

    this.readlink = function(context, inode, reply) {
        console.log('Readlink was called!');
        //reply.readlink('eso');
        reply.err(PosixError.EIO);
    };

    this.mknod = function(context, parent, name, mode, rdev, reply) {
        console.log('Mknod was called!');
        reply.err(PosixError.ENOENT);
        //reply.entry(entry);
    };

    this.mkdir = function(context, parent, name, mode, reply) {
        console.log('Mkdir was called!');
        reply.err(PosixError.EIO);
        //reply.entry(entry);
    };

    this.unlink = function(context, parent, name, reply) {
        console.log('Unlink was called!');
        reply.err(PosixError.EIO);
    };

    this.rmdir = function(context, parent, name, reply) {
        console.log('Rmdir was called!');
        reply.err(0);
    };

    this.symlink = function(context, parent, link, name, reply) {
        console.log('Symlink was called!');
        reply.err(0);
        //reply.entry(entry);
    };

    this.rename = function(context, parent, name, newParent, newName, reply) {
        console.log('Rename was called!');
        reply.err(0);
        //reply.err(PosixError.EIO);
    };

    this.link = function(context, inode, newParent, newName, reply) {
        console.log('Link was called!');
        reply.err(PosixError.EIO);
        //reply.entry(entry);
    };

    this.open = function(context, inode, fileInfo, reply) {
        console.log('Open was called!');
        //reply.err(0);
        reply.open(fileInfo);
    };

    this.read = function(context, inode, size, offset, fileInfo, reply) {
        console.log('Read was called!');
        reply.buffer(new Buffer('hellow world'));
        //reply.err(0);
    };

    this.write = function(context, inode, buffer, offset, fileInfo, reply) {
        console.log('Write was called!');
        console.log('Writing ' + buffer);
        reply.write(buffer.length);
        //reply.err(0);
    };

    this.flush = function(context, inode, fileInfo, reply) {
        console.log('Flush was called!');
        //console.log(fileInfo);
        reply.err(0);
    };

    this.release = function(context, inode, fileInfo, reply) {
        console.log('Release was called!');
        reply.err(0);
    };

    //if datasync is true then only user data is flushed, not metadata
    this.fsync = function(context, inode, datasync, fileInfo, reply) {
        console.log('Fsync was called!');
        console.log('datasync -> ' + datasync);
        reply.err(0);
    };

    this.opendir = function(context, inode, fileInfo, reply) {
        console.log('Opendir was called!');
        //reply.err(0);
        reply.open(fileInfo);
    };

    this.readdir = function(context, inode, size, offset, fileInfo, reply) {
        console.log('Readdir was called!');
        console.log('Readdir Size ---> ' + size);
        var entries = ['.', '..', 'dir1', 'dir2'];
        for (var i = 0, len = entries.length; i < len; i++) {
          var attrs = {};
          attrs.inode = i;
          reply.addDirEntry(entries[i], size, attrs, offset + i);
        }

        //signals end of entries, this is required or
        //it will block the user
        reply.buffer(new Buffer(''));
    };

    this.releasedir = function(context, inode, fileInfo, reply) {
        console.log('Releasedir was called!');
        console.log(fileInfo);
        reply.err(0);
    };

    //if datasync is true then only directory contents is flushed, not metadata
    this.fsyncdir = function(context, inode, datasync, fileInfo, reply) {
        console.log('FsyncDir was called!');
        console.log('datasync -> ' + datasync);
        reply.err(0);
    };

    this.statfs = function(context, inode, reply) {
        console.log('Statfs was called!');

        var statvfs = {
            bsize: 1024, /* file system block size */
            frsize: 0, /* fragment size */
            blocks: 0, /* size of fs in f_frsize units */
            bfree: 0, /* # free blocks */
            bavail: 0, /* # free blocks for unprivileged users */
            files: 5, /* # inodes */
            ffree: 2, /* # free inodes */
            favail: 2, /* # free inodes for unprivileged users */
            fsid: 4294967295, /* file system ID */
            flag: 0, /* mount flags */
            namemax: 1.7976931348623157e+308 /* maximum filename length */
        };

        reply.statfs(statvfs);
    };

    this.setxattr = function(context, inode, name, value, size, flags, position, reply) {
        console.log('SetXAttr was called!');
        console.log('Attr name -> ' + name);
        reply.err(0);
    };

    this.getxattr = function(context, inode, name, size, position, reply) {
        console.log('GetXAttr was called!');
        console.log('Extended attribute name -> ' + name);
        reply.err(0);
        //reply.xattr(1024); //needed buffer size
    };

    this.listxattr = function(context, inode, size, reply) {
        console.log('ListXAttr was called!');
        console.log(size);
        reply.err(0);
        //reply.buffer(new Buffer('list,of,extended,attributes'));
        //reply.xattr(1024);
    };

    this.removexattr = function(context, inode, name, reply) {
        console.log('RemoveXAttr was called!');
        console.log(name);
        reply.err(0);
    };

    this.access = function(context, inode, mask, reply) {
        console.log('Access was called!');
        reply.err(0);
    };

    this.create = function(context, parent, name, mode, fileInfo, reply) {
        console.log('Create was called!');
        console.log('Create -> ' + name);
        //reply.create({});
        reply.err(0);
    };

    this.getlk = function(context, inode, fileInfo, lock, reply) {
        console.log('GetLock was called!');
        console.log('Lock -> ' + lock);
        //reply.lock(lock);
        reply.err(0);
    };

    this.setlk = function(context, inode, fileInfo, lock, sleep, reply) {
        console.log('SetLock was called!!');
        console.log('Lock -> ' + lock);
        console.log('sleep -> ' + sleep);
        reply.err(0);
    };

    this.bmap = function(context, inode, blocksize, index, reply) {
        console.log('BMap was called!');
        //reply.err(0);
        reply.bmap(12344);
    };

    this.ioctl = function() {

    };

    this.poll = function() {

    };
}).call(Loopback.prototype);

module.exports = Loopback;
