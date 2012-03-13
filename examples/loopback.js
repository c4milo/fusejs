/**
 * Loopback filesystem
 **/
var FileSystem = require('../fuse').FileSystem;
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
        //reply();
    };

    this.forget = function(context, inode, nlookup) {

    };

    this.getattr = function(context, inode) {

    };

    this.setattr = function(context, inode, attr, toSet, fileInfo) {

    };

    this.readlink = function(context, inode) {

    };

    this.mknod = function(context, parent, name, mode, rdev) {

    };

    this.mkdir = function(context, parent, name, mode) {

    };

    this.unlink = function(context, parent, name) {

    };

    this.rmdir = function(context, parent, name) {

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
