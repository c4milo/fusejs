var fuse = require('../fuse');

/**
 * Loopback filesystem operations.
 **/

module.exports = {
    /**
     * Initialize filesystem.
     * Called before any other filesystem method.
     *
     * param {Object} userData The user data passed to fuse.mount()
     * param {Object} connInfo Fuse connection information.
     *
     * There's no reply to this function.
     **/
    init: function(userData, connInfo) {

    },

    /**
     * Clean up filesystem.
     * Called on filesystem exit.
     *
     * @param {Object} userData The user data passed to fuse.mount().
     *
     * There's no reply to this function.
     **/
    destroy: function(userData) {

    },

    /**
     * Look up a directory entry by name and get its attributes.
     *
     * @param {Request} request Request instance.
     * @param {Number} parent Inode number of the parent directory.
     * @param {String} name the name to look up.
     *
     * Valid replies: fuse.reply_entry() or fuse.reply_err().
     **/
    lookup: function(request, parent, name) {

    },

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
    forget: function(request, inode, nlookup) {

    },

    /**
     * Get file attributes
     *
     * @param {Request} request Request instance.
     * @param {Number} inode Inode number.
     *
     * Valid replies: fuse.reply_attr() or fuse.reply_err()
     **/
    getattr: function(request, inode) {

    },

    /**
     * Set file attributes
     *
     * In the 'attr' argument only members
     * indicated by the 'to_set' bitmask contain
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
     * @param {Object} attrs Same attributes as util.inspect(stats)
     * @param {Number} toSet Bit mask of attributes
     * which should be set.
     * @param {Object|Undefined} fileInfo File information.
     *
     * Valid replies: fuse.reply_attr() or fuse.reply_err()
     **/
    setattr: function(request, inode, attrs, toSet, fileInfo) {

    },

    /**
     * Read symbolic link
     *
     * @param {Request} request Request instance.
     * @param {Number} inode Inode number.
     *
     * Valid replies: fuse.reply_readlink() or fuse.reply_err()
     *
     */
    readlink: function(request, inode) {

    },

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
    mknod: function(request, parent, name, mode, rdev) {

    },

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
    mkdir: function(request, parent, name, mode) {

    },

    /**
     * Remove a file
     *
     * @param {Request} request Request instance.
     * @param {Number} parent Inode number of the parent directory.
     * @param {String} name Name of the file to remove.
     *
     * Valid replies: fuse.reply_err()
     **/
    unlink: function(request, parent, name) {

    },

    /**
     * Remove a directory
     *
     * @param {Request} request Request instance.
     * @param {Number} parent Inode number of the parent directory.
     * @param {String} name Name of the directory to remove.
     *
     * Valid replies: fuse.reply_err()
     **/
    rmdir: function(request, parent, name) {

    },

    /**
     * Create a symbolic link
     *
     * @param {Request} request Request instance.
     * @param {String} link The contents of the symbolic link.
     * @param {Number} parent Inode number of the parent directory.
     * @param {String} name Name of the symbolic link to create.
     *
     * Valid replies: fuse_reply_entry fuse_reply_err*
     **/
    symlink: function(request, link, parent, name) {

    },

    rename: function() {

    },

    link: function() {

    },

    open: function() {

    },

    read: function() {

    },

    write: function() {

    },

    flush: function() {

    },

    release: function() {

    },

    fsync: function() {

    },

    opendir: function() {

    },

    readdir: function() {

    },

    releasedir: function() {

    },

    fsyncdir: function() {

    },

    statfs: function() {

    },

    setxattr: function() {

    },

    getxattr: function() {

    },

    listxattr: function() {

    },

    removexattr: function() {

    },

    access: function() {

    },

    create: function() {

    },

    getlk: function() {

    },

    setlk: function() {

    },

    bmap: function() {

    },

    ioctl: function() {

    },

    poll: function() {

    }
};
