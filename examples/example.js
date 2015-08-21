/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

/*
 This example has 3 inodes ( /hello/world.txt ).
 1: the root inode is always the first
 2: a folder.
 3: a file.


*/
"use strict";
const now = Date.now()

const root = {
	inode:1, 
	ctime: now, 
	mtime:now,
	mode: 16895,//0o40777
	size: 4096, //standard size of a folder
	nlink: 2 //itself counts as one and the folder
} 

const folder = {
	inode:2, 
	ctime: now, 
	mtime:now,
	mode: 16895,//0o40777
	size: 4096, //standard size of a folder
	nlink: 2 //itself counts as one and the file 
}

var file_content = "hello world";
const file = {
	inode:3, 
	ctime: now, 
	mtime:now,
	mode: 33279,//0o100777
	size: file_content.length, //standard size of a folder
	nlink: 1 //a file only has one link
}

const fusejs = require('../fuse') //require('fusejs')
const FileSystem = fusejs.FileSystem;
const PosixError = fusejs.PosixError;


class ExampleFS extends fusejs.FileSystem{

	/* 
	the context object contains 4 fields:
		uid: user id
		gid: group id
		pid: process id
		umask: Umask of the calling process (only available for fuse version 2.8 and higher)
	*/

	/* lookup, getattr, releasedir, opendir, readdir are the minimum functions that need to be implemented for listing directories */
	lookup(context, parentInode, name, reply){

		if(parentInode == 1 && name === "hello" ){
			const entry = {
				inode: 2, //inode of the child, in this case the folder hello
				attr: folder,
				generation: 1 //some filesystems rely on this generation number, such as the  Network Filesystem
			} 
			reply.entry(entry);
			return;
		}

		if(parentInode == 2 && name === "world.txt" ){
			const entry = {
				inode: 3, //inode of the child, in this case the file world.txt
				attr: file,
				generation: 1 //some filesystems rely on this generation number, such as the  Network Filesystem
			} 
			reply.entry(entry);
			return;
		}
		reply.err(PosixError.ENOENT);

	}

	getattr(context, inode, reply){
		//Get file attributes
		//http://fuse.sourceforge.net/doxygen/structfuse__lowlevel__ops.html#a994c316fa7a1ca33525a4540675f6b47
		switch(inode){
			case 1:
				reply.attr(root, 3600); //3600, a timeout value, in seconds, for the validity of this inode. so one hour
				break;
			case 2: 
				reply.attr(folder, 3600); //3600, a timeout value, in seconds, for the validity of this inode. so one hour
				break;
			case 3: 
				reply.attr(file, 3600); //3600, a timeout value, in seconds, for the validity of this inode. so one hour
				break;
			default:
				reply.err(PosixError.ENOTENT);			
		}
		return;
	}
	releasedir(context, inode, fileInfo, reply){
	    // console.log('Releasedir was called!');
	    // console.log(fileInfo);
	    reply.err(0);
	}

	opendir(context, inode, fileInfo, reply){
	    reply.open(fileInfo);
	}


	readdir(context, inode, requestedSize, offset, fileInfo, reply){
		//http://fuse.sourceforge.net/doxygen/structfuse__lowlevel__ops.html#af1ef8e59e0cb0b02dc0e406898aeaa51
		
		/*
		Read directory
		Send a buffer filled using reply.addDirEntry. Send an empty buffer on end of stream.
		fileInfo.fh will contain the value set by the opendir method, or will be undefined if the opendir method didn't set any value.
		Returning a directory entry from readdir() does not affect its lookup count.
		Valid replies: reply.addDirEntry reply.buffer, reply.err
		*/

		/*
		size is the maximum memory size of the buffer for the underlying fuse
		filesystem. currently this cannot be determined a priori
		*/

		// const size = Math.max( requestedSize , children.length * 256);
		const size = Math.max( requestedSize , 256);


		switch(inode){
			case 1:
				reply.addDirEntry("hello", size, folder, offset);
				reply.buffer(new Buffer(0), requestedSize)
				break;
			case 2:
				reply.addDirEntry("world.txt", size, file, offset);
				reply.buffer(new Buffer(0), requestedSize);
				break;
			default:
				reply.err(PosixError.ENOENT)
		}
	}

	open(context, inode, fileInfo, reply){
		if(inode == 3)
		{	
			reply.open(fileInfo);		
			return;
		}
		if(inode < 3){
			reply.err(PosixError.EISDIR);
			return;
		}

		reply.err(PosixError.ENOENT);

	}

	read(context, inode, len, offset, fileInfo, reply){
		if(inode == 3){
			const length = file_content.length
			const content = file_content.substr(offset,Math.min(length, offset + len));
			reply.buffer(new Buffer(content), content.length);
			return;
		}

		reply.err(PosixError.ENOENT);
		return;
	}

	release(context, inode, fileInfo, reply){
		reply.err(0);
	}


}

setTimeout( function(){
console.log("example timeout still working")
},5000);

module.exports.ExampleFS = ExampleFS;