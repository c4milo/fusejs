"use strict";
const fuse = require('fusejs').fuse
const pth = require('path');
const fs = require('fs');
const os = require('os');
const exec = require('child_process').exec;
const expect = require('chai').expect;

// setup the directories
var mountpoint = "/tmp/mnt2";
var homeDirectory =  process.env['HOME'];
var loopback = require('../examples/loopback.js');
const LoopbackFS = loopback.LoopbackFS
loopback.setLoopback( homeDirectory );

describe('The Filesystem', function() {
	before('should be mountable', function(done){		
		fs.mkdir(mountpoint, function(){
			fuse.mount({
				filesystem: LoopbackFS,
				options: ["TestLoopbackFS","-o", "allow_other", mountpoint]
			});
			setTimeout(done, 150); //wait a second before actually starting the tests
		});
	});

	it('should have the same root content', function(done){

		fs.readdir(homeDirectory, function(err, homeFiles){
			if(err){
				done(err);
				return;
			}
			fs.readdir(mountpoint, function(err, mountedFiles){
				if(err){
					done(err);
					return
				}

				expect(homeFiles).to.have.length(mountedFiles.length);
				for(let file of mountedFiles){
					expect(homeFiles).to.include(file);
				}
				done();
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