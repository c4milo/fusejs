"use strict";
const ExampleFS = require('../examples/example.js').ExampleFS;
const fuse = require('../fuse').fuse
const pth = require('path');
const assert = require("assert");
const fs = require('fs');
const os = require('os');
const exec = require('child_process').exec;

var mountpoint = "/tmp/mnt2";
// fuse.mount({
// 	filesystem: ExampleFS,
// 	options: ["TestExampleFS","-o", "allow_other", mountpoint]
// });

describe('The Filesystem', function() {
	before('should be mountable', function(done){		
		fuse.mount({
			filesystem: ExampleFS,
			options: ["TestExampleFS","-o", "allow_other", mountpoint]
		});
		setTimeout(done, 25); //wait a second before actually starting the tests
	});

    it('should have one folder called hello', function (done) {
    	fs.readdir(mountpoint, function(err, files){
    		if(err){
    			done(err);
    			return;
    		}
    		if(files.length == 1){
    			let folder = files[0];
    			if(folder === "hello"){
    				done();
    				return;
    			} 
    		}
    		done(files);
    	});
    });

    describe("with a file called world.txt", function(){
    	it('should exist', function(done){
	    	fs.readdir(pth.join(mountpoint,'hello'), function(err, files){
	    		if(err){
	    			done(err);
	    			return;
	    		}
	    		if(files.length == 1){
	    			let folder = files[0];
	    			if(folder === "world.txt"){
	    				done();
	    				return;
	    			} 
	    		}
	    		done(files);
	    	});

    	});

    	it('should say "hello world"', function(done){
    		const path = pth.join(mountpoint,'hello','world.txt');
    		fs.readFile(path, {encoding: "utf8"},function(err,data){
    			if(err){
    				done(err);
    				return;
    			}
    			if(data === "hello world"){
    				done();
    				return;
    			}
    			console.log(data);
    			done(data);
    			return;
    		});
    	});

    });

    after('should be unmounted', function(done){
		var command;
		switch (os.type()){
		    case 'Linux':
		        command = `umount -f /tmp/mnt2`
		        break;
		    case 'Darwin':
		        command = `diskutil umount force /tmp/mnt2`
		        break;
	    }
	    exec(command,   function (error, stdout, stderr) {
	    	if(error){
	    		console.log(error);
	    		done(error);
	    		return;
	    	}

	    	if(stderr){
	    		console.log(stderr);
	    		done(stderr);
	    		return;
	    	}
	    	done();

		});

    });
});