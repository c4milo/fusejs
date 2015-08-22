#!/usr/bin/env node
/**
 * The following options are parsed from the command line arguments:
 * 'folder to mirror' //the first argument should be the folder to mirror
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

const fuse = require('fusejs').fuse

// setup the directories
var loopback = require('./loopback.js');
const LoopbackFS = loopback.LoopbackFS;
loopback.setLoopback( process.argv[1] );
 
fuse.mount({
	filesystem: LoopbackFS,
	options: ['LoopbackFS'].concat(process.argv.slice(2,process.argv.length))
})

