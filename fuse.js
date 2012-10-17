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
     * @param {Object} reply Reply instance.
     *
     * Valid replies: reply.entry() or reply.err().
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
     **/
    this.forget = function(context, inode, nlookup) {

    };

    /**
     * Get file attributes
     *
     * @param {Object} context Context info of the calling process.
     * @param {Number} inode Inode number.
     * @param {Object} reply Reply instance.
     *
     * Valid replies: reply.attr() or reply.err()
     **/
    this.getattr = function(context, inode, reply) {

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
     * @param {Object} attrs Attributes to be set.
     * @param {Object} reply Reply instance.
     *
     * Valid replies: reply.attr() or reply.err();
     **/
    this.setattr = function(context, inode, attrs, reply) {

    };

    /**
     * Read symbolic link
     *
     * @param {Object} context Context info of the calling process.
     * @param {Number} inode Inode number.
     * @param {Object} reply Reply instance.
     *
     * Valid replies: reply.readlink() or reply.err()
     **/
    this.readlink = function(context, inode, reply) {

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
     * @param {Object} reply Reply instance.
     *
     * Valid replies: reply.entry() or reply.err()
     **/
    this.mknod = function(context, parent, name, mode, rdev, reply) {

    };

    /**
     * Create a directory
     *
     * @param {Object} context Context info of the calling process.
     * @param {Number} parent Inode number of the parent directory.
     * @param {String} name Name to be created.
     * @param {Number} mode with which to create the new file.
     * @param {Object} reply Reply instance.
     *
     * Valid replies: reply.entry() or reply.err()
     **/
    this.mkdir = function(context, parent, name, mode, reply) {

    };

    /**
     * Remove a file
     *
     * @param {Object} context Context info of the calling process.
     * @param {Number} parent Inode number of the parent directory.
     * @param {String} name Name of the file to remove.
     * @param {Object} reply Reply instance.
     *
     * Valid replies: reply.err()
     **/
    this.unlink = function(context, parent, name, reply) {

    };

    /**
     * Remove a directory
     *
     * @param {Object} context Context info of the calling process.
     * @param {Number} parent Inode number of the parent directory.
     * @param {String} name Name of the directory to remove.
     *
     * Valid replies: reply.err()
     **/
    this.rmdir = function(context, parent, name, reply) {

    };

    /**
     * Create a symbolic link
     *
     * @param {Object} context Context info of the calling process.
     * @param {Number} parent Inode number of the parent directory.
     * @param {String} link The contents of the symbolic link.
     * @param {String} name Name of the symbolic link to create.
     * @param {Object} reply Reply instance.
     *
     * Valid replies: reply.entry() or reply.err()
     **/
    this.symlink = function(context, parent, link, name, reply) {

    };

    /**
     * Rename a file
     *
     * @param {Object} context Context info of the calling process.
     * @param {Number} parent Inode number of the old parent directory.
     * @param {String} name Old name.
     * @param {Number} newParent Inode number of the new parent directory.
     * @param {String} newName New name.
     * @param {Object} reply Reply instance.
     *
     * Valid replies: reply.err()
     **/
    this.rename = function(context, parent, name, newParent, newName, reply) {

    };

    /**
     * Create a hard link
     *
     * @param {Object} context Context info of the calling process.
     * @param {Number} inode The old inode number.
     * @param {Number} newparent Inode number of the new parent directory.
     * @param {String} newname New name to create.
     * @param {Object} reply Reply instance.
     *
     * Valid replies: reply.entry() or reply.err()
     **/
    this.link = function(context, inode, newParent, newName, reply) {

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
     * @param {Object} reply Reply instance.
     *
     * Valid replies: reply.open() or reply.err()
     **/
    this.open = function(context, inode, fileInfo, reply) {

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

var PosixError = {
    //https://github.com/torvalds/linux/blob/master/include/asm-generic/errno-base.h
    EPERM               : 1,/* Operation not permitted */
    ENOENT              : 2,/* No such file or directory */
    ESRCH               : 3,/* No such process */
    EINTR               : 4,/* Interrupted system call */
    EIO                 : 5,/* I/O error */
    ENXIO               : 6,/* No such device or address */
    E2BIG               : 7,/* Argument list too long */
    ENOEXEC             : 8,/* Exec format error */
    EBADF               : 9,/* Bad file number */
    ECHILD              : 10,/* No child processes */
    EAGAIN              : 11,/* Try again */
    ENOMEM              : 12,/* Out of memory */
    EACCES              : 13,/* Permission denied */
    EFAULT              : 14,/* Bad address */
    ENOTBLK             : 15,/* Block device required */
    EBUSY               : 16,/* Device or resource busy */
    EEXIST              : 17,/* File exists */
    EXDEV               : 18,/* Cross-device link */
    ENODEV              : 19,/* No such device */
    ENOTDIR             : 20,/* Not a directory */
    EISDIR              : 21,/* Is a directory */
    EINVAL              : 22,/* Invalid argument */
    ENFILE              : 23,/* File table overflow */
    EMFILE              : 24,/* Too many open files */
    ENOTTY              : 25,/* Not a typewriter */
    ETXTBSY             : 26,/* Text file busy */
    EFBIG               : 27,/* File too large */
    ENOSPC              : 28,/* No space left on device */
    ESPIPE              : 29,/* Illegal seek */
    EROFS               : 30,/* Read-only file system */
    EMLINK              : 31,/* Too many links */
    EPIPE               : 32,/* Broken pipe */
    EDOM                : 33,/* Math argument out of domain of func */
    ERANGE              : 34,/* Math result not representable */

    //https://github.com/torvalds/linux/blob/master/include/asm-generic/errno.h
    EDEADLK             : 35,/* Resource deadlock would occur */
    ENAMETOOLONG        : 36,/* File name too long */
    ENOLCK              : 37,/* No record locks available */
    ENOSYS              : 38,/* Function not implemented */
    ENOTEMPTY           : 39,/* Directory not empty */
    ELOOP               : 40,/* Too many symbolic links encountered */
    EWOULDBLOCK         : 11,/* Operation would block */
    ENOMSG              : 42,/* No message of desired type */
    EIDRM               : 43,/* Identifier removed */
    ECHRNG              : 44,/* Channel number out of range */
    EL2NSYNC            : 45,/* Level 2 not synchronized */
    EL3HLT              : 46,/* Level 3 halted */
    EL3RST              : 47,/* Level 3 reset */
    ELNRNG              : 48,/* Link number out of range */
    EUNATCH             : 49,/* Protocol driver not attached */
    ENOCSI              : 50,/* No CSI structure available */
    EL2HLT              : 51,/* Level 2 halted */
    EBADE               : 52,/* Invalid exchange */
    EBADR               : 53,/* Invalid request descriptor */
    EXFULL              : 54,/* Exchange full */
    ENOANO              : 55,/* No anode */
    EBADRQC             : 56,/* Invalid request code */
    EBADSLT             : 57,/* Invalid slot */

    EDEADLOCK           : 35,
    EBFONT              : 59,/* Bad font file format */
    ENOSTR              : 60,/* Device not a stream */
    ENODATA             : 61,/* No data available */
    ETIME               : 62,/* Timer expired */
    ENOSR               : 63,/* Out of streams resources */
    ENONET              : 64,/* Machine is not on the network */
    ENOPKG              : 65,/* Package not installed */
    EREMOTE             : 66,/* Object is remote */
    ENOLINK             : 67,/* Link has been severed */
    EADV                : 68,/* Advertise error */
    ESRMNT              : 69,/* Srmount error */
    ECOMM               : 70,/* Communication error on send */
    EPROTO              : 71,/* Protocol error */
    EMULTIHOP           : 72,/* Multihop attempted */
    EDOTDOT             : 73,/* RFS specific error */
    EBADMSG             : 74,/* Not a data message */
    EOVERFLOW           : 75,/* Value too large for defined data type */
    ENOTUNIQ            : 76,/* Name not unique on network */
    EBADFD              : 77,/* File descriptor in bad state */
    EREMCHG             : 78,/* Remote address changed */
    ELIBACC             : 79,/* Can not access a needed shared library */
    ELIBBAD             : 80,/* Accessing a corrupted shared library */
    ELIBSCN             : 81,/* .lib section in a.out corrupted */
    ELIBMAX             : 82,/* Attempting to link in too many shared libraries */
    ELIBEXEC            : 83,/* Cannot exec a shared library directly */
    EILSEQ              : 84,/* Illegal byte sequence */
    ERESTART            : 85,/* Interrupted system call should be restarted */
    ESTRPIPE            : 86,/* Streams pipe error */
    EUSERS              : 87,/* Too many users */
    ENOTSOCK            : 88,/* Socket operation on non-socket */
    EDESTADDRREQ        : 89,/* Destination address required */
    EMSGSIZE            : 90,/* Message too long */
    EPROTOTYPE          : 91,/* Protocol wrong type for socket */
    ENOPROTOOPT         : 92,/* Protocol not available */
    EPROTONOSUPPORT     : 93,/* Protocol not supported */
    ESOCKTNOSUPPORT     : 94,/* Socket type not supported */
    EOPNOTSUPP          : 95,/* Operation not supported on transport endpoint */
    EPFNOSUPPORT        : 96,/* Protocol family not supported */
    EAFNOSUPPORT        : 97,/* Address family not supported by protocol */
    EADDRINUSE          : 98,/* Address already in use */
    EADDRNOTAVAIL       : 99,/* Cannot assign requested address */
    ENETDOWN            : 100,/* Network is down */
    ENETUNREACH         : 101,/* Network is unreachable */
    ENETRESET           : 102,/* Network dropped connection because of reset */
    ECONNABORTED        : 103,/* Software caused connection abort */
    ECONNRESET          : 104,/* Connection reset by peer */
    ENOBUFS             : 105,/* No buffer space available */
    EISCONN             : 106,/* Transport endpoint is already connected */
    ENOTCONN            : 107,/* Transport endpoint is not connected */
    ESHUTDOWN           : 108,/* Cannot send after transport endpoint shutdown */
    ETOOMANYREFS        : 109,/* Too many references: cannot splice */
    ETIMEDOUT           : 110,/* Connection timed out */
    ECONNREFUSED        : 111,/* Connection refused */
    EHOSTDOWN           : 112,/* Host is down */
    EHOSTUNREACH        : 113,/* No route to host */
    EALREADY            : 114,/* Operation already in progress */
    EINPROGRESS         : 115,/* Operation now in progress */
    ESTALE              : 116,/* Stale NFS file handle */
    EUCLEAN             : 117,/* Structure needs cleaning */
    ENOTNAM             : 118,/* Not a XENIX named type file */
    ENAVAIL             : 119,/* No XENIX semaphores available */
    EISNAM              : 120,/* Is a named type file */
    EREMOTEIO           : 121,/* Remote I/O error */
    EDQUOT              : 122,/* Quota exceeded */

    ENOMEDIUM           : 123,/* No medium found */
    EMEDIUMTYPE         : 124,/* Wrong medium type */
    ECANCELED           : 125,/* Operation Canceled */
    ENOKEY              : 126,/* Required key not available */
    EKEYEXPIRED         : 127,/* Key has expired */
    EKEYREVOKED         : 128,/* Key has been revoked */
    EKEYREJECTED        : 129,/* Key was rejected by service */

    /* for robust mutexes */
    EOWNERDEAD          : 130,/* Owner died */
    ENOTRECOVERABLE     : 131,/* State not recoverable */
    ERFKILL             : 132,/* Operation not possible due to RF-kill */
    EHWPOISON           : 133/* Memory page has hardware error */
};

var fuse = new bindings.Fuse();
fuse.fuse_version = bindings.fuse_version;

module.exports = {
    fuse: fuse,
    FileSystem: FileSystem,
    PosixError: PosixError
};
