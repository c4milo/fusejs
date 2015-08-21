"use strict";

const fuse = require('../fuse').fuse;
const ExampleFS = require('./example').ExampleFS;
const  exec = require('child_process').exec;
const os = require('os');
const fs = require('fs');

console.log(process.argv.slice(2,process.argv.length));

const m = fuse.mount({
    filesystem: ExampleFS,
    options: ["ExampleFS"].concat(process.argv.slice(2,process.argv.length))
});

// unmounting 
// setTimeout(function(){
// 	console.log("starting to unmount");
// 	var command;
// 	switch (os.type()){
// 	    case 'Linux':
// 	        command = `umount -f /tmp/mnt2`
// 	        break;
// 	    case 'Darwin':
// 	        command = `diskutil umount force /tmp/mnt2`
// 	        break;
//     }
//     exec(command,   function (error, stdout, stderr) {
//     	console.log('stdout: ' + stdout);
//     	console.log('stderr: ' + stderr);
//     	if (error !== null) {
//       		console.log('exec error: ' + error);
//     	}
// 	});
// }, 1200);