var bindings = require('./bindings');

var FileSystem = function() {

};

(function() {
     /**
     * Initialize filesystem.
     * Called before any other filesystem method.
     *
     * param {Object} connInfo Fuse connection information.
     *
     * There's no reply to this function.
     **/
    this.init = function(connInfo) {

    };

    /**
     * Clean up filesystem.
     * Called on filesystem exit.
     *
     * There's no reply to this function.
     **/
    this.destroy = function() {

    };

    /**
     * Look up a directory entry by name and get its attributes.
     *
     * @param {Object} context Context info of the calling process.
     * @param {Number} parent Inode number of the parent directory.
     * @param {String} name the name to look up.
     * @param {Function} reply Function to reply to this operation.
     *
     * Valid replies: fuse.reply_entry() or fuse.reply_err().
     **/
    this.lookup = function(context, parent, name, reply) {

    };

    /**
     * Forget about an inode
     * The nlookup parameter indicates the number of
     * lookups previously performed on this inode.
     *
     * If the filesystem implements inode lifetimes,
     * it is recommended that inodes acquire a single
     * reference on each lookup, and lose nlookup references
     * on each forget.
     *
     * The filesystem may ignore forget calls, if the inodes
     * don't need to have a limited lifetime.
     *
     * On unmount it is not guaranteed, that all referenced
     * inodes will receive a forget message.
     *
     * @param {Object} context Context info of the calling process.
     * @param {Number} inode Inode number.
     * @param {Number} nlookup The number of lookups to forget.
     *
     * Valid replies: fuse.reply_none().
     **/
    this.forget = function(context, inode, nlookup) {

    };

    /**
     * Get file attributes
     *
     * @param {Object} context Context info of the calling process.
     * @param {Number} inode Inode number.
     *
     * Valid replies: fuse.reply_attr() or fuse.reply_err()
     **/
    this.getattr = function(context, inode) {

    };

    /**
     * Set file attributes
     *
     * In the 'attr' argument only members
     * indicated by the 'toSet' bitmask contain
     * valid values. Other members contain undefined values.
     *
     * If the setattr was invoked from the ftruncate() system call
     * under Linux kernel versions 2.6.15 or later,
     * the fi->fh will contain the value set by the open
     * method or will be undefined if the open method
     * didn't set any value. Otherwise (not ftruncate
     * call, or kernel version earlier than 2.6.15)
     * the fi parameter will be NULL.
     *
     * @param {Object} context Context info of the calling process.
     * @param {Number} inode Inode Number.
     * @param {Object} attr Same attributes as return them for
     * util.inspect(stats).
     * @param {Number} toSet Bit mask of attributes
     * which should be set.
     * @param {Object|Undefined} fileInfo File information.
     *
     * Valid replies: fuse.reply_attr() or fuse.reply_err()
     **/
    this.setattr = function(context, inode, attr, toSet, fileInfo) {

    };

    /**
     * Read symbolic link
     *
     * @param {Object} context Context info of the calling process.
     * @param {Number} inode Inode number.
     *
     * Valid replies: fuse.reply_readlink() or fuse.reply_err()
     **/
    this.readlink = function(context, inode) {

    };

    /**
     * Create file node
     * Create a regular file, character device,
     * block device, fifo or socket node.
     *
     * @param {Object} context Context info of the calling process.
     * @param {Number} parent Inode number of the parent directory.
     * @param {String} name Name to be created.
     * @param {Number} mode File type and mode with which
     * to create the new file.
     * @param {Number} rdev The device number
     * (only valid if created file is a device).
     *
     * Valid replies: fuse.reply_entry() or fuse.reply_err()
     **/
    this.mknod = function(context, parent, name, mode, rdev) {

    };

    /**
     * Create a directory
     *
     * @param {Object} context Context info of the calling process.
     * @param {Number} parent Inode number of the parent directory.
     * @param {String} name Name to be created.
     * @param {Number} mode with which to create the new file.
     *
     * Valid replies: fuse.reply_entry() or fuse.reply_err()
     **/
    this.mkdir = function(context, parent, name, mode) {

    };

    /**
     * Remove a file
     *
     * @param {Object} context Context info of the calling process.
     * @param {Number} parent Inode number of the parent directory.
     * @param {String} name Name of the file to remove.
     *
     * Valid replies: fuse.reply_err()
     **/
    this.unlink = function(context, parent, name) {

    };

    /**
     * Remove a directory
     *
     * @param {Object} context Context info of the calling process.
     * @param {Number} parent Inode number of the parent directory.
     * @param {String} name Name of the directory to remove.
     *
     * Valid replies: fuse.reply_err()
     **/
    this.rmdir = function(context, parent, name) {

    };

    /**
     * Create a symbolic link
     *
     * @param {Object} context Context info of the calling process.
     * @param {String} link The contents of the symbolic link.
     * @param {Number} parent Inode number of the parent directory.
     * @param {String} name Name of the symbolic link to create.
     *
     * Valid replies: fuse.reply_entry() or fuse.reply_err()
     **/
    this.symlink = function(context, link, parent, name) {

    };

    /**
     * Rename a file
     *
     * @param {Object} context Context info of the calling process.
     * @param {Number} parent Inode number of the old parent directory.
     * @param {String} name Old name.
     * @param {Number} newparent Inode number of the new parent directory.
     * @param {String} newname New name.
     *
     * Valid replies: fuse.reply_err()
     **/
    this.rename = function(context, parent, name, newparent, newname) {

    };

    /**
     * Create a hard link
     *
     * @param {Object} context Context info of the calling process.
     * @param {Number} inode The old inode number.
     * @param {Number} newparent Inode number of the new parent directory.
     * @param {String} newname New name to create.
     *
     * Valid replies: fuse.reply_entry() or fuse.reply_err()
     **/
    this.link = function(context, inode, newparent, newname) {

    };


    /**
     * Open a file
     *
     * Open flags (with the exception of O_CREAT, O_EXCL, O_NOCTTY and O_TRUNC)
     * are available in fileInfo.flags.
     *
     * Filesystem may store an arbitrary file handle (pointer, index, etc)
     * in fileInfo.fh, and use this in other all other file operations
     * (read, write, flush, release, fsync).
     *
     * Filesystem may also implement stateless file I/O and not store
     * anything in fileInfo.fh.
     *
     * There are also some flags (direct_io, keep_cache) which the
     * filesystem may set in fileInfo, to change the way the file is
     * opened. See fuse_file_info structure in <fuse_common.h> for more details.
     *
     * @param {Object} context Context info of the calling process.
     * @param {Number} inode The inode number.
     * @param {Object} fileInfo File information.
     *
     * Valid replies: fuse.reply_open() or fuse.reply_err()
     **/
    this.open = function(context, inode, fileInfo) {

    };

    this.read = function() {

    };

    this.write = function() {

    };

    this.flush = function() {

    };

    this.release = function() {

    };

    this.fsync = function() {

    };

    this.opendir = function() {

    };

    this.readdir = function() {

    };

    this.releasedir = function() {

    };

    this.fsyncdir = function() {

    };

    this.statfs = function() {

    };

    this.setxattr = function() {

    };

    this.getxattr = function() {

    };

    this.listxattr = function() {

    };

    this.removexattr = function() {

    };

    this.access = function() {

    };

    this.create = function() {

    };

    this.getlk = function() {

    };

    this.setlk = function() {

    };

    this.bmap = function() {

    };

    this.ioctl = function() {

    };

    this.poll = function() {

    };
}).call(FileSystem.prototype);

module.exports = {
    fuse: new bindings.Fuse(),
    FileSystem: FileSystem
};
