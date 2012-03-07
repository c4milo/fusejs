#!/usr/bin/env node
var fuse = require('../fuse');

/**
 * This file does the following:
 *
 * - parses command line options (-d -s and -h)
 * - passes relevant mount options to the fuse_mount()
 * - installs signal handlers for INT, HUP, TERM and PIPE
 * - registers an exit handler to unmount the filesystem on program exit
 * - creates a fuse handle
 * - registers the operations
 * - calls the single-threaded loop
 **/
var operations = require('./loopback');

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

fuse.mount({
    mountpoint: '/tmp/loopbackjs',
    operations: operations,
    options: process.argv
});

