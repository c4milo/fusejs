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
				// options: ["TestLoopbackFS","-o", "allow_other", '-d',mountpoint]
				options: ["TestLoopbackFS","-o", "allow_other",'-s', mountpoint]
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

	it('multiple reads should not cause it to crash', function(done){
        let done1 = false;
        let done2 = false;
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
                done1 = true;
                if( done1 && done2){
    				done();
                }
			});
		});
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
                done2 = true;
                if( done1 && done2){
    				done();
                }
			});
		});

	});
    
    it( 'should report the same size for each file', function(done){
        this.timeout(2000);
        var once = false;
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
                expect(homeFiles).to.have.length(mountedFiles.length)
                var count = 0;
                for( const file of mountedFiles ){
    				expect(homeFiles).to.include(file);
	
                    fs.stat(pth.join(homeDirectory, file), (err, attr1)=>{
                        if(err){
                            once = true;
                            done(err);
                            return;
                        }
                        fs.stat(pth.join(mountpoint, file), (err, attr2)=>{                               
                            count++;
                            if(err){
                                once = true;
                                console.log(err);
                                done(err);


                                return;
                            }
                            // if( attr2.size == 5678 && attr2.size != attr1.size){
                            //     // console.log(file, attr2, attr1);
                            //     // return;
                            // }
                            // console.log(file, attr1.size, attr2.size);
                            expect(attr2.atime.getTime()).to.be.closeTo(attr1.atime.getTime(),1);
                            expect(attr2.size).to.equal(attr1.size);
                            expect(attr2.ino).to.equal(attr1.ino);
                            expect(attr2.atime).to.deep.equal(attr1.atime);
                            expect(attr2.ctime).to.deep.equal(attr1.ctime);
                            expect(attr2.mtime).to.deep.equal(attr1.mtime);
                            // expect(attr2).to.deep.equal(attr1);
                            // console.log(count, mountedFiles.length,  homeFiles.length);
                            if(count == mountedFiles.length){
                                if(!once){
                                    once = true;
                                    done();
                                }
                                return;
                            }
                        });
                    });     
                }
                // done();
            });
        })
        
    })
    after('should be unmounted', function(done){
        const timeoutTime = 5000;
        this.timeout(timeoutTime);
		var command;
		switch (os.type()){
		    case 'Linux':
		        command = `umount -f /tmp/mnt2`
		        break;
		    case 'Darwin':
		        command = `diskutil umount force /tmp/mnt2`
		        break;
	    }
	    setTimeout(exec,0,command,   function (error, stdout, stderr) {
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