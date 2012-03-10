#!/usr/bin/env node
var fuse = require('../fuse').fuse;

var Loopback = require('./loopback');

/**
 * The following options are parsed for process.argv:
 * '-f' foreground
 * '-d' debug
 * '-odebug' foreground, but keep the debug option
 * '-s' single threaded
 * '-h' '--help' help
 * '-ho' help without header
 * '-ofsname=..' file system name, if not present,
 * then set to the program name
 *
 * It will also parse and store
 * any custom options for a given filesystem.
 **/

// /dev/hda1       /mnt/WinXP      ntfs-3g      quiet,defaults,locale=en_US.utf8,umask=0,noexec

fuse.mount({
    mountpoint: '/tmp/test',
    filesystem: Loopback,
    options: process.argv
});

