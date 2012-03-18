/**
 * Loopback filesystem
 **/
var FileSystem = require('../fuse').FileSystem;
var PosixError = require('../fuse').PosixError;

var util = require('util');

var Loopback = function(options) {
    //this.fuse = fuse;
    this.options = options;

    console.log(options);
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
        //reply.entry(PosixError.ENOENT);
    };

    this.forget = function(context, inode, nlookup) {
        console.log('Forget was called!!');
    };

    this.getattr = function(context, inode, reply) {
        console.log('Getattr was called!!');
        console.log(context);
        console.log(inode);

        //stat object
        var attrs = {
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
        };
        //reply.attr(attrs, 1000);
        reply.attr(PosixError.EIO);
    };

    this.setattr = function(context, inode, attrs, reply) {
        console.log('Setattr was called!!');
        console.log(attrs);

        //reply.attr(attrs, 1000);
        reply.attr(PosixError.EIO);
    };

    this.readlink = function(context, inode, reply) {

    };

    this.mknod = function(context, parent, name, mode, rdev, reply) {

    };

    this.mkdir = function(context, parent, name, mode, reply) {

    };

    this.unlink = function(context, parent, name, reply) {

    };

    this.rmdir = function(context, parent, name, reply) {

    };

    this.symlink = function(context, link, parent, name) {

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
