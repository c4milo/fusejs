/**
 * Loopback filesystem
 **/
var FileSystem = require('../fuse').FileSystem;
var util = require('util');

var Loopback = function(fuse, options) {
    this.fuse = fuse;
    this.options = options;

    console.log(options);
    FileSystem.call(this);
};

util.inherits(Loopback, FileSystem);

(function() {
    /**
     * Initialize filesystem.
     * Called before any other filesystem method.
     *
     * param {Object} userData The user data passed to fuse.mount()
     * param {Object} connInfo Fuse connection information.
     *
     * There's no reply to this function.
     **/
    this.init = function(connInfo) {
        console.log(connInfo);
        console.log('Initializing Loopback filesystem!!');
        console.log(this.options);
    };

    /**
     * Clean up filesystem.
     * Called on filesystem exit.
     *
     * @param {Object} userData The user data passed to fuse.mount().
     *
     * There's no reply to this function.
     **/
    this.destroy = function(userData) {

    };

    /**
     * Look up a directory entry by name and get its attributes.
     *
     * @param {Request} request Request instance.
     * @param {Number} parent Inode number of the parent directory.
     * @param {String} name the name to look up.
     *
     * Valid replies: fuse.reply_entry() or fuse.reply_err().
     **/
    this.lookup = function(request, parent, name) {

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
     * @param {Request} request Request instance.
     * @param {Number} inode Inode number.
     * @param {Number} nlookup The number of lookups to forget.
     *
     * Valid replies: fuse.reply_none().
     **/
    this.forget = function(request, inode, nlookup) {

    };

    /**
     * Get file attributes
     *
     * @param {Request} request Request instance.
     * @param {Number} inode Inode number.
     *
     * Valid replies: fuse.reply_attr() or fuse.reply_err()
     **/
    this.getattr = function(request, inode) {

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
     * @param {Request} request Request instance.
     * @param {Number} inode Inode Number.
     * @param {Object} attr Same attributes as return them for
     * util.inspect(stats)
     * @param {Number} toSet Bit mask of attributes
     * which should be set.
     * @param {Object|Undefined} fileInfo File information.
     *
     * Valid replies: fuse.reply_attr() or fuse.reply_err()
     **/
    this.setattr = function(request, inode, attr, toSet, fileInfo) {

    };

    /**
     * Read symbolic link
     *
     * @param {Request} request Request instance.
     * @param {Number} inode Inode number.
     *
     * Valid replies: fuse.reply_readlink() or fuse.reply_err()
     *
     */
    this.readlink = function(request, inode) {

    };

    /**
     * Create file node
     * Create a regular file, character device,
     * block device, fifo or socket node.
     *
     *
     * @param {Request} request Request instance.
     * @param {Number} parent Inode number of the parent directory.
     * @param {String} name Name to be created.
     * @param {Number} mode File type and mode with which
     * to create the new file.
     * @param {Number} rdev The device number
     * (only valid if created file is a device)
     *
     * Valid replies: fuse.reply_entry() or fuse.reply_err()
     **/
    this.mknod = function(request, parent, name, mode, rdev) {

    };

    /**
     * Create a directory
     *
     * @param {Request} request Request instance.
     * @param {Number} parent Inode number of the parent directory.
     * @param {String} name Name to be created.
     * @param {Number} mode with which to create the new file.
     *
     * Valid replies: fuse.reply_entry() or fuse.reply_err()
     **/
    this.mkdir = function(request, parent, name, mode) {

    };

    /**
     * Remove a file
     *
     * @param {Request} request Request instance.
     * @param {Number} parent Inode number of the parent directory.
     * @param {String} name Name of the file to remove.
     *
     * Valid replies: fuse.reply_err()
     **/
    this.unlink = function(request, parent, name) {

    };

    /**
     * Remove a directory
     *
     * @param {Request} request Request instance.
     * @param {Number} parent Inode number of the parent directory.
     * @param {String} name Name of the directory to remove.
     *
     * Valid replies: fuse.reply_err()
     **/
    this.rmdir = function(request, parent, name) {

    };

    /**
     * Create a symbolic link
     *
     * @param {Request} request Request instance.
     * @param {String} link The contents of the symbolic link.
     * @param {Number} parent Inode number of the parent directory.
     * @param {String} name Name of the symbolic link to create.
     *
     * Valid replies: fuse.reply_entry() or fuse.reply_err()
     **/
    this.symlink = function(request, link, parent, name) {

    };

    this.rename = function() {

    };

    this.link = function() {

    };

    this.open = function() {

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
}).call(Loopback.prototype);

module.exports = Loopback;
